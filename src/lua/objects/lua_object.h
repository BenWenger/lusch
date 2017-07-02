#ifndef LUSCH_LUA_OBJECTS_LUA_OBJECT_H_INCLUDED
#define LUSCH_LUA_OBJECTS_LUA_OBJECT_H_INCLUDED

/*
    This comment is sort of a continuation of the one found in lua_wrapper.h.  Read that first.

    Some objects (files, graphics, etc) need to have shared ownership with Lua's garbage collector.
    Lua needs ownership because the object will be accessible via Lua code... and the C++ portion
    needs ownership because some of these objects may exist inside the Project.

    LuaObject and LuaUserData<T> handle the nitty gritty of making this actually work.

    To have a class which is a Lua-acessible object, it should do the following:

    - Publicly derive from LuaUserData<T>, where T is the derived class type (no virtual functions
        need to be implemented -- the inheritance is just for RTTI, type checks, and common code reuse).

    - Have some kind of restricted interface for construction to ensure that objects are ALWAYS
        ALWAYS ALWAYS created on the heap and owned by a std::shared_ptr.  ALWAYS ALWAYS ALWAYS.

    - Implement the below static functions:

            static const char* getClassName()   <-  does nothing but return a string to identify the 
                    class name (for error reporting only).  The function could also return a std::string
                    if desired.

            static void registerMemberFunctions()  <-  Calls LuaFunction::addMember<T> for each
                    "member function" that will be exposed to Lua for this class.




        Classes which do this will have a public function 'pushToLua' provided for them, which will push
    a representation of this object onto the Lua stack so its members can be called from the Lua code.
 */

#include <memory>
#include <stdexcept>
#include "lua/lua_wrapper.h"
#include "lua/lua_function.h"
#include "lua/lua_stacksaver.h"

namespace lsh
{
    class LuaObject : public std::enable_shared_from_this<LuaObject>
    {
    public:
        typedef std::shared_ptr<LuaObject>      Ptr;
        
        virtual                 ~LuaObject() = default;
        void                    pushToLua(Lua& lua);
        
        static Ptr getPointerFromLuaStack(Lua& lua, int index, const char* errname)
        {
            // I'm doing a lot more safeguarding here than I probably need to.

            if(lua_type(lua, index) != LUA_TUSERDATA)
                throw Error(std::string("Expected ") + errname + " to be a Lusch Object");

            void* rawptr = lua_touserdata(lua, index);
            if(!rawptr)         throw Error("Internal error: Raw pointer in user data when attempting to get object off Lua stack");

            Ptr** pp = reinterpret_cast<Ptr**>(rawptr);
            if(!*pp)            throw Error("Internal error: Raw pointer in user data when attempting to get object off Lua stack");

            Ptr p = **pp;
            if(!p)              throw Error("Internal error: Raw pointer in user data when attempting to get object off Lua stack");

            return p;
        }

    private:
        // assert that all objects must be derived from LuaUserData<T> and not from this class directly
        template <typename T> friend class LuaUserData;
        LuaObject() = default;                                  // only LuaUserData can construct
        LuaObject(const LuaObject&) = delete;
        LuaObject& operator = (const LuaObject&) = delete;

        //  To be implemented by LuaUserData<T>
        virtual void            pushIndexingFunction(Lua& lua) const = 0;


    protected:
        static int lua__gc(lua_State* L)
        {
            try
            {
                // paramter 1 should be userdata
                if(lua_type(L, 1) != LUA_TUSERDATA)     luaL_error(L, "Internal error:  Lua __gc metamethod called without userdata as the argument");

                void* rawptr = lua_touserdata(L, 1);
                if(!rawptr)                             luaL_error(L, "Internal error:  Lua __gc metamethod called with null pointer as the argument");

                // we can (or rather, unfortunately must) assume all userdata points to Ptr**
                Ptr** pp = reinterpret_cast<Ptr**>(rawptr);
                delete *pp;
                *pp = nullptr;

                return 1;
            }
            catch(...)
            {
                luaL_error(L, "REALLY BAD INTERNAL ERROR:  Exception thrown during __gc metamethod call.  Program is leaking memory!!!");
            }

            return 0;
        }
    };

    
    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    template <typename T>
    class LuaUserData : public LuaObject
    {
    public:
        static std::shared_ptr<T> getPointerFromLuaStack(Lua& lua, int index, const char* errname)
        {
            Ptr p = LuaObject::getPointerFromLuaStack(lua, index, errname);

            auto ret = std::dynamic_pointer_cast<T>( p );
            if(!ret)
                throw Error(std::string("Expected ") + errname + " to be of type " + T::getClassName());

            return ret;
        }

    protected:
        LuaUserData()
        {
            if(LuaFunction::isMemberListEmpty<T>())
                T::registerMemberFunctions();
        }
        
    private:
        virtual void pushIndexingFunction(Lua& lua) const override
        {
            lua_pushcfunction(lua, &LuaUserData<T>::lua__index);
        }

        static int lua__index(lua_State* L)
        {
            try
            {
                auto str = lua_tostring(L, 1);
                if(!str)        lua_pushnil(L);
                else            LuaFunction::pushMember<T>( L, str );

                return 1;
            }
            catch(std::exception& e)
            {
                luaL_error( L, "Internal error:  Exception throw during __index:  %s", e.what() );
            }
            catch(...)
            {
                luaL_error( L, "Internal error:  Unknown value thrown during __index" );
            }

            return 0;
        }
    };
    
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    
    inline void LuaObject::pushToLua(Lua& lua)
    {
        LuaStackSaver stk(lua);

        // Step 1:  make the userdata
        void* rawptr = lua_newuserdata(lua, sizeof(Ptr*));
        if(!rawptr)     throw std::bad_alloc();
        Ptr** pp = reinterpret_cast<Ptr**>(rawptr);

        // Step 2:  build the metatable
        lua_createtable(lua, 0, 3);
        lua_pushcfunction(lua, &LuaObject::lua__gc);
        lua_setfield(lua, -2, "__gc");
        pushIndexingFunction(lua);                      // the one thing that is type specific
        lua_setfield(lua, -2, "__index");
        lua_newtable(lua);                              // could this just be nil? ??
        lua_setfield(lua, -2, "__metatable");

                // TODO - do __eq?

        // Step 3:  put the shared pointer in Lua owned memory
        *pp = new Ptr( shared_from_this() );

        // Step 4:  assign the metatable
        lua_setmetatable(lua, -2);

        // Now everything is bound -- and ownership is properly shared!  We're done!
        //    but leave the user data on the stack, as that was the entire point of all this
        stk.escape();
    }

}

#endif
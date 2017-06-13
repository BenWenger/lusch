#ifndef LUSCH_LUA_LUAUSERDATA_H_INCLUDED
#define LUSCH_LUA_LUAUSERDATA_H_INCLUDED

#include "error.h"
#include "luaobj.h"
#include "luastacksaver.h"
#include <unordered_map>
#include <string>
#include <memory>

/*
        Objects that are available to Lua (like graphics objects) need to be owned by Lua's garbage
    collection, otherwise there is no way to know when to destroy them.  This actually turns out to
    be somewhat complicated to do.

        To simplify this, classes that want to be accessible via Lua should derive from LuaUserData<T>,
    where T is their class (ie... MyGraphicsClass should derive from LuaUserData<MyGraphicsClass>).
    They should then be constructed by 'createAndPush', which will construct the object as well as push
    it to the Lua stack.  It will also do all the necessary cleanup work.

        Instantiating objects "normally" (ie, on the stack or directly without using createAndPush),
    will theoretically work fine, but the object will not be accessible via Lua.  However, many derived
    classes will forbid construction by means other than createAndPush.

        Classes will generally want some member functions available (a graphics class might want a
    'setPixel' or something).  In their constructor, classes should call 'addLuaMemberFunc'.  The
    given function will then be called via Lua with traditional syntax:

            obj:funcName(params)
 */

namespace lsh
{

    /*      This is just an abstract root class from which LuaUserData<T> is derived.
        It exists as an anchoring point for communication with Lua.  Lua userdata provides no
        direct way to check types, it just gives you a pointer.  So in our case, we can assume
        that ALL userdata in the Lua is of type 'LuaUserDataAbstract**'.. and we can dynamic_cast
        down to more appropriate types when needed.
    */
    class LuaUserDataAbstract
    {
    public:
        virtual ~LuaUserDataAbstract() {}

    private:
        // Only LuaUserData should derive from this class directly!
        template <typename T> friend class LuaUserData;
        LuaUserDataAbstract() = default;

        LuaUserDataAbstract(const LuaUserDataAbstract&) = delete;
        LuaUserDataAbstract& operator = (const LuaUserDataAbstract&) = delete;
        

    protected:
        virtual bool pushIndexingFunction(Lua&) = 0;

        ////////////////////////////////
        // Called by the __gc metamethod for the Lua object, when the object is to be destroyed
        //    Don't throw exceptions here... use luaL_error, as we don't want exceptions to fall into Lua.
        static int garbageCollect(lua_State* L)
        {
            // First (only) parameter should be the userdata
            if(lua_type(L, 1) != LUA_TUSERDATA)         luaL_error(L, "%s", "Internal Error:  __gc metamethod called without an object as its first parameter");

            void* ptr = lua_touserdata(L, 1);
            if(!ptr)                                    luaL_error(L, "%s", "Internal Error:  __gc metamethod called with null pointer");

            // Assume the userdata is LuaUserDataAbstract**, and delete the object it points to
            auto obj = reinterpret_cast<LuaUserDataAbstract**>(ptr);
            delete *obj;
            *obj = nullptr;

            return 0;
        }
    };
    
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////
    
    /*      This class is the real meat of this approach.  The 'T' template argument is the derived class,
        and it exists here so this class can do typechecking to ensure member functions are being
        called with the correct object.
    */

    template <typename T> class LuaUserData : public LuaUserDataAbstract
    {
    public:
        template <typename... ARGS>
        static T* createAndPush(Lua& lua, ARGS&&... args)
        {
            LuaStackSaver  stk(lua);

            std::unique_ptr<T>  obj( new T(std::forward<ARGS>(args)...) );              // create an object
            LuaUserDataAbstract* abstract_ptr = obj.get();                              // an abstact pointer (to get around some template permission weirdness)
            T*                   derived_ptr = obj.get();
            auto memptr = reinterpret_cast<LuaUserDataAbstract**>(                      // create (and push) a pointer
                                lua_newuserdata(lua, sizeof(LuaUserDataAbstract*)) );

            //////////////////////////////
            // now create the metatable
            //
            //   Note:  it's important that lua.pushFunction is not called unconditionally here.  Since functions
            //  are also UserData, that would lead to an infinite recursive loop.  Therefore, the __gc handler
            //  should be pushed with the raw lua function and not with the lua wrapper.
            //
            //    And the __index function should only be pushed if necessary (if not a function object)
            //  That is why pushIndexingFunction is a separate, virtual function.  It will be pushed for all
            //  objects EXCEPT for function objects.
            lua_newtable(lua);
            lua_pushcfunction(lua, &LuaUserDataAbstract::garbageCollect);       // plug in __gc
            lua_setfield(lua, -2, "__gc");

            // plug in __index (if necessary -- if not a function object)
            if(abstract_ptr->pushIndexingFunction(lua))
                lua_setfield(lua, -2, "__index");

            // plug in __metatable (to stop Lua code from accessing this metatable)
            lua_newtable(lua);                          // can you push nil for the fake metatable??
            lua_setfield(lua, -2, "__metatable");

            // Now that we have the metatable, assign it to this object
            lua_setmetatable(lua, -2);

            // After we set the metatable, Lua officially owns our object, so transfer ownership
            *memptr = obj.get();
            obj.release();
            stk.escape();

            return derived_ptr;
        }

    protected:
        LuaUserData(const std::string& name)
        {
            className = name;
        }
        LuaUserData() = delete;
        

        ///////////////////////////////////////////////////////

        virtual bool pushIndexingFunction(Lua& lua) override
        {
            lua.pushFunction(&LuaUserData<T>::indexing);
            return true;
        }

        ///////////////////////////////////////////////////////
        typedef int (T::*member_t)(Lua&);
        void addMemberFunction(const char* name, member_t func)
        {
            memberFuncs[name] = func;
        }


    private:
        typedef std::unordered_map<std::string, member_t>   map_t;
        static map_t                memberFuncs;
        static std::string          className;

        //////////////////////////////////////////////////////////////////////
        //  Indexing is called by the __index metamethod.
        //   Get the member function by name and push it

        static int indexing(Lua& lua)
        {
            //  param 1 = the object being indexed
            //  param 2 = the index (key)

            if( lua_type(lua, 2) == LUA_TSTRING )   // The key should be a string
            {
                std::string str( lua_tostring(lua, 2) );
                auto i = memberFuncs.find(str);
                if(i == memberFuncs.end())      return 0;       // not a real member!  Don't push anything, just exit

                // otherwise, push the 'callMember' wrapper
                lua.pushFunction( std::bind(&LuaUserData<T>::callMember, std::placeholders::_1, str) );
                return 1;
            }

            // otherwise (bad key), push nothing
            return 0;
        }

        ///////////////////////////////////////////////////////////
        //
        //   callMember has to be wrapped, in the rare event the Lua script uses the wrong object to look up
        //  the function.  Example:
        //
        //      obj1.func(obj2)
        //
        //    While stupid, it is perfectly legal Lua code, and should be handled accordingly.
        //
        //    In this case, 'obj1' is our 'this' pointer which was previously used to find out which
        //  function to call.  'obj1' is not used (or available) in this function.  However, T is used to verify
        //  the type of obj2.  obj2 is the object on which to call the function.

        static int callMember(Lua& lua, std::string membername)
        {
            // For all member functions, param 1 should be the object (obj2)

            if(lua_type(lua, 1) == LUA_TUSERDATA)
            {
                // Get the userdata
                void* ptr = lua_touserdata(lua, 1);
                if(!ptr)                    throw Error( "Internal error:  null object pointer used in 'LuaUserData<>::callMember'" );

                auto i = memberFuncs.find(membername);
                if(i == memberFuncs.end())  throw Error( "Internal error:  bad membername string passed to 'LuaUserData<>::callMember'" );

                auto baseobj = *reinterpret_cast<LuaUserDataAbstract**>(ptr);
                auto realobj = dynamic_cast<T*>(baseobj);       // use dynamic cast to verify the object is the correct type
                if(realobj) // realobj is our 'obj2'
                {
                    return (realobj->*(i->second))(lua);        // call that function!
                }
            }

            // falls through here only if the user provided the wrong type of object
            throw Error( membername + ": first parameter is not a '" +  className + "' object." );
            return 0;
        }
    };
    
    ////////////////////////////////////////////////////////////////
    //  Some static crap needs to be instantiated.
    template <typename T> typename LuaUserData<T>::map_t LuaUserData<T>::memberFuncs;
    template <typename T> std::string                    LuaUserData<T>::className;

    
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////
    
    /*      Finally... pushing functions with C++ data is difficult.  Member function pointers
     *  can't be cast to void pointers, so I can't push the function I want to call as an up-value.
     *  Therefore I have to encapsulate it in a class and push a pointer to the class.  But this also
     *  means that the class will need to be owned by the Lua and be garbage collected!
     *
     *      There might be a better way of doing this, as this means that every single time Lusch
     *  pushes a function to the stack (which would happen every time an object is indexed, and a bunch
     *  of times at startup), there's overhead in creating a new userdata, setting its metatable, etc.
     *  However I spent hours racking my brain to come up with a simpler solution... and came up empty.
     *  Only other reasonable solution I can think of was to abandon the idea of an RAII 'Lua' wrapper
     *  class, which I'm not keen on doing.
     *
     *      So functions are LuaUserData!  Fuck it!
     */
    class LuaFunctionWrapper : public LuaUserData<LuaFunctionWrapper>
    {
    public:
        static void pushFunction(Lua& lua, const Lua::function_t& f)
        {
            LuaStackSaver stk(lua);

            lua_pushlightuserdata( lua, reinterpret_cast<void*>(&lua) );    // upvalue 1 = the Lua object (OK to be light user data, as it's not owned by the Lua)
            auto* obj = createAndPush( lua );                               // upvalue 2 = the LuaFunctionWrapper object
            lua_pushcclosure( lua, &LuaFunctionWrapper::wrapperFunc, 2 );

            obj->func = f;
            stk.escape();
        }

    private:
        friend class LuaUserData<LuaFunctionWrapper>;
        LuaFunctionWrapper() : LuaUserData("LuaFunctionWrapper") {}
        
    private:
        static int wrapperFunc(lua_State* L)
        {
            /////////////////////////////////////////////////
            //      Note:  It's important that this function not have any object that has a destructor, as it
            //  may exit with lua_error which may not properly unwind the stack  (it should -- if Lua is
            //  configured to use exceptions, but if someone compiles it to use longjmp it won't)
            //
            //      Though there's no getting around the exception object.  I'm not entirely sure if that will
            //  leak or not.
            //
            //      Basically... if Lua is compiled incorrectly, I'm doing what I can to avoid memory leaks --
            //  but have it use exceptions goddammit!
            
            
            //  We don't want thrown errors to fall through into Lua.  We want to use Lua's error reporting.  So put all of this
            //    in a try block, and transform exceptions into luaL_Error calls.
            try
            {
                // upvalue 1 = the Lua object
                if( lua_type(L, lua_upvalueindex(1)) != LUA_TLIGHTUSERDATA )
                    throw Error("Internal Error: function expected upvalue 1 to be light user data");

                Lua* luaptr = reinterpret_cast<Lua*>( lua_touserdata(L, lua_upvalueindex(1)) );
                if(!luaptr || (*luaptr != L))
                    throw Error("Internal Error: function Lua object from upvalue was null or did not match the given lua_State");

                // upvalue 2 = the LuaFunctionWrapper object
                if( lua_type(L, lua_upvalueindex(2)) != LUA_TUSERDATA )
                    throw Error("Internal Error: function expected upvalue 2 to be user data");

                auto baseptr = reinterpret_cast<LuaUserDataAbstract**>( lua_touserdata(L, lua_upvalueindex(2)) );
                if(!baseptr)
                    throw Error("Internal Error: function upvalue 2 was a null pointer" );

                auto obj = dynamic_cast<LuaFunctionWrapper*>(*baseptr);
                if(!obj)
                    throw Error("Internal Error: function upvalue 2 was not a function object" );

                // FINALLY we have our Function object and our Lua object.  Call it!
                return obj->func(*luaptr);
            }
            catch(std::exception& e)
            {
                luaL_error(L, "%s", e.what());
            }
            catch(...)
            {
                luaL_error(L, "Internal Error:  luaFunctionWrapper function call caught something that wasn't an exception");
            }

            return 0;       // shouldn't reach here
        }

    protected:
        virtual bool pushIndexingFunction(Lua& lua) override
        {
            return false;
        }


        Lua::function_t         func;
    };
}


#endif
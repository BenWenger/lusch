#ifndef LUSCH_LUA_LUA_FUNCTION_H_INCLUDED
#define LUSCH_LUA_LUA_FUNCTION_H_INCLUDED
/*
    This comment is sort of a continuation of the one found in lua_wrapper.h.  Read that first.


    LuaFunction is a global registry of function callbacks of slightly-different type signatures than
    those directly supported by Lua.  The idea is, in order to push a callback to Lua, you need to first
    add that callback to the LuaFunction registry (addXXX functions) with an associated name.  Then to
    push it to the Lua stack, you call the matching pushXXX function with that name as the key.

    The pushXXX functions actually push a wrapper function which accomplishes the below goals:
    - Wraps the callback in a try/catch block
    - Fetches any extra data (like an associated object for a member function) needed
    - Calls the function with the differing signature.


    LuaFunction supports three different types of callbacks:
    1)  "Global", which are more or less the same as normal Lua callbacks, with the exception that
        they're wrapped in try/catch and they take a Lua& as a parameter instead of a lua_State*.

    2) "Member", which are member functions.  The associated object is assumed to be the first parameter
        passed in via the Lua code --- typically with the colon operator, a la "file:read(...)".  In
        addition to above benefits, this also does type checking to ensure the object passed in is actually
        of the desired object type.

    3) "Bounded", which are also member functions, but the associated object is not a parameter or upvalue.
        The object is assumed to have been "bounded" to the Lua state (this is primarily for the Project
        object, for use in functions like lsh.get and lsh.set).  See lua_binding.h for details of how
        Bounded objects work.


    pushXXX functions record the string key of the desired callback in an upvalue.  This string key is the
    one you'd use when you want to push the function to Lua via pushXXX functions.


    Cleanup here is pretty much nonexistent, and the registries are completely global.  As there are only a
    finite number of callback functions, and it doesn't matter what instance of Lua we have -- they're all
    going to use the same callbacks.
 */

#include <string>
#include <map>
#include <functional>
#include <stdexcept>
#include "lua_wrapper.h"
#include "error.h"

namespace lsh
{
    class LuaFunction
    {
    public:
                                static void addGlobal (const std::string& name, int (*func)(Lua&))          { globalMap[name] = func;           }
        template <typename T>   static void addMember (const std::string& name, int (T::*func)(Lua&))       { Hack<T>::memberMap[name] = func;  }
        template <typename T>   static void addBounded(const std::string& name, int (T::*func)(Lua&))       { Hack<T>::boundedMap[name] = func; }

                                static void pushGlobal (lua_State* L, const std::string& name);
        template <typename T>   static void pushMember (lua_State* L, const std::string& name);
        template <typename T>   static void pushBounded(lua_State* L, const std::string& name);
        
        template <typename T>   static bool isMemberListEmpty();
        template <typename T>   static bool isBoundedListEmpty();

    private:
        static std::map<std::string, int (*)(Lua&)>         globalMap;
        template <typename T> struct Hack       // VS doesn't support templated vars, so this is a bit of a hacky workaround
        {
            static std::map<std::string, int (T::*)(Lua&)>  memberMap;
            static std::map<std::string, int (T::*)(Lua&)>  boundedMap;
        };

    private:
        ///////////////////////////////////////////////
        ///////////////////////////////////////////////
        static int launch_global(lua_State* L)
        {
            const char* name = "(Internal Error)";
            try
            {                
                Lua* lua = Lua::fromLuaState(L);
                if(!lua)                                    throw Error("In launch function, null pointer for Lua object obtained");
                
                int upindex = lua_upvalueindex(1);          // get the index of the upvalue (function name)
                if(lua_type(L, upindex) != LUA_TSTRING)     throw Error("In launch function, Lua upvalue was not a string");
                name = lua_tostring(L, upindex);
                
                auto i = globalMap.find(name);
                if(i == globalMap.end())                    throw Error("In launch function, bad function name given (name not in registry)");
                auto func = i->second;

                // Global functions:  don't need anything extra

                // Finally call the function
                return (*func)(*lua);
            }
            catch(...) {    doException(L, name);      }

            return 0;
        }
        
        ///////////////////////////////////////////////
        ///////////////////////////////////////////////
        template <typename T>
        static int launch_member(lua_State* L)
        {
            const char* name = "(Internal Error)";
            try
            {                
                Lua* lua = Lua::fromLuaState(L);
                if(!lua)                                    throw Error("In launch function, null pointer for Lua object obtained");
                
                int upindex = lua_upvalueindex(1);          // get the index of the upvalue (function name)
                if(lua_type(L, upindex) != LUA_TSTRING)     throw Error("In launch function, Lua upvalue was not a string");
                name = lua_tostring(L, upindex);
                
                auto i = Hack<T>::memberMap.find(name);
                if(i == Hack<T>::memberMap.end())           throw Error("In launch function, bad function name given (name not in registry)");
                auto func = i->second;

                //  For member functions, the object pointer is the first parameter
                auto obj = T::getPointerFromLuaStack(*lua, 1, "parameter 1");

                // Finally call the function
                return (obj.get()->*func)(*lua);
            }
            catch(...) {    doException(L, name);      }

            return 0;
        }
        
        ///////////////////////////////////////////////
        ///////////////////////////////////////////////
        template <typename T>
        static int launch_bounded(lua_State* L)
        {
            const char* name = "(Internal Error)";
            try
            {                
                Lua* lua = Lua::fromLuaState(L);
                if(!lua)                                    throw Error("In launch function, null pointer for Lua object obtained");

                int upindex = lua_upvalueindex(1);          // get the index of the upvalue (function name)
                if(lua_type(L, upindex) != LUA_TSTRING)     throw Error("In launch function, Lua upvalue was not a string");
                name = lua_tostring(L, upindex);
                
                auto i = Hack<T>::boundedMap.find(name);
                if(i == Hack<T>::boundedMap.end())          throw Error("In launch function, bad function name given (name not in registry)");
                auto func = i->second;

                //  For bounded functions, the object pointer comes from the lua_State
                T* obj = T::fromLuaState(L);
                if(!obj)                                    throw Error("(Internal Error) In launch function, null pointer obtained for bounded object.");

                // Finally call the function
                return (obj->*func)(*lua);
            }
            catch(...) {    doException(L, name);      }

            return 0;
        }
        
        ///////////////////////////////////////////////
        ///////////////////////////////////////////////
        static void doException(lua_State* L, const char* name)
        {
            try                         { throw;                                                        }
            catch(std::exception& e)    { luaL_error(L, "%s: %s", name, e.what());                      }
            catch(...)                  { luaL_error(L, "%s: An unknown exception was thrown", name);   }
        }


    private:
        //  This class cannot be instantiated
        LuaFunction() = delete;
        LuaFunction(const LuaFunction&) = delete;
        ~LuaFunction() = delete;
    };
    
    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////
    
    template <typename T>
    inline bool LuaFunction::isMemberListEmpty()
    {
        return Hack<T>::memberMap.empty();
    }
    
    template <typename T>
    inline bool LuaFunction::isBoundedListEmpty()
    {
        return Hack<T>::boundedMap.empty();
    }

    inline void LuaFunction::pushGlobal(lua_State* L, const std::string& name)
    {
        if(globalMap.find(name) == globalMap.end())
            lua_pushnil(L);
        else
        {
            lua_pushstring(L, name.c_str());
            lua_pushcclosure(L, &LuaFunction::launch_global, 1);
        }
    }
    
    template <typename T>
    inline void LuaFunction::pushMember(lua_State* L, const std::string& name)
    {
        if(Hack<T>::memberMap.find(name) == Hack<T>::memberMap.end())
            lua_pushnil(L);
        else
        {
            lua_pushstring(L, name.c_str());
            lua_pushcclosure(L, &LuaFunction::launch_member<T>, 1);
        }
    }
    
    template <typename T>
    inline void LuaFunction::pushBounded(lua_State* L, const std::string& name)
    {
        if(Hack<T>::boundedMap.find(name) == Hack<T>::boundedMap.end())
            lua_pushnil(L);
        else
        {
            lua_pushstring(L, name.c_str());
            lua_pushcclosure(L, &LuaFunction::launch_bounded<T>, 1);
        }
    }

    ////////////////////////////////////////////////////////
    //  Instantiation
    template <typename T>   std::map<std::string, int (T::*)(Lua&)>  LuaFunction::Hack<T>::memberMap;
    template <typename T>   std::map<std::string, int (T::*)(Lua&)>  LuaFunction::Hack<T>::boundedMap;
}

#endif
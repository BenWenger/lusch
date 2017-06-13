#ifndef LUSCH_LUA_LUAOBJ_H_INCLUDED
#define LUSCH_LUA_LUAOBJ_H_INCLUDED


#include <functional>
#include <QIODevice>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include <string>

namespace lsh
{

    class Lua
    {
    public:
        class       StackSaver;
        typedef     std::function<int(Lua&)>    function_t;

                    Lua();
                    Lua(const Lua&) = delete;
                    Lua(Lua&& rhs);
        Lua&        operator = (const Lua&) = delete;
        Lua&        operator = (Lua&& rhs);
                    ~Lua();

        void        reset();
        operator    lua_State* () { return L;      }
        
        void        addLuaFile(const QString& filename, QIODevice& file);
        void        addInternalCode(const char* str);
        void        doComplexString(const std::string& rawstring, const char* str);

        std::string getString(int idx);
        void        pushString(const std::string& str);

        void        errIfTooFewArgs(const char* funcname, int argmin);
        void        warnIfTooManyArgs(const char* funcname, int argmax);


        void        pushFunction(const function_t& func);
        template <typename T>
        void        pushFunction(T* obj, int (T::*func)(Lua&))
        {
            pushFunction( std::bind(func, obj, std::placeholders::_1) );
        }

        int         call(int numParams, int numrets = LUA_MULTRET);

    private:
        lua_State*      L = nullptr;

        void        assertReady();
        void        pushLuaCodeFromFile(const char* name, QIODevice& file);
        void        pushLuaCodeFromString(const char* name, const char* str);
        
        struct CallbackContainer
        {
            Lua*        obj;
            function_t  func;
        };

        //////////////////////////////////////
        //  lua_environment.cpp
        void        buildEnvironment();
        
        void        loadLib_Base();
        void        loadLib_String();
        void        loadLib_Utf8();
        void        loadLib_Table();
        void        loadLib_Math();
        void        loadLib_Io();
        void        loadLib_Os();

        void        loadLib_Lusch();
    };

}


#endif
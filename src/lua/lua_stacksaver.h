#ifndef LUSCH_LUA_LUA_STACKSAVER_H_INCLUDED
#define LUSCH_LUA_LUA_STACKSAVER_H_INCLUDED

/*
    Very simple RAII object to preserve the size of the Lua stack.

    'escape' can be used to sever ties and allow the stack size to change.

*/

#include <lua/lua.h>

namespace lsh
{
    class LuaStackSaver
    {
    public:
        LuaStackSaver(lua_State* x, int adj = 0)
        {
            L = x;
            if(L)       stk = lua_gettop(L) + adj;
            else        stk = 0;
        }

        ~LuaStackSaver()
        {
            if(L)       lua_settop(L, stk);
        }

        void escape()
        {
            L = nullptr;
        }

    private:
        lua_State*      L;
        int             stk;
    };
}

#endif
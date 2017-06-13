#ifndef LUSCH_LUASTACKSAVER_H_INCLUDED
#define LUSCH_LUASTACKSAVER_H_INCLUDED

#include "luaobj.h"

namespace lsh
{
    class LuaStackSaver
    {
    public:
        LuaStackSaver(const LuaStackSaver&) = delete;
        LuaStackSaver& operator = (const LuaStackSaver&) = delete;

        LuaStackSaver(lua_State* L)
        {
            stt = L;
            stack = lua_gettop(stt);
        }

        ~LuaStackSaver()
        {
            setToStartSize();
        }

        void escape()
        {
            stt = nullptr;
        }

        void setToStartSize()
        {
            if(stt)
                lua_settop(stt, stack);
        }

    private:
        lua_State*  stt;
        int         stack;
    };

}

#endif
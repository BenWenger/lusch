#ifndef LUSCH_CORE_PROJECT_H_INCLUDED
#define LUSCH_CORE_PROJECT_H_INCLUDED

#include <stdexcept>
#include <QString>
#include <vector>
#include "util/qtjson.h"
#include "lua/luaobj.h"

namespace lsh
{
    class Project
    {
    public:
        void        bindToLua(Lua& lua);


    private:
        int         lua_openFile(Lua& lua);
        int         lua_setData(Lua& lua);
        int         lua_getData(Lua& lua);
    };

}

#endif
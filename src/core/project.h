#ifndef LUSCH_CORE_PROJECT_H_INCLUDED
#define LUSCH_CORE_PROJECT_H_INCLUDED

#include <stdexcept>
#include <QString>
#include <vector>
#include <unordered_map>
#include "util/qtjson.h"
#include "lua/lua_wrapper.h"
#include "projectdata.h"
#include "lua/lua_binding.h"

namespace lsh
{
    class Project : public LuaBinding<Project>
    {
    public:
                    Project();
                    Project(Project&&);
                    Project(const Project&) = delete;
        Project&    operator = (Project&&);
        Project&    operator = (const Project&) = delete;
        void        bindToLua(Lua& lua);


    private:
        int         lua_openFile(Lua& lua);
        int         lua_setData(Lua& lua);
        int         lua_getData(Lua& lua);


        std::unordered_map<std::string, ProjectData>        dat;
    };

}

#endif

#include "lua/lua_wrapper.h"
#include "lua/lua_stacksaver.h"
#include "lua/lua_function.h"
#include "project.h"
#include "projectdata.h"

namespace lsh
{
    Project::Project()
    {
        if( LuaFunction::isBoundedListEmpty<Project>() )
        {
            LuaFunction::addBounded<Project>("io.open", &Project::lua_openFile);
            LuaFunction::addBounded<Project>("lsh.get", &Project::lua_getData);
            LuaFunction::addBounded<Project>("lsh.set", &Project::lua_setData);
        }
    }

    void Project::bindToLua(Lua& lua)
    {
        addBinding(lua);

        LuaStackSaver stk(lua);

        if(lua_getglobal(lua, "io") != LUA_TTABLE)      throw Error("Internal error:  global 'io' Lua symbol is not a table");
        LuaFunction::pushBounded<Project>(lua, "io.open");
        lua_setfield(lua, -2, "open");
        lua_pop(lua, 1);                // drop the "io" table

        
        if(lua_getglobal(lua, "lsh") != LUA_TTABLE)     throw Error("Internal error:  global 'lsh' Lua symbol is not a table");
        LuaFunction::pushBounded<Project>(lua, "lsh.get");
        lua_setfield(lua, -2, "get");
        LuaFunction::pushBounded<Project>(lua, "lsh.set");
        lua_setfield(lua, -2, "set");
        lua_pop(lua, 1);                // drop the "lsh" table
    }

    int Project::lua_openFile(Lua& lua)
    {
        // TODO finish this
        return 0;
    }

    int         Project::lua_setData(Lua& lua) { return 0; }
    int         Project::lua_getData(Lua& lua) { return 0; }


}
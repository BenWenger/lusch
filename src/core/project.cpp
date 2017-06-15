
#include "lua/lua_wrapper.h"
#include "lua/lua_stacksaver.h"
#include "project.h"
#include "projectdata.h"

namespace lsh
{

    void Project::bindToLua(Lua& lua)
    {
        LuaStackSaver stk(lua);
        /*
        // Set io.open
        if(lua_getglobal(lua, "io") != LUA_TTABLE)      throw Error("Internal error:  global 'io' Lua symbol is not a table");
        lua.pushFunction(this, &Project::lua_openFile);
        lua_setfield(lua, -2, "open");
        lua_pop(lua, 1);                        // pop the io table


        // Set lsh.get
        if(lua_getglobal(lua, "lsh") != LUA_TTABLE)     throw Error("Internal error:  global 'lsh' Lua symbol is not a table");
        lua.pushFunction(this, &Project::lua_getData);
        lua_setfield(lua, -2, "get");
            // and lsh.set
        lua.pushFunction(this, &Project::lua_setData);
        lua_setfield(lua, -2, "set");
        lua_pop(lua,1);                         // pop the lsh table
        */
    }

    
    int         Project::lua_openFile(Lua& lua) { return 0; }
    int         Project::lua_setData(Lua& lua) { return 0; }
    int         Project::lua_getData(Lua& lua) { return 0; }


}
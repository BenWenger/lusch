#ifndef LUSCH_LUA_OBJECTS_LUA_IOFILE_H_INCLUDED
#define LUSCH_LUA_OBJECTS_LUA_IOFILE_H_INCLUDED

/*
        Since I don't want scripts to be able to open/modify any file on the user's HD, I'm
    reimplementing the standard IO library that comes with Lua.  This class represents
    file objects ( obtained via io.open() )
 */

#include "core/fileinfo.h"
#include "lua/lua_function.h"
#include "lua_object.h"
#include <memory>
#include <QFile>

namespace lsh
{
    class LuaIOFile : public LuaUserData<LuaIOFile>
    {
    public:
        static int          openForLua(Lua& lua, const std::string& filepath, const FileFlags& mode);

        static const char*  getClassName()                  { return "io:file";     }
        static void         registerMemberFunctions();

    private:
        int     lua_close(Lua& lua);
        int     lua_read(Lua& lua);
        int     lua_seek(Lua& lua);
        int     lua_write(Lua& lua);
        
        bool    lua_read_a(Lua& lua);
        bool    lua_read_l(Lua& lua, bool keepnewline);
        bool    lua_read_num(Lua& lua, lua_Integer num);

        QFile   file;

        LuaIOFile() = default;
        LuaIOFile(const LuaIOFile&) = delete;
        LuaIOFile& operator = (const LuaIOFile&) = delete;
    };
}

#endif
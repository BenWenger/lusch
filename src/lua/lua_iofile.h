#ifndef LUSCH_LUA_LUA_IOFILE_H_INCLUDED
#define LUSCH_LUA_LUA_IOFILE_H_INCLUDED


#include "luauserdata.h"
#include <QIODevice>
#include <memory>

namespace lsh
{
    class Lua_IOFile : public LuaUserData<Lua_IOFile>
    {
    public:
        typedef     std::unique_ptr<QIODevice>  IOPtr;

        Lua_IOFile(IOPtr&& ioptr);

    private:
        IOPtr       file;
        
        int         lua_Close(Lua& lua);
        int         lua_Read(Lua& lua);
        int         lua_Seek(Lua& lua);
        int         lua_Write(Lua& lua);
        
        bool        read_n(Lua& lua);
        bool        read_a(Lua& lua);
        bool        read_l(Lua& lua, bool include_line_break);
        bool        read_number(Lua& lua, lua_Integer num);
    };
}


#endif
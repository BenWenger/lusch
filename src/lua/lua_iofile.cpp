
#include "luaobj.h"
#include "lua_iofile.h"

namespace lsh
{    
    Lua_IOFile::Lua_IOFile(IOPtr&& ioptr)
        : LuaUserData("IO.File")
        , file(std::move(ioptr))
    {
        if(!file)
            throw Error("Internal error:  Lua_IOFile constructed with a null file pointer");
        
        addMemberFunction("close", &Lua_IOFile::lua_Close);
        addMemberFunction("read",  &Lua_IOFile::lua_Read);
        addMemberFunction("seek",  &Lua_IOFile::lua_Seek);
        addMemberFunction("write", &Lua_IOFile::lua_Write);
    }

    ////////////////////////////////////////////////

    int Lua_IOFile::lua_Close(Lua& lua)
    {
        lua.warnIfTooManyArgs("file:close", 1);
        file->close();
        return 0;
    }

    int Lua_IOFile::lua_Seek(Lua& lua)
    {
        if(!file->isOpen())         throw Error("File is not open.");

        lua.warnIfTooManyArgs("file:seek", 3);

        int stacksize = lua_gettop(lua);
        const char* whence = "cur";
        lua_Integer offset = 0;

        if(stacksize >= 2)
        {
            if(!lua_isstring(lua,2))        throw Error("file:seek parameter 2 must be a string");
            whence = lua_tostring(lua,2);
        }
        if(stacksize >= 3)
        {
            if(!lua_isinteger(lua, 3))      throw Error("file:seek parameter 3 must be an integer");
            offset = lua_tointeger(lua,3);
        }

        if     (!strcmp(whence,"set"))      ;       // no change to offset
        else if(!strcmp(whence,"cur"))      offset += file->pos();
        else if(!strcmp(whence,"end"))      offset += file->size();
        else                                throw Error("file:seek parameter 2 has unrecognized value");

        file->seek(offset);
        lua_pushinteger(lua, file->pos());
        return 1;
    }

    int Lua_IOFile::lua_Write(Lua& lua)
    {
        if(!file->isOpen())         throw Error("File is not open.");
        if(!file->isWritable())     throw Error("File is not open for writing.");

        int stacksize = lua_gettop(lua);
        for(int i = 2; i <= stacksize; ++i)
        {
            if(!lua_isstring(lua,i))    throw Error("file:write parameters must be either strings or numbers");

            size_t siz = 0;
            auto str = lua_tolstring(lua,i,&siz);

            file->write(str, static_cast<qint64>(siz));
        }

        // push 'this' on success
        lua_pushvalue(lua,1);
        return 1;
    }

    /*
        int         lua_Write(Lua& lua);
    */

    ////////////////////////////////////////////////////
    //   File Reading

    int Lua_IOFile::lua_Read(Lua& lua)
    {
        if(!file->isOpen())         throw Error("File is not open.");
        if(!file->isReadable())     throw Error("File is not open for reading.");

        if(file->atEnd())
        {
            lua_pushnil(lua);
            return 1;
        }

        if(lua_gettop(lua) < 2)     // no parameters
        {
            read_l(lua, false);
            return 1;
        }
        else
        {
            //  I use goto here.  Yeah, yeah, I know... bad practice.  But it really is the simplest way to 
            //    have an aborting procedure
            int numpushed = 0;
            int stopindex = lua_gettop(lua);
            bool ok;

            for(int i = 2; i <= stopindex; ++i)
            {
                if(lua_isinteger(lua,i))            // integer param
                {
                    ok = read_number(lua, lua_tointeger(lua,i));
                }
                else if(lua_isstring(lua,i))        // string (code) param
                {
                    auto str = lua_tostring(lua, i);
                    if      (!strcmp(str, "n"))     ok = read_n(lua);
                    else if (!strcmp(str, "a"))     ok = read_a(lua);
                    else if (!strcmp(str, "l"))     ok = read_l(lua, false);
                    else if (!strcmp(str, "L"))     ok = read_l(lua, true);
                    else
                    {
                        lua_pushnil(lua);
                        ok = false;
                    }
                }
                else
                {
                    lua_pushnil(lua);
                    ok = false;
                }

                ++numpushed;
                if(!ok)
                    break;
            }

            return numpushed;
        }

        // Should never reach here
        return 0;
    }

    /////////////
    bool Lua_IOFile::read_n(Lua& lua)
    {
        // TODO - this involves parsing a numeric literal from the file, which... ugh.  I'll do this later
        //
        //   One way to do this is to extract the string, then use lua_stringtonumber to convert it.
        // The problem with that, though, is I still need to know how much of a string to extract from the
        // file, which means doing a mini-lex job.

        lua_pushnil(lua);
        return false;
    }

    bool Lua_IOFile::read_a(Lua& lua)
    {
        try
        {
            auto dat = file->readAll();
            lua_pushlstring( lua, dat.constData(), dat.size() );
            return true;
        }
        catch(...)
        {
            lua_pushnil(lua);
            return false;
        }
    }
    
    bool Lua_IOFile::read_l(Lua& lua, bool include_line_break)
    {
        try
        {
            auto line = file->readLine();
            if(include_line_break)
                line.append('\n');

            lua_pushlstring( lua, line.constData(), line.size() );
            return true;
        }
        catch(...)
        {
            lua_pushnil(lua);
            return false;
        }
    }

    bool Lua_IOFile::read_number(Lua& lua, lua_Integer num)
    {
        try
        {
            if(num < 0)
            {
                lua_pushnil(lua);
                return false;
            }
            auto dat = file->read(num);
            lua_pushlstring( lua, dat.constData(), dat.size() );
            return true;
        }
        catch(...)
        {
            lua_pushnil(lua);
            return false;
        }
    }

    /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
    //  TODO this is wrong, move this to Project
#if 0
    int Lua::ioOpen(Lua& /*usused lua*/)
    {
        if(lua_type(L,1) != LUA_TSTRING)    throw Error("io.open: Parameter 1 must be a filename (string)");
        auto fn = getString(1);

        // Get the 'mode' flags
        std::string mode = "r";     // default to "r"
        if(lua_gettop(L) >= 2)      // did they provide a 2nd parameter?
        {
            if(lua_type(L,2) != LUA_TSTRING)    throw Error("io.open: Parameter 2 must be a mode (string)");
            mode = getString(2);
        }

        // figure out the mode
        enum {  r_flg = 1,
                w_flg = 2,
                p_flg = 4,
                b_flg = 8,
                t_flg = 16 };
        int flgs = 0;
        for(auto& c : mode)
        {
            switch(c)
            {
            case 'r':   flgs |= r_flg;  break;
            case 'w':   flgs |= w_flg;  break;
            case 'p':   flgs |= p_flg;  break;
            case 'b':   flgs |= b_flg;  break;
            case 't':   flgs |= t_flg;  break;
            default:    throw Error( "Unrecognized mode '" + mode + "'" );
            }
        }
        if(     ((flgs & r_flg) && (flgs & w_flg))      // can't have both "rw"
            ||  ((flgs & b_flg) && (flgs & t_flg))      // can't have both "bt"
            ||  ((flgs & (r_flg | w_flg)) == 0)         // must have either "r" or "w"
           )
            throw Error("Unrecognized mode '" + mode + "'");

        bool writing =      (flgs & (w_flg | p_flg)) != 0;
        bool truncating =   (flgs & w_flg) != 0;
        bool binary =       (flgs & b_flg) != 0;

        // Now that we have the filename and the mode... get the qualified file name
    }
#endif
}
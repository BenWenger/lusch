
#include "lua_iofile.h"
#include "log.h"

namespace lsh
{
    void LuaIOFile::registerMemberFunctions()
    {
        LuaFunction::addMember("close", &LuaIOFile::lua_close);
        LuaFunction::addMember("read",  &LuaIOFile::lua_read );
        LuaFunction::addMember("seek",  &LuaIOFile::lua_seek );
        LuaFunction::addMember("write", &LuaIOFile::lua_write);
    }

    ///////////////////////////////////////////////////////

    std::shared_ptr<LuaIOFile> LuaIOFile::open(const std::string& filepath, std::string mode)
    {
        bool binary = false;
        if(!mode.empty() && mode.back() == 'b')
        {
            binary = true;
            mode.pop_back();
        }

        int qmode = binary ? 0 : QIODevice::Text;
        
        if     (mode == "r")    qmode |= QIODevice::ReadOnly;
        else if(mode == "w")    qmode |= QIODevice::WriteOnly | QIODevice::Truncate;
        else if(mode == "a")    qmode |= QIODevice::Append;
        else if(mode == "r+")   qmode |= QIODevice::ReadWrite;
        else if(mode == "w+")   qmode |= QIODevice::ReadWrite | QIODevice::Truncate;
        else if(mode == "a+")   qmode |= QIODevice::ReadOnly | QIODevice::Append;
        else
        {
            Log::dbg("Invalid mode '" + mode + "' given to io.open");
            return nullptr;
        }

        //////////////////////
        auto out = std::shared_ptr<LuaIOFile>(new LuaIOFile);
        out->file.setFileName( QString::fromStdString(filepath) );
        if(!out->file.open(QIODevice::OpenModeFlag(qmode)))
            return nullptr;

        return out;
    }

    /////////////////////////////////////////////////////////////

    int LuaIOFile::lua_close(Lua& lua)
    {
        lua.checkTooManyParams(1, "file:close");

        file.close();
        return 0;
    }

    int LuaIOFile::lua_seek(Lua& lua)
    {
        lua.checkTooManyParams(3, "file:seek");

        if(!file.isOpen())
        {
            lua_pushnil(lua);
            lua_pushliteral(lua, "file:seek:  File handle is not open");
            return 2;
        }

        std::string whence = lua.getStringParam(2, "file:seek", "cur");
        lua_Integer offset = lua.getIntParam(3, "file:seek", 0 );

        if     (whence == "set")        /* no change to offset */;
        else if(whence == "cur")        offset += file.pos();
        else if(whence == "end")        offset += file.size();
        else                            throw Error( "Whence parameter '" + whence + "' is unrecognized." );

        if(!file.seek(offset))
        {
            lua_pushnil(lua);
            lua.pushString( "Failure in file:seek: '" + file.errorString().toStdString() + "'" );
            return 2;
        }

        lua_pushinteger( lua, file.pos() );
        return 1;
    }

    int LuaIOFile::lua_write(Lua& lua)
    {
        if(!file.isWritable())
        {
            lua_pushnil(lua);
            lua_pushliteral(lua, "file:write:  File handle is not open for writing");
            return 2;
        }

        int stk = lua_gettop(lua);
        std::string v;

        for(int i = 2; i <= stk; ++i)
        {
            if(!lua_isstring(lua,i))
            {
                lua_pushnil(lua);
                lua.pushString("file:write:  Parameter " + std::to_string(i) + " is not a string or number");
                return 2;
            }
            v = lua.toString(i);
            if( file.write(v.data(), v.size()) < 0)
            {
                lua_pushnil(lua);
                lua.pushString( "Failure in file:write: '" + file.errorString().toStdString() + "'" );
                return 2;
            }
        }

        // if we reached here, we have successfully written all values
        lua_settop(lua, 1);     // pop everything except for the 'this' object
        return 1;               // and return that object
    }

    /////////////////////////////////////////////////
    //  Reading is a pain in the arse

    int LuaIOFile::lua_read(Lua& lua)
    {
        if(!file.isReadable())
        {
            lua_pushnil(lua);
            return 1;
        }

        ///////////////////
        int stk = lua_gettop(lua);
        if(stk == 1)            // no params
        {
            lua_read_l(lua, false);
            return 1;
        }

        // otherwise, we have params!
        int values_pushed = 0;
        std::string m;
        for(int i = 2; i <= stk; ++i)
        {
            if(lua_isinteger(lua, i))
            {
                ++values_pushed;
                if( lua_read_num(lua, lua_tointeger(lua, i)) )      break;
            }
            else if(lua_type(lua, i) != LUA_TSTRING)
                throw Error( "file:read:  Expected parameter " + std::to_string(i) + " to be an integer or a string" );
            else
            {
                ++values_pushed;
                m = lua.toString(i);
                if     (m == "l") {     if(lua_read_l(lua, false))      break;  }
                else if(m == "L") {     if(lua_read_l(lua, true))       break;  }
                else if(m == "a") {     if(lua_read_a(lua))             break;  }
                else if(m == "n")       throw Error( "file:read:  'n' read mode is not currently supported.  Sorry");   // TODO support this eventually maybe
                else                    throw Error( "file:read:  Unrecognized read mode '" + m + "'" );
            }
        }

        return values_pushed;
    }
    
    ///////////////////////////////////////////
    //  These support functions return true if 'nil' was pushed (requiring that lua_read exit without 
    //     doing any further reading)

    bool LuaIOFile::lua_read_a(Lua& lua)
    {
        // unlike other functions, we do not push nil if at EOF
        auto dat = file.readAll();
        lua_pushlstring(lua, dat.data(), dat.size());
        return false;
    }

    bool LuaIOFile::lua_read_l(Lua& lua, bool keepnewline)
    {
        // Qt has a way to read individual lines, but it doesn't really document what it does with the line breaks
        //  so I do my own thing here.
        
        // if at EOF, push nil, always
        if(file.atEnd())
        {
            lua_pushnil(lua);
            return true;
        }

        //  I peek then read here... which is redundant... but because of BS text file mode, seeking
        //    is unreliable.
        
        std::string out;
        constexpr qint64 bufsize = 100;
        char buffer[bufsize];

        while(true)
        {
            // peek in 'bufsize' sized chunks
            auto bytes = file.peek(buffer, bufsize);

            if(!bytes)      // no more data = we're done
                break;

            // Search for the newline
            int nl;
            for(nl = 0; (nl < bytes) && (buffer[nl] != '\n'); ++nl) {}

            if(nl == bytes)     // we didn't find a newline
            {
                bytes = file.read(buffer, nl);
                if(bytes < 0)  bytes = 0;

                out.append( buffer, static_cast<std::size_t>(bytes) );
            }
            else                // we DID find a newline
            {
                bytes = file.read(buffer, nl+1);
                if(!keepnewline)
                    --bytes;
                if(bytes < 0)  bytes = 0;
                
                out.append( buffer, static_cast<std::size_t>(bytes) );
                break;
            }
        }

        lua.pushString(out);
        return false;
    }

    bool LuaIOFile::lua_read_num(Lua& lua, lua_Integer num)
    {
        // push nil if at end of file -- always
        // otherwise, if num == 0, push empty string
        // otherwise, if num > 0, read that many bytes and push as a string
        // The Lua spec does not indicate what happens if num<0, so treat it same as zero

        if(file.atEnd())
        {
            lua_pushnil(lua);
            return true;
        }
        else if(num <= 0)
        {
            lua_pushliteral(lua, "");
            return false;
        }
        else
        {
            auto dat = file.read(num);
            lua_pushlstring(lua, dat.data(), dat.size());
            return false;
        }
    }
}


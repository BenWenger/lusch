
#include "luaobj.h"
#include "luastacksaver.h"
#include "luauserdata.h"

#include <cstring>              // for strlen

namespace lsh
{
    namespace
    {
        ////////////////////////////////////////////
        //  Interface for progressively reading a file into Lua

        class LuaFileReader
        {
        private:
            static const int buffersize = 1000;

            QIODevice*      io;
            char            buffer[buffersize];
            
        public:
            static const char* read(lua_State* L, void* data, size_t* size)
            {
                auto rdr = reinterpret_cast<LuaFileReader*>(data);

                int bytes = static_cast<int>( rdr->io->read(rdr->buffer, buffersize) );
                if(bytes <= 0)
                {
                    *size = 0;
                    return nullptr;
                }
                else
                {
                    *size = static_cast<size_t>(bytes);
                    return rdr->buffer;
                }
            }

            LuaFileReader(QIODevice& dev) : io(&dev) {}
        };
    }

    Lua::Lua(Lua&& rhs)
    {
        L = rhs.L;
        rhs.L = nullptr;
    }

    Lua& Lua::operator = (Lua&& rhs)
    {
        if(L)
            lua_close(L);
        L = rhs.L;
        rhs.L = nullptr;
        return *this;
    }

    Lua::~Lua()
    {
        if(L)
            lua_close(L);
    }

    Lua::Lua()
    {
        L = luaL_newstate();
        if(!L)
            throw std::bad_alloc();

        buildEnvironment();
    }

    void Lua::reset()
    {
        Lua tmp;
        *this = std::move(tmp);
    }

    void Lua::assertReady()
    {
        if(!L)
            L = lua_newstate(nullptr, nullptr);
    }
    
    void Lua::pushFunction(const function_t& func)
    {
        LuaFunctionWrapper::pushFunction(*this, func);
    }

    //////////////////////////////////////////////////////
    //  Calling a function!
    int Lua::call(int numParams, int numrets /*= LUA_MULTRET*/)
    {
        // Get the expected 'zero' stack size.  We will use this to determine how many values
        //   were returned by the function
        int zeroStackSize = lua_gettop(L) - numParams - 1;  // lua_pcall will pop all params and the function

        // TODO - figure out the message handler business
        int result = lua_pcall(L, numParams, numrets, 0);

        // TODO handle errors better than this
        if(result != LUA_OK)
        {
            lua_pop(L, 1);
            throw Error("A Lua Error occurred");
        }

        return lua_gettop(L) - zeroStackSize;
    }

    //////////////////////////////////////////////////////
    //  Stuff for strings
    std::string Lua::getString(int idx)
    {
        LuaStackSaver       stk(L);

        lua_pushvalue(L, idx);

        std::string out;

        std::size_t siz = 0;
        auto rawstr = lua_tolstring(L, -1, &siz);
        if(!rawstr)     throw Error("Internal error:  Lua::getString called before verifying value was a string");

        out.resize(siz);
        std::copy(rawstr, rawstr + siz, out.begin());
        return out;
    }

    void Lua::pushString(const std::string& str)
    {
        lua_pushlstring(L, str.data(), str.size());
    }
    
    void Lua::errIfTooFewArgs(const char* funcname, int argmin)
    {
        // TODO
    }

    void Lua::warnIfTooManyArgs(const char* funcname, int argmax)
    {
        // TODO
    }

    //////////////////////////////////////////////////////
    //  File/code loading
    
    void Lua::addLuaFile(const QString& filename, QIODevice& file)
    {
        auto str = filename.toStdString();
        pushLuaCodeFromFile(str.c_str(), file);
        call(0,0);
    }

    void Lua::addInternalCode(const char* str)
    {
        pushLuaCodeFromString("(Internal Code)", str);
        call(0,0);
    }

    void Lua::doComplexString(const std::string& rawstring, const char* str)
    {
        auto desc = "[Complex String] " + rawstring;
        pushLuaCodeFromString(desc.c_str(), str);
        call(0,0);
    }

    void Lua::pushLuaCodeFromFile(const char* name, QIODevice& file)
    {
        LuaStackSaver stk(L);
        
        LuaFileReader rdr(file);
        int result = lua_load(L, &LuaFileReader::read, reinterpret_cast<void*>(&rdr), name, nullptr);

        if(result != LUA_OK)
        {
            // TODO - handle errors better than this
            throw Error("A Lua Load Error occurred");
        }

        stk.escape();
    }

    void Lua::pushLuaCodeFromString(const char* name, const char* str)
    {
        LuaStackSaver stk(*this);
        int result = luaL_loadbufferx(*this, str, std::strlen(str), name, nullptr);

        if(result != LUA_OK)
        {
            // TODO - handle errors better than this
            throw Error("A Lua Load Error occurred");
        }

        stk.escape();
    }
}
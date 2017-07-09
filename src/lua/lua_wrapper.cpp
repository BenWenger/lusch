
#include "lua_wrapper.h"
#include "lua_stacksaver.h"

#include "objects/lua_object.h"
#include "log.h"
#include <QIODevice>

namespace lsh
{
    Lua::Lua()
    {
        L = luaL_newstate();
        if(!L)                  throw std::bad_alloc();

        try
        {
            addBinding(L);
            buildLuaEnvironment();
        }
        catch(...)
        {
            lua_close(L);
            throw;
        }
    }

    Lua::~Lua()
    {
        if(L)
            lua_close(L);
    }

    Lua::Lua(Lua&& rhs)
    {
        L = nullptr;
        *this = std::move(rhs);
    }

    Lua& Lua::operator = (Lua&& rhs)
    {
        if(L)
            lua_close(L);
        
        moveBindings(std::move(rhs));
        L = rhs.L;
        rhs.L = nullptr;

        return *this;
    }
    
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    void Lua::pushString(const std::string& str)
    {
        assertActive();
        lua_pushlstring(L, str.data(), str.size());
    }

    std::string Lua::toString(int index)
    {
        assertActive();
        LuaStackSaver stk(L);

        // don't use lua_tolstring directly, as the string conversion can confuse lua_next...
        //   so duplicate the entry to the top of the stack first
        lua_pushvalue(L, index);

        std::size_t     siz = 0;
        const char* ptr = lua_tolstring(L, -1, &siz);
        
        return std::string(ptr, siz);
    }

    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    void Lua::checkTooManyParams(int maxparams, const char* func_name)
    {
        if(lua_gettop(L) > maxparams)
            throw Error( std::string("Too many parameters passed to '") + func_name + "'. Expected a maximum of " + std::to_string(maxparams) + " params." );
    }
    
    void Lua::checkTooFewParams(int minparams, const char* func_name)
    {
        if(lua_gettop(L) < minparams)
            throw Error( std::string("Too few parameters passed to '") + func_name + "'. Expected a minimum of " + std::to_string(minparams) + " params." );
    }
    
    lua_Integer Lua::getIntParam(int index, const char* func_name)
    {
        if(!lua_isinteger(L,index))
            throw Error( std::string("In function '") + func_name + "', expected parameter " + std::to_string(index) + " to be an integer" );
        return lua_tointeger(L, index);
    }
    
    lua_Integer Lua::getIntParam(int index, const char* func_name, lua_Integer defoption)
    {
        if( lua_isnoneornil(L, index) )     return defoption;
        return getIntParam(index, func_name);
    }

    std::string Lua::getStringParam(int index, const char* func_name)
    {
        if(!lua_isstring(L,index))
            throw Error( std::string("In function '") + func_name + "', expected parameter " + std::to_string(index) + " to be a string" );
        return toString(index);
    }
    
    std::string Lua::getStringParam(int index, const char* func_name, const std::string& defoption)
    {
        if( lua_isnoneornil(L, index) )     return defoption;
        return getStringParam(index, func_name);
    }
        
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    namespace
    {
        int luaMessageHandler(lua_State* L)
        {
            luaL_traceback( L, L, lua_tostring(L, -1), 1);
            return 1;
        }
    }
    
    int Lua::callFunction(int nparams, int nrets)
    {
        if(lua_type(L, -1) != LUA_TFUNCTION)
            throw Error("Internal Error:  Lua::callFunction is called without having a function on the top of the stack.");

        int expectedZero = lua_gettop(L) - nparams - 1;
        if(expectedZero < 0)
            throw Error("Internal Error:  Not enough values pushed to the stack in Lua::callFunction");
        
        LuaStackSaver stk(expectedZero, L);

        /*
        int msgh = expectedZero + 1;

        lua_pushcfunction( L, &luaMessageHandler );             // cram the message handler in there
        lua_insert( L, msgh );

        handleLuaError( lua_pcall( L, nparams, nrets, msgh ) );

        lua_remove( L, msgh );                                  // remove the message handler
        */
        handleLuaError( lua_pcall( L, nparams, nrets, 0 ) );

        stk.escape();
        return lua_gettop(L) - expectedZero;
    }

    int Lua::callGlobalFunction(const char* funcname, int nparams, int nrets)
    {
        if(lua_getglobal(L, funcname) != LUA_TFUNCTION)
        {
            Log::wrn("'" + std::string(funcname) + "' is not a global function name.");
        }
        else
        {
            return callFunction(nparams,nrets);
        }
        return 0;
    }

    
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////
    //  File loading
    namespace
    {
        class LuaReader
        {
        public:
            static int go(lua_State* L, QIODevice& file, const char* filename)
            {
                LuaReader reader(file);
                return lua_load(L, &LuaReader::callback, reinterpret_cast<void*>(&reader), filename, nullptr);
            }

        private:
            static constexpr int    bufsize = 2000;
            QIODevice&              file;
            char                    buffer[bufsize];

            LuaReader(QIODevice& f) : file(f) {}

            static const char* callback(lua_State* L, void* data, std::size_t* size)
            {
                auto* obj = reinterpret_cast<LuaReader*>(data);

                auto bytes = obj->file.read(obj->buffer, bufsize);
                if(bytes <= 0)
                {
                    *size = 0;
                    return nullptr;
                }

                *size = static_cast<std::size_t>(bytes);
                return obj->buffer;
            }
        };
    }

    void Lua::loadScript(QIODevice& file, const char* filename)
    {
        LuaStackSaver stk(L);


        handleLuaError( LuaReader::go(L, file, filename) );
        callFunction(0,0);
    }

    void Lua::handleLuaError(int code)
    {
        if(code == LUA_OK)      return;
        if(code == LUA_ERRMEM)  throw std::bad_alloc();

        std::string msg = "<No error message provided by Lua>";
        if(lua_isstring(L, -1))
            msg = lua_tostring(L, -1);

        switch(code)
        {
        case LUA_ERRSYNTAX:     throw Error( "Lua Syntax Error:  " + msg );
        case LUA_ERRGCMM:       throw Error( "Internal error occurred during Lua garbage collection:  " + msg );
        case LUA_ERRRUN:        throw Error( "Lua Error:  " + msg );
        case LUA_ERRERR:        throw Error( "Internal error occurred during message handler:  " + msg );
        }

        throw Error( "Unknown error reported from Lua:  " + msg );
    }


}
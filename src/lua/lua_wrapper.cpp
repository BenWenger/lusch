
#include "lua_wrapper.h"
#include "lua_stacksaver.h"

#include "objects/lua_object.h"

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
    
    void Lua::buildLuaEnvironment()
    {
        // TODO
    }

}
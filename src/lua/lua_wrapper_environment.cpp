
#include "lua_wrapper.h"
#include "lua_stacksaver.h"
#include "log.h"
#include <lua/lualib.h>
#include <vector>
#include <string>

namespace lsh
{
    namespace
    {
        //////////////////////////////////
        //  global tables to create (after processing whitelists)
        const char* const global_tables_to_create[] = {
            "io",
            "lsh"
        };

        struct WhitelistEntry
        {
            const char* name;
            bool istable;
        };

        ////////////////////////////////////////////////////
        //  Whitelists
        const WhitelistEntry whitelist_global[] = {
            //  need to reimplement:   print, [io]
            {"assert"       , false},
            {"error"        , false},
            {"_G"           , true },
            {"getmetatable" , false},
            {"ipairs"       , false},
            {"next"         , false},
            {"pairs"        , false},
            {"pcall"        , false},
            {"rawequal"     , false},
            {"rawget"       , false},
            {"rawlen"       , false},
            {"rawset"       , false},
            {"select"       , false},
            {"setmetatable" , false},
            {"tonumber"     , false},
            {"tostring"     , false},
            {"type"         , false},
            {"_VERSION"     , false},
            {"xpcall"       , false},
            {"string"       , true },
            {"utf8"         , true },
            {"table"        , true },
            {"math"         , true },
            {"os"           , true }
        };
        
        const WhitelistEntry whitelist_string[] = {
            //  need to reimplement:   <none>
            {"byte"         , false},
            {"char"         , false},
            {"find"         , false},
            {"format"       , false},
            {"gmatch"       , false},
            {"gsub"         , false},
            {"len"          , false},
            {"lower"        , false},
            {"match"        , false},
            {"pack"         , false},
            {"packsize"     , false},
            {"rep"          , false},
            {"reverse"      , false},
            {"packsize"     , false},
            {"sub"          , false},
            {"unpack"       , false},
            {"upper"        , false}
        };
        
        const WhitelistEntry whitelist_utf8[] = {
            //  need to reimplement:   <none>
            {"char"         , false},
            {"charpattern"  , false},
            {"codes"        , false},
            {"codepoint"    , false},
            {"len"          , false},
            {"offset"       , false}
        };
        
        const WhitelistEntry whitelist_table[] = {
            //  need to reimplement:   <none>
            {"concat"       , false},
            {"insert"       , false},
            {"move"         , false},
            {"pack"         , false},
            {"remove"       , false},
            {"sort"         , false},
            {"unpack"       , false}
        };
        
        const WhitelistEntry whitelist_math[] = {
            //  need to reimplement:   <none>
            {"abs"          , false},
            {"acos"         , false},
            {"asin"         , false},
            {"atan"         , false},
            {"ceil"         , false},
            {"cos"          , false},
            {"deg"          , false},
            {"exp"          , false},
            {"floor"        , false},
            {"fmod"         , false},
            {"huge"         , false},
            {"log"          , false},
            {"max"          , false},
            {"maxinteger"   , false},
            {"min"          , false},
            {"mininteger"   , false},
            {"modf"         , false},
            {"pi"           , false},
            {"rad"          , false},
            {"random"       , false},
            {"randomseed"   , false},
            {"sin"          , false},
            {"sqrt"         , false},
            {"tan"          , false},
            {"tointeger"    , false},
            {"type"         , false},
            {"ult"          , false}
        };
        
        const WhitelistEntry whitelist_os[] = {
            //  need to reimplement:   remove, rename
            {"clock"        , false},
            {"date"         , false},
            {"difftime"     , false},
            {"time"         , false}
        };


        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////

        void filterByWhitelist( Lua& lua, const WhitelistEntry* whitelist, int whitelistsize )
        {
            std::vector<std::string>    itemsToBlacklist;
            std::string v;

            {
                LuaStackSaver stk(lua);

                //  Assumes the table to filter has already been pushed onto the stack
                lua_pushnil(lua);
                while(lua_next(lua, -2))
                {
                    v = lua.toString(-2);
                    bool good = false;
                    for(int i = 0; i < whitelistsize; ++i)
                    {
                        if(v == whitelist[i].name)
                        {
                            if(whitelist[i].istable || (lua_type(lua,-1) != LUA_TTABLE))
                                good = true;
                            break;
                        }
                    }

                    if(!good)
                        itemsToBlacklist.push_back(v);

                    lua_pop(lua, 1);
                }
            }

            // now we have our blacklist (everything that wasn't in the whitelist) - remove all those
            for(auto& i : itemsToBlacklist)
            {
                lua_pushnil(lua);
                lua_setfield(lua, -2, i.c_str());
            }
        }

        template <int S>
        void whitelistFilter( Lua& lua, const char* tablename, const WhitelistEntry (&whitelist)[S] )
        {
            LuaStackSaver stk(lua);
            if(!tablename)          lua_pushglobaltable(lua);
            else                    lua_getglobal(lua, tablename);

            if(lua_type(lua, -1) == LUA_TTABLE)
                filterByWhitelist(lua, whitelist, S);
        }
    }


    void Lua::buildLuaEnvironment()
    {
        LuaStackSaver stk(L);

        ////////////////////////
        //  Load main libraries
        luaL_openlibs(L);

        // but filter them so insecure stuff is omitted
        whitelistFilter( *this, nullptr,    whitelist_global    );
        whitelistFilter( *this, "string",   whitelist_string    );
        whitelistFilter( *this, "utf8",     whitelist_utf8      );
        whitelistFilter( *this, "table",    whitelist_table     );
        whitelistFilter( *this, "math",     whitelist_math      );
        whitelistFilter( *this, "os",       whitelist_os        );

        // Then add a few global tables (to be populated by other areas of Lusch)
        for(auto& i : global_tables_to_create)
        {
            lua_newtable(L);
            lua_setglobal(L, i);
        }
    }

}

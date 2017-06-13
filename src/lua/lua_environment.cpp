
#include "luaobj.h"
#include "luastacksaver.h"

namespace lsh
{
    void Lua::buildEnvironment()
    {
        // Load common libraries
        loadLib_Base();
        //loadLib_Coroutine();      Coroutines not supported
        //loadLib_Package();        External packages not supported
        loadLib_String();
        loadLib_Utf8();
        loadLib_Table();
        loadLib_Math();
        loadLib_Io();
        loadLib_Os();
        //loadLib_Debug();          Debug routines not supported

        // Load the 'lsh' library
        loadLib_Lusch();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    void Lua::loadLib_Base()
    {
        // Call the main loader
        lua_pushcfunction( L, luaopen_base );       call(0,0);

        // Blacklist some stuff that isn't secure
        static const char* blacklist = "\
            collectgarbage  = nil\n\
            dofile          = nil\n\
            getmetatable    = nil\n\
            load            = nil\n\
            loadfile        = nil\n\
            print           = nil\n\
            ";
        addInternalCode(blacklist);
    }
    
    void Lua::loadLib_String()
    {
        // Call the main loader
        lua_pushcfunction( L, luaopen_string );     call(0,0);

        // Blacklist some stuff that isn't secure
        static const char* blacklist = "\
            string.dump     = nil\n\
            ";
        addInternalCode(blacklist);
    }
    
    void Lua::loadLib_Utf8()
    {
        // Call the main loader   (This lib is safe, no sanitation needed)
        lua_pushcfunction( L, luaopen_utf8 );       call(0,0);
    }
    
    void Lua::loadLib_Table()
    {
        // Call the main loader   (This lib is safe, no sanitation needed)
        lua_pushcfunction( L, luaopen_table );      call(0,0);
    }
    
    void Lua::loadLib_Math()
    {
        // Call the main loader   (This lib is safe, no sanitation needed)
        lua_pushcfunction( L, luaopen_math );       call(0,0);
    }
    
    // IO lib can't be imported -- it needs to be recreated.
    //    But since opening files relies on project data, io.open will be supplied by the
    // Project class
    void Lua::loadLib_Io()
    {
        LuaStackSaver stk(L);
        lua_newtable(L);
        lua_setglobal(L, "io");
    }
    
    void Lua::loadLib_Os()
    {
        // Call the main loader
        lua_pushcfunction( L, luaopen_os );         call(0,0);

        // Blacklist some stuff that isn't secure
        static const char* blacklist = "\
            os.execute          = nil\n\
            os.exit             = nil\n\
            os.getenv           = nil\n\
            os.remove           = nil\n\
            os.rename           = nil\n\
            os.setlocale        = nil\n\
            os.tmpname          = nil\n\
            ";
        addInternalCode(blacklist);
    }

    // Lsh stuff will also be supplied by other areas.  Just make the table now
    void Lua::loadLib_Lusch()
    {
        LuaStackSaver stk(L);
        lua_newtable(L);
        lua_setglobal(L, "lsh");
    }

}
#ifndef LUSCH_LUA_LUA_WRAPPER_H_INCLUDED
#define LUSCH_LUA_LUA_WRAPPER_H_INCLUDED
/*

    Join me in my adventures in attempting to make Lua work with my RAII and OOP desires.

    My goals were as follows:

    - Have an RAII wrapper class (defined in this file) which has support functions to make dealing with
        strings and other things easier.
    - Wrap all Lua callback code in try/catch blocks to prevent exceptions from spilling into the Lua source
        where it might not be able to appropriately handle it
    - Change the signature of callbacks to take my Lua wrapper class as the parameter rather than
        the lua_State* pointer.
    - Allow for objects that share ownership between the C++ code, and the Lua garbage collector (objects
        not destroyed until both areas are done with them)
    - Allow for callbacks to be member functions rather than globals .. for things like file:open (which
        needs an associated File type object) or the lsh.get / lsh.set functions (which need an associated
        project)

    Shared ownership of objects was pretty simple.  This can be illustrated by lua/objects/lua_object.h.
    See that file for details.

    The callback changes were the most difficult.  Changing the signature and allowing for member functions turned
    out to be a lot of work.  Lua has support for "upvalues" which allowed you to bind external information to
    callbacks, but they are restricted to types that Lua recognizes... and custom types ("user data" and "light
    user data") are restricted to void pointers.  So there is no way to push a function pointer as an upvalue,
    which means functions need to be stored in memory outside of Lua.

    Using the upvalues to assiciate an object COULD work.  For example, lsh.get and lsh.set need an associated
    Project object to know where to get/set the data from.  So I could have just pushed a pointer to the Project
    as an upvalue, but this had two major flaws:
        1)  The Project object could not be moved/recreated or that pointer would go bad.  This is a problem if
            I want to do "build then swap" techniques with Projects -- which are generally advantageous because
            they're more exception safe (and I'm using lots of exceptions for error handling)
        2)  It would require my Project stay alive for at least as long as the Lua (otherwise the pointer will
            go bad), which probably would be the case, but is an easy thing to screw up and an unnecessary risk.


    So upvalues are out.  Can't put the Project as an upvalue, can't put callbacks as an upvalue, and those are
    the two things I need to track.

    Enter the 'LuaFunction' class, which is effectively a global registry of callbacks.  The idea being, all
    code (outside of the Lua wrapper or LuaObject base classes) should push their callbacks via the LuaFunction
    interface.  See that file for details.

 */


#include <lua/lua.h>
#include <lua/lauxlib.h>
#include "lua_binding.h"
#include "error.h"

class QIODevice;

namespace lsh
{

    class Lua : public LuaBinding<Lua>
    {
    public:
        /////////////////////////////////
        //  Ctors and Dtors  (moving allowed, but no copying)
        Lua();
        ~Lua();
        Lua(Lua&&);
        Lua& operator = (Lua&&);
        Lua(const Lua&)                 = delete;
        Lua& operator = (const Lua&)    = delete;

        // Implicit cast to lua_State so we can use standard lua functions
        inline operator lua_State* ()           { assertActive();       return L;   }

        // Calling functions!
        int             callFunction(int nparams, int nrets);
        int             callGlobalFunction(const char* funcname, int nparams, int nrets);

        // String shit
        void            pushString(const std::string& str);
        std::string     toString(int index);

        // Parameter shit
        void            checkTooManyParams(int maxparams, const char* func_name);
        void            checkTooFewParams(int minparams, const char* func_name);

        std::string     getStringParam(int index, const char* func_name);
        std::string     getStringParam(int index, const char* func_name, const std::string& defoption);
        lua_Integer     getIntParam(int index, const char* func_name);
        lua_Integer     getIntParam(int index, const char* func_name, lua_Integer defoption);

        void            loadScript(QIODevice& file, const char* filename);

    private:
        lua_State*      L;

        inline void     assertActive() { if(!L) throw Error("Internal Error:  Lua object used after state was moved");  }

        
        void            handleLuaError(int code);

        /////////////////////////////////////
        //  Defined in lua_wrapper_environment.cpp
        void            buildLuaEnvironment();
    };
}

#endif
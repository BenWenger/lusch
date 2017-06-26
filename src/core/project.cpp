
#include <QDir>
#include <QMessageBox>
#include "lua/lua_wrapper.h"
#include "lua/lua_stacksaver.h"
#include "lua/lua_function.h"
#include "project.h"
#include "projectdata.h"
#include "lua/objects/lua_iofile.h"
#include "util/filename.h"

namespace lsh
{
    Project::Project()
    {
        loaded = false;
        dirty  = false;

        if( LuaFunction::isBoundedListEmpty<Project>() )
        {
            LuaFunction::addBounded<Project>("io.open", &Project::lua_openFile);
            LuaFunction::addBounded<Project>("lsh.get", &Project::lua_getData);
            LuaFunction::addBounded<Project>("lsh.set", &Project::lua_setData);
        }
    }

    void Project::bindToLua(Lua& lua)
    {
        addBinding(lua);

        LuaStackSaver stk(lua);

        if(lua_getglobal(lua, "io") != LUA_TTABLE)      throw Error("Internal error:  global 'io' Lua symbol is not a table");
        LuaFunction::pushBounded<Project>(lua, "io.open");
        lua_setfield(lua, -2, "open");
        lua_pop(lua, 1);                // drop the "io" table

        
        if(lua_getglobal(lua, "lsh") != LUA_TTABLE)     throw Error("Internal error:  global 'lsh' Lua symbol is not a table");
        LuaFunction::pushBounded<Project>(lua, "lsh.get");
        lua_setfield(lua, -2, "get");
        LuaFunction::pushBounded<Project>(lua, "lsh.set");
        lua_setfield(lua, -2, "set");
        lua_pop(lua, 1);                // drop the "lsh" table
    }

    int Project::lua_openFile(Lua& lua)
    {
        lua.checkTooFewParams(1, "io.open");
        lua.checkTooManyParams(2, "io.open");

        std::string name;
        std::string mode = "r";

        if(lua_type(lua, 1) == LUA_TSTRING)     name = lua.toString(1);
        else                                    throw Error("In 'io.open', expected parameter 1 to be a string");

        if(lua_gettop(lua) >= 2)
        {
            if(lua_type(lua, 2) == LUA_TSTRING)     mode = lua.toString(2);
            else                                    throw Error("In 'io.open', expected parameter 2 to be a string");
        }

        FileFlags flgs;
        if(!flgs.fromStringMode(mode))              throw Error("Invalid mode string '" + mode + "' passed to io.open");

        name = translateFileName(name);
        verifyFileOpenPath(name, flgs.write);

        return LuaIOFile::openForLua(lua, name, flgs);
    }
    
    std::string Project::translateFileName(const std::string& givenname)
    {
        std::string path;

        std::string prefix;
        std::string suffix;

        auto i = givenname.find('$');
        if(i == std::string::npos)
            prefix = givenname;
        else
        {
            prefix = givenname.substr(0, i);
            suffix = givenname.substr(i+1);
        }

        // Everything left of the $ is a file ID.  Look the file ID up and replace it with the actual file path
        auto item = fileInfo.find(prefix);
        if(item == fileInfo.end())
            throw Error("io.open:  '" + prefix + "' is not a recognized file ID");

        return FileName(projectFileDir, prefix, suffix);
    }
    
    void Project::verifyFileOpenPath(const std::string& name, bool writable)
    {
        bool ok = false;

        for(auto& i : fileInfo)
        {
            bool found = false;

            if(i.second.directory)
            {
                found = ( name.substr(0, i.second.filePath.length()) == i.second.filePath );
            }
            else
            {
                found = ( name == i.second.filePath );
            }

            if(found)
            {
                if(!writable || i.second.writable)
                {
                    ok = true;
                    break;
                }
            }
        }

        if(!ok)
            throw Error( "File: '" + name + "' could not be opened for " + (writable ? "writing" : "reading") + ". It is not part of the project file list." );
    }

    // TODO finish this
    int         Project::lua_setData(Lua& lua) { return 0; }
    int         Project::lua_getData(Lua& lua) { return 0; }

    
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////

    void Project::makeDirty()
    {
        if(dirty)       return;

        dirty = true;
        emit dirtyChanged(true);
    }
    
    bool Project::promptIfDirty(QWidget* parent, const char* prompt)
    {
        if(!loaded)     return true;
        if(!dirty)      return true;

        auto answer = QMessageBox::warning(parent, "Are you sure?", prompt,
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                           QMessageBox::Yes );

        switch(answer)
        {
        case QMessageBox::No:       return true;
        case QMessageBox::Yes:      /* save */ break;
        default:                    return false;
        }

        return !dirty;
    }


}

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

        bool waswritable;
        auto filename = translateFileName(name, waswritable);

        if(flgs.write && !waswritable)              throw Error("File '" + name + "' is marked in the project as read-only and cannot be opened for writing.");

        return LuaIOFile::openForLua(lua, filename.getFullPath(true), flgs);
    }
    
    FileName Project::translateFileName(const std::string& givenname, bool& waswritable)
    {
        // If there's a '/' in the given name, everything left of the / is a directory ID
        // Otherwise, the whole thing is a file ID

        auto slsh = givenname.find('/');
        std::string id = givenname.substr(0,slsh);

        // Find the ID in the files list
        auto item = fileInfo.find(id);
        if(item == fileInfo.end())          throw Error("Unrecognized file ID:  '" + id + "'");

        FileName out;

        if(slsh == givenname.npos)
        {
            // Normal file?
            if(item->second.directory)      throw Error("File ID '" + id + "' is a directory, not a file.  If you meant to use the directory, append a slash (/).");
            out = item->second.fileName;
            out.makeAbsoluteWith( projectFileName );
        }
        else
        {
            // Directory
            if(!item->second.directory)     throw Error("File ID '" + id + "' is a file, not a directory.  You cannot use a slash (/) when accessing this file.");
            FileName base = item->second.fileName;
            base.makeAbsoluteWith( projectFileName );

            out.setFullPath( givenname.substr(slsh+1) );
            if(out.beginsWithDoubleDot())   throw Error("Given file '" + givenname + "' moves outside of directory indicated by '" + id + "'.  This directory is inaccessible.");
            out.makeAbsoluteWith( base );
        }

        waswritable = item->second.writable;
        return out;
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
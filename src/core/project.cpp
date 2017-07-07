
#include <QDir>
#include <QMessageBox>
#include "lua/lua_wrapper.h"
#include "lua/lua_stacksaver.h"
#include "lua/lua_function.h"
#include "project.h"
#include "projectdata.h"
#include "lua/objects/lua_iofile.h"
#include "util/filename.h"
#include "blueprint.h"
#include "log.h"
#include "util/safecall.h"
#include "versioninfo.h"

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
    
    Project& Project::operator = (Project&& rhs)
    {
        moveBindings(rhs);
        blueprint =             std::move(rhs.blueprint);
        projectFileName =       std::move(rhs.projectFileName);
        bpFileName =            std::move(rhs.bpFileName);
        fileInfoIndexes =       std::move(rhs.fileInfoIndexes);
        dat =                   std::move(rhs.dat);
        loaded =                rhs.loaded;
        dirty =                 rhs.dirty;
        savePretty =            rhs.savePretty;
        saveCompressed =        rhs.saveCompressed;

        // TODO - need to emit a signal that causes all project data ties to be rebound.

        rhs.loaded = rhs.dirty = false;
        return *this;
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
        lua.checkTooManyParams(3, "io.open");

        std::string name;
        std::string mode = "r";
        bool mustopen = false;

        if(lua_type(lua, 1) == LUA_TSTRING)     name = lua.toString(1);
        else                                    throw Error("In 'io.open', expected parameter 1 to be a string");

        if(lua_gettop(lua) >= 2)
        {
            if(lua_type(lua, 2) == LUA_TSTRING)     mode = lua.toString(2);
            else                                    throw Error("In 'io.open', expected parameter 2 to be a string");
        }
        if(lua_gettop(lua) >= 3)
        {
            if(lua_type(lua, 3) == LUA_TBOOLEAN)    mustopen = !!lua_toboolean(lua,3);
            else                                    throw Error("In 'io.open', expected parameter 3 to be a boolean");
        }

        FileFlags flgs;
        if(!flgs.fromStringMode(mode))              throw Error("Invalid mode string '" + mode + "' passed to io.open");

        bool waswritable;
        auto filename = translateFileName(name, waswritable);

        if(flgs.write && !waswritable)              throw Error("File '" + name + "' is marked in the project as read-only and cannot be opened for writing.");

        return LuaIOFile::openForLua(lua, filename.getFullPath(true), flgs, mustopen);
    }
    
    FileName Project::translateFileName(const std::string& givenname, bool& waswritable)
    {
        // If there's a '/' in the given name, everything left of the / is a directory ID
        // Otherwise, the whole thing is a file ID

        auto slsh = givenname.find('/');
        std::string id = givenname.substr(0,slsh);

        // Find the ID in the files list
        auto itemIndex = fileInfoIndexes.find(id);
        if(itemIndex == fileInfoIndexes.end())      throw Error("Unrecognized file ID:  '" + id + "'");
        auto& item = blueprint.files[itemIndex->second];

        FileName out;

        if(slsh == givenname.npos)
        {
            // Normal file?
            if(item.directory)              throw Error("File ID '" + id + "' is a directory, not a file.  If you meant to use the directory, append a slash (/).");
            out = item.fileName;
            out.makeAbsoluteWith( projectFileName );
        }
        else
        {
            // Directory
            if(!item.directory)             throw Error("File ID '" + id + "' is a file, not a directory.  You cannot use a slash (/) when accessing this file.");
            FileName base = item.fileName;
            base.makeAbsoluteWith( projectFileName );

            out.setFullPath( givenname.substr(slsh+1) );
            if(out.beginsWithDoubleDot())   throw Error("Given file '" + givenname + "' moves outside of directory indicated by '" + id + "'.  This directory is inaccessible.");
            out.makeAbsoluteWith( base );
        }

        waswritable = item.writable;
        return out;
    }

    int Project::lua_setData(Lua& lua)
    {
        lua.checkTooFewParams(2,"lsh.set");
        lua.checkTooManyParams(2,"lsh.set");

        if(lua_type(lua, 1) != LUA_TSTRING)     throw Error("lsh.set:  Parameter 1 must be a string");
        auto name = lua.toString(1);
        
        auto& item = dat[name];

        connect(&item, &ProjectData::dataChanged, this, &Project::dirtyByData, Qt::ConnectionType(Qt::DirectConnection | Qt::UniqueConnection) );


        switch( lua_type(lua, 2) )
        {
        case LUA_TNIL:          item.setNull();                         break;
        case LUA_TSTRING:       item.set( lua.toString(2) );            break;
        case LUA_TNUMBER:
            if(lua_isinteger(lua,2))    item.set( lua_tointeger(lua, 2) );
            else                        item.set( lua_tonumber (lua, 2) );
            break;
        case LUA_TBOOLEAN:      item.set( !!lua_toboolean(lua,2) );     break;

        case LUA_TUSERDATA:
            item.set( LuaObject::getPointerFromLuaStack(lua, 2, "lsh.set 2nd parameter") );
            break;

        default:
            throw Error(std::string("Unsupported type (") + lua_typename(lua,2) + ") passed to lsh.set");
        }

        return 0;
    }

    int Project::lua_getData(Lua& lua)
    {
        lua.checkTooFewParams(1,"lsh.get");
        lua.checkTooManyParams(1,"lsh.get");
        
        if(lua_type(lua, 1) != LUA_TSTRING)     throw Error("lsh.get:  Parameter 1 must be a string");
        auto name = lua.toString(1);

        auto i = dat.find(name);
        if(i == dat.end())          // not found, just return nil
        {
            lua_pushnil(lua);
            return 1;
        }

        auto& item = i->second;

        switch(item.getType())
        {
        case ProjectData::Type::Null:       lua_pushnil(lua);                       break;
        case ProjectData::Type::Bool:       lua_pushboolean(lua, item.asBool());    break;
        case ProjectData::Type::Int:        lua_pushinteger(lua, item.asInt());     break;
        case ProjectData::Type::Dbl:        lua_pushnumber(lua, item.asDbl());      break;
        case ProjectData::Type::Str:        lua.pushString(item.asString());        break;
        case ProjectData::Type::Obj:        item.asObj()->pushToLua(lua);           break;
        default:                            throw Error("Internal Error:  ProjectData '" + name + "' has unknown/unexpected type!");
        }

        return 1;
    }

    
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    
    int Project::doCallback(const char* callback_name, int params, int rets)
    {
        LuaStackSaver stk(blueprint.lua, -params);

        auto& x = blueprint.callbacks.find(callback_name);
        if(x != blueprint.callbacks.end())
        {
            if(lua_getglobal(blueprint.lua, x->second.c_str()) != LUA_TFUNCTION)
            {
                Log::wrn("'" + std::string(callback_name) + "' is not a global function name.");
            }
            else
            {
                return blueprint.lua.callFunction(0,rets);
            }
        }

        return 0;
    }

    void Project::makeDirty()
    {
        if(dirty)       return;

        dirty = true;
        emit projectStateChanged();
    }
    
    void Project::newProject(const FileName& projectPath, const FileName& bpPathRelative, Blueprint&& bp)
    {
        projectFileName = projectPath;
        bpFileName = bpPathRelative;

        blueprint = std::move(bp);
        for(auto& i : bp.files)
        {
            i.fileName.clear();
        }

        bindToLua(blueprint.lua);
    }

    void Project::doImport()
    {
        Lua& lua = blueprint.lua;
        LuaStackSaver stk(lua);

        /////////////////////////////////////////
        //  If there is a pre-import callback... call it
        int top = lua_gettop(lua);
        int params = doCallback("pre-import",0,LUA_MULTRET);

        try
        {
            //  Call each section's import function
            for(auto& x : blueprint.sections)
            {
                LuaStackSaver loopstk(lua);

                for(int i = 0; i < params; ++i)         lua_pushvalue(lua, top+i);
                doCallback(x.importFunc.c_str(), params, 0);
            }
        }
        catch(...)
        {
            // If an import failed, try to cleanup with the post-import callback
            BEGIN_SAFE
            doCallback("post-import", params, 0);
            END_SAFE
            throw;
        }

        /////////////////////////////////////
        //  Call the post-import if there is one
        doCallback("post-import", params, 0);
    }

    bool Project::doSave()
    {
        BEGIN_SAFE

            // TODO

        END_SAFE
        return false;
    }
}
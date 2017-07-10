#ifndef LUSCH_CORE_PROJECT_H_INCLUDED
#define LUSCH_CORE_PROJECT_H_INCLUDED

#include <stdexcept>
#include <QString>
#include <vector>
#include <unordered_map>
#include "util/qtjson.h"
#include "lua/lua_wrapper.h"
#include "projectdata.h"
#include "lua/lua_binding.h"
#include "util/filename.h"
#include "fileinfo.h"
#include "blueprint.h"


namespace lsh
{
    class Project : public QObject, public LuaBinding<Project>
    {
        Q_OBJECT

    public:
                    Project();
                    Project(const Project&) = delete;
        Project&    operator = (const Project&) = delete;
        Project&    operator = (Project&& rhs);

        bool        isDirty() const { return loaded && dirty; }

        void        newProject(const FileName& projectPath, const FileName& bpPathRelative, Blueprint&& bp);

        std::vector<FileInfo>&      getFileInfoArray()          { return blueprint.files;       }
        const FileName&             getProjectFileName() const  { return projectFileName;       }
        
        void        doImport();
        bool        doSave();

    signals:
        void        projectStateChanged();
        
    private:
        int         lua_openFile(Lua& lua);
        int         lua_setData(Lua& lua);
        int         lua_getData(Lua& lua);


    private:
        void        makeDirty();
        void        dirtyByData(ProjectData*)   { makeDirty();      }

        FileName    translateFileName(const std::string& name, bool& waswritable);
        void        bindToLua(Lua& lua);
        void        populateFileInfoIndexes();

        bool                loaded = false;
        bool                dirty = false;
        bool                savePretty = true;
        bool                saveAsTree = true;

        Blueprint                                       blueprint;
        FileName                                        projectFileName;
        FileName                                        bpFileName;
        std::unordered_map<std::string, std::size_t>    fileInfoIndexes;
        std::unordered_map<std::string, ProjectData>    dat;
        
        bool        pushCallback(const char* callback_name);


        json::object dataToJson() const;
    };

}

#endif
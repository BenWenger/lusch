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
#include "fileinfo.h"


namespace lsh
{
    class Project : public QObject, public LuaBinding<Project>
    {
        Q_OBJECT

    public:
                    Project();
                    Project(Project&&);
                    Project(const Project&) = delete;
        Project&    operator = (Project&&);
        Project&    operator = (const Project&) = delete;
        void        bindToLua(Lua& lua);

        bool        promptIfDirty(QWidget* parent, const char* prompt);

    signals:
        void        loadedChanged(bool loaded);
        void        dirtyChanged(bool dirty);

    public slots:
        void        makeDirty();

    private:
        int         lua_openFile(Lua& lua);
        int         lua_setData(Lua& lua);
        int         lua_getData(Lua& lua);


        std::string translateFileName(const std::string& name);
        void        verifyFileOpenPath(const std::string& name, bool writable);

        bool                loaded;
        bool                dirty;

        std::string                                         projectFileName;
        std::string                                         projectFileDir;
        std::unordered_map<std::string, FileInfo>           fileInfo;
        std::unordered_map<std::string, ProjectData>        dat;
    };

}

#endif
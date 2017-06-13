#ifndef LUSCH_CORE_BLUEPRINT_H_INCLUDED
#define LUSCH_CORE_BLUEPRINT_H_INCLUDED

#include <stdexcept>
#include <QString>
#include <vector>
#include "util/qtjson.h"
#include "lua/luaobj.h"

namespace lsh
{
    class DirTraverser;

    class Blueprint : public QObject
    {
        Q_OBJECT

    public:
                    Blueprint() = default;
        void        load(const QString& filename);
        void        unload();
        
        struct FileInfo
        {
            std::string     id;
            QString         displayName;
            bool            optional        = false;
            bool            directory       = false;
            bool            writable        = false;
        };

        struct SectionInfo
        {
            std::string     id;
            std::string     importFunc;
            std::string     exportFunc;     // optional
        };

    private:
                    Blueprint(DirTraverser& dir);
        void        doLoad(DirTraverser& dir);

        void        transferFromAnotherObject(Blueprint&& rhs);

        void        loadIndexFile(DirTraverser& dir);
        static FileInfo     loadFileInfoFromJson(const json::object& info);
        static SectionInfo  loadSectionInfoFromJson(const json::object& info);

        
        QString                     luschVersion;
        QString                     blueprintVersion;
        std::vector<FileInfo>       files;
        std::vector<SectionInfo>    sections;
    };

}

#endif
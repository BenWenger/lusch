#ifndef LUSCH_CORE_BLUEPRINT_H_INCLUDED
#define LUSCH_CORE_BLUEPRINT_H_INCLUDED

#include <stdexcept>
#include <QString>
#include <vector>
#include <unordered_map>
#include "util/qtjson.h"
#include "lua/lua_wrapper.h"
#include "fileinfo.h"
#include "util/filename.h"

namespace lsh
{
    class DirTraverser;

    class Blueprint
    {
    public:
                    Blueprint() = default;
                    Blueprint(const Blueprint&) = delete;
                    Blueprint(Blueprint&&) = default;
        Blueprint& operator = (const Blueprint&) = delete;
        Blueprint& operator = (Blueprint&&) = default;

        void        load(const FileName& filename);
        void        unload();

        struct SectionInfo
        {
            std::string     id;
            std::string     importFunc;
            std::string     exportFunc;     // optional
            bool            toImport = true;
            bool            toExport = true;
        };
        
        QString                                     luschVersion;
        QString                                     blueprintVersion;
        std::vector<FileInfo>                       files;
        std::vector<SectionInfo>                    sections;
        std::unordered_map<std::string,std::string> callbacks;
        Lua                                         lua;

    private:
                    Blueprint(DirTraverser& dir);
        void        doLoad(DirTraverser& dir);

        void        loadIndexFile(DirTraverser& dir);
        static FileInfo     loadFileInfoFromJson(const json::object& info);
        static SectionInfo  loadSectionInfoFromJson(const json::object& info);

    };

}

#endif
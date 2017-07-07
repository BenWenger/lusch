
#include "error.h"
#include "blueprint.h"
#include "util/dirtraverser_qdir.h"
#include "util/qtjson.h"
#include "log.h"
#include <set>
#include <unordered_set>
#include "fileinfo.h"

namespace lsh
{
    namespace
    {
        const std::unordered_set<std::string> recognized_callback_names =
        {
            "pre-import",
            "pre-export",
            "post-import",
            "post-export",
        };
    }


    void Blueprint::transferFromAnotherObject(Blueprint&& rhs)
    {
        luschVersion        = std::move(rhs.luschVersion);
        blueprintVersion    = std::move(rhs.blueprintVersion);
        files               = std::move(rhs.files);
        sections            = std::move(rhs.sections);
    }

    void Blueprint::unload()
    {
        luschVersion.clear();
        blueprintVersion.clear();
        files.clear();
        sections.clear();

        // TODO - emit a signal?
    }

    void Blueprint::load(const FileName& filename)
    {
        QString ext = QString::fromStdString( filename.getExt() ).toLower();
        
        if     (ext == "json")                  doLoad( DirTraverser_QDir( QString::fromStdString(filename.getFullPath(true))  ));
        else if(ext == "lshbp" || ext == "zip") throw Error("Internal error - zip files not yet supported");
        else                                    throw Error("Blueprint file '" + filename.getFullPath() + "' has unrecognized extension");
    }

    void Blueprint::doLoad(DirTraverser& dir)
    {
        Blueprint newobj(dir);
        transferFromAnotherObject(std::move(newobj));
        // TODO - emit a signal?
    }

    void Blueprint::loadIndexFile(DirTraverser& dir)
    {
        auto file = dir.openFile("index.json", false);
        if(!file->isOpen())                     throw Error("Blueprint does not contain an 'index.json' file or the file was unable to be opened.");

        auto dat = loadJsonFromFile(*file);

        // Now that we have the parsed json -- load the primary header
        auto i = dat.find("lusch header");
        if(i == dat.end())                      throw Error("Blueprint does not contain a 'lusch header' entry");
        if(!i->second.is<json::object>())       throw Error("Blueprint 'lusch header' entry is not an object");
        else
        {
            auto& hdr = i->second.get<json::object>();

            i = hdr.find("lusch version");
            if(i == hdr.end())                  throw Error("Blueprint is missing 'lusch version' setting");
            if(!i->second.is<std::string>())    throw Error("Blueprint 'lusch version' setting is not a string");
            luschVersion = QString::fromStdString(i->second.get<std::string>());
            
            i = hdr.find("blueprint version");
            if(i == hdr.end())                  throw Error("Blueprint is missing 'blueprint version' setting");
            if(!i->second.is<std::string>())    throw Error("Blueprint 'blueprint version' setting is not a string");
            blueprintVersion = QString::fromStdString(i->second.get<std::string>());
        }

        // Now, load the file list
        i = dat.find("files");
        if(i == dat.end())                      throw Error("Blueprint does not contain a 'files' entry");
        if(!i->second.is<json::array>())        throw Error("Blueprint 'files' entry is not an array");
        else
        {
            std::set<std::string>       names;

            auto& ar = i->second.get<json::array>();
            for(auto& item : ar)
            {
                if(!item.is<json::object>())    { Log::wrn("Blueprint 'files' array contains an entry that is not an object.");     continue;   }

                auto inf = loadFileInfoFromJson(item.get<json::object>());
                if(!inf.id.empty())
                {
                    if(!names.insert(inf.id).second)        Log::wrn("Multiple files with id '" + inf.id + "' found.");
                    else                                    files.emplace_back(std::move(inf));
                }
            }
        }

        // Then the 'sections'
        i = dat.find("sections");
        if(i == dat.end())                      throw Error("Blueprint does not contain a 'sections' entry");
        if(!i->second.is<json::array>())        throw Error("Blueprint 'sections' entry is not an array");
        else
        {
            std::set<std::string>       names;

            auto& ar = i->second.get<json::array>();
            for(auto& item : ar)
            {
                if(!item.is<json::object>())    { Log::wrn("Blueprint 'sections' array contains an entry that is not an object.");     continue;   }
                
                auto inf = loadSectionInfoFromJson(item.get<json::object>());
                if(!inf.id.empty())
                {
                    if(!names.insert(inf.id).second)        Log::wrn("Multiple sections with id '" + inf.id + "' found.");
                    else                                    sections.emplace_back(std::move(inf));
                }
            }
        }

        // And the "callbacks"
        i = dat.find("callbacks");
        if(i != dat.end())
        {
            if(!i->second.is<json::object>())   throw Error("Blueprint 'callbacks' entry is not an object");
            auto& x = i->second.get<json::object>();
            for(auto& item : x)
            {
                //  Is this callback a recognized name?
                if( recognized_callback_names.find( item.first ) == recognized_callback_names.end() )
                {
                    Log::wrn("In blueprint, ignoring unrecognized callback '" + item.first + "'");
                    continue;
                }

                // Is the value a string?
                if( !item.second.is<std::string>() )
                {
                    Log::wrn("In blueprint, ignoring callback '" + item.first + "' because it does not have a string value.");
                    continue;
                }

                callbacks[item.first] = item.second.get<std::string>();
            }
        }
    }

    Blueprint::SectionInfo Blueprint::loadSectionInfoFromJson(const json::object& info)
    {
        SectionInfo out;
        for(auto& i : info)
        {
            if      (i.first == "name"   && i.second.is<std::string>())     out.id =            i.second.get<std::string>();
            else if (i.first == "import" && i.second.is<std::string>())     out.importFunc =    i.second.get<std::string>();
            else if (i.first == "export" && i.second.is<std::string>())     out.exportFunc =    i.second.get<std::string>();
            else
            {
                Log::wrn("Entry in Blueprint 'sections' has a field '" + i.first + "' that is unrecognized, or is of an unexpected type.");
                return SectionInfo();
            }
        }

        if     (out.id.empty())         Log::wrn("Entry in Blueprint 'sections' has no 'name' field, or 'name' field is empty.");
        else if(out.importFunc.empty()) Log::wrn("Entry '" + out.id + "' in Blueprint 'sections' has no 'import' field, or 'import' field is empty.");
        else                            return out;

        return SectionInfo();
    }
    
    FileInfo Blueprint::loadFileInfoFromJson(const json::object& info)
    {
        FileInfo out;
        for(auto& i : info)
        {
            if      (i.first == "id"     && i.second.is<std::string>())     out.id =            i.second.get<std::string>();
            else if (i.first == "name"   && i.second.is<std::string>())     out.displayName =   QString::fromStdString( i.second.get<std::string>() );
            else if (i.first == "optional"  && i.second.is<bool>())         out.optional =      i.second.get<bool>();
            else if (i.first == "directory" && i.second.is<bool>())         out.directory =     i.second.get<bool>();
            else if (i.first == "write"     && i.second.is<bool>())         out.writable =      i.second.get<bool>();
            else
            {
                Log::wrn("Entry in Blueprint 'files' has a field '" + i.first + "' that is unrecognized, or is of an unexpected type.");
                return FileInfo();
            }
        }

        if     (out.id.empty())             Log::wrn("Entry in Blueprint 'files' has no 'id' field, or 'id' field is empty.");
        else if(out.displayName.isEmpty())  Log::wrn("Entry '" + out.id + "' in Blueprint 'files' has no 'name' field, or 'name' field is empty.");
        else                                return out;

        return FileInfo();
    }
    
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////

    Blueprint::Blueprint(DirTraverser& dir)
    {
        //  Step 1, load the index file
        loadIndexFile(dir);

        // TODO finish this
    }

}

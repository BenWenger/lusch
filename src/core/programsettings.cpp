
#include "util/filename.h"
#include "programsettings.h"
#include "versioninfo.h"

namespace lsh
{

    json::object ProgramSettings::toJson() const
    {
        json::object        out;

        {
            auto& blk = json::setNew<json::object>(out["header"]);

            blk["filetype"] = json::value( settingsFileHeaderString );
            blk["version"]  = json::value( settingsFileVersion      );
        }
        {
            auto& blk = json::setNew<json::object>(out["directories"]);

            blk["blueprint"] = json::value( blueprintDir.getFullPath() );
            blk["lastProject"] = json::value( lastProjectDir.getFullPath() );
        }
        {
            auto& blk = json::setNew<json::object>(out["mainwindow"]);

            blk["geometry"] = json::value( mainWindowGeometry.toStdString() );
            blk["state"] = json::value( mainWindowState.toStdString() );
        }

        return out;
    }
    
    void ProgramSettings::fromJson(const json::object& obj)
    {
        ///////////////////////////////////////
        //  Check the header
        bool ok = true;

        ok = ok && json::readField<json::object>(obj, "header", [&](const json::object& blk) {
            ok = ok && json::readField<std::string>(blk, "filetype", [&](const std::string& v){ ok = (v == settingsFileHeaderString);   });
            ok = ok && json::readField<std::string>(blk, "version",  [&](const std::string& v){ ok = (v == settingsFileVersion);        });
        });

        if(!ok)
            return;

        ///////////////////////////////////////
        //  "directories" section
        json::readField<json::object>(obj, "directories", [&](const json::object& blk) {
            json::readField<std::string>(blk, "blueprint", [&](const std::string& v)    {  blueprintDir.set(v,"");              });
            json::readField<std::string>(blk, "lastProject", [&](const std::string& v)  {  lastProjectDir.set(v,"");            });
        });

        ///////////////////////////////////////
        //  "mainwindow" section
        json::readField<json::object>(obj, "mainwindow", [&](const json::object& blk) {
            json::readField<std::string>(blk, "geometry", [&](const std::string& v) {   mainWindowGeometry = QByteArray::fromStdString(v);  });
            json::readField<std::string>(blk, "state",    [&](const std::string& v) {   mainWindowState = QByteArray::fromStdString(v);     });
        });
    }

}

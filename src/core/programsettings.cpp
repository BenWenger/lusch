
#include "util/filename.h"
#include "programsettings.h"
#include "versioninfo.h"

namespace lsh
{

    json::object ProgramSettings::toJson() const
    {
        json::object        out;

        {
            auto& blk = (out["header"] = json::value( json::object() )).get<json::object>();

            blk["filetype"] = json::value( "Lusch settings file" );
            blk["version"]  = json::value( settingsFileVersion   );
        }
        {
            auto& blk = (out["directories"] = json::value( json::object() )).get<json::object>();

            if(absoluteBlueprintDir)        blk["blueprint"] = json::value( blueprintDirectory );
            else                            blk["blueprint"] = json::value( FileName::makeRelativeTo(blueprintDirectory) );
        }
        {
            auto& blk = (out["mainwindow"] = json::value( json::object() )).get<json::object>();

            blk["geometry"] = json::value( 
        }


        return out;
    }

}

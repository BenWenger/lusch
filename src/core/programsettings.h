
#ifndef LUSCH_CORE_PROGRAMSETTINGS_H_INCLUDED
#define LUSCH_CORE_PROGRAMSETTINGS_H_INCLUDED

#include <QByteArray>
#include <string>
#include "util/qtjson.h"

namespace lsh
{
    struct ProgramSettings
    {
        std::string         blueprintDirectory;     // if stored relative, relative to the exe directory
        bool                absoluteBlueprintDir;
        
        QByteArray          mainWindowGeometry;
        QByteArray          mainWindowState;

        json::object        toJson() const;
        void                fromJson(const json::object& obj);
    };
}


#endif

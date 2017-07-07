
#ifndef LUSCH_CORE_PROGRAMSETTINGS_H_INCLUDED
#define LUSCH_CORE_PROGRAMSETTINGS_H_INCLUDED

#include <QByteArray>
#include <string>
#include "util/qtjson.h"
#include "util/filename.h"

namespace lsh
{
    struct ProgramSettings
    {
        FileName            blueprintDir = FileName("blueprints","");   // if relative, relative to the exe directory
        FileName            lastProjectDir;
        
        QByteArray          mainWindowGeometry;
        QByteArray          mainWindowState;

        json::object        toJson() const;
        void                fromJson(const json::object& obj);
    };
}


#endif

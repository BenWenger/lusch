#ifndef LUSCH_UTIL_QTJSON_H_INCLUDED
#define LUSCH_UTIL_QTJSON_H_INCLUDED

#include "error.h"
#include "picojson.h"
#include <string>
#include <QIODevice>
#include <QByteArray>

namespace json = picojson;

/* I'm using PicoJson because it has core support for integers.  Qt's implementation (as well as the
    Json spec) treats all numbers as floating point.

   Plus, should I choose to modify the json a bit to allow hexadecimal numbers in the future, picojson is
    much easier to modify and redistribute.
 */

namespace lsh
{
    inline json::object loadJsonFromFile(QIODevice& file)
    {
        // TODO - maybe iterate over the file in place instead of reading it into a giant buffer?

        auto data = file.readAll();
        if(data.isEmpty())
            throw Error("Unknown error occurred when trying to read json file");

        std::string err;
        json::value output;
        json::parse( output, data.begin(), data.end(), &err );

        if(!err.empty())
            throw Error(err);

        if(!output.is<json::object>())
            throw Error("Json file does not contain a root object.");

        return output.get<json::object>();
    }

    inline std::string QByteArrayToString(const QByteArray& v)
    {
        return std::string( v.data(), v.size() );
    }
}


#endif

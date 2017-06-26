#ifndef LUSCH_CORE_FILEINFO_H_INCLUDED
#define LUSCH_CORE_FILEINFO_H_INCLUDED

#include <string>
#include <QString>

namespace lsh
{
    struct FileInfo
    {
        std::string     id;
        std::string     filePath;
        QString         displayName;
        bool            optional        = false;
        bool            directory       = false;
        bool            writable        = false;
        bool            storeAbsolute   = false;
    };

    struct FileFlags
    {
        bool            append  = false;
        bool            read    = false;
        bool            write   = false;
        bool            trunc   = false;
        bool            binary  = false;

        bool            fromStringMode(std::string mode)
        {
            append = false;
            read = false;
            write = false;
            binary = false;
            trunc = false;

            if(!mode.empty() && mode.back() == 'b')
            {
                binary = true;
                mode.pop_back();
            }

            if     (mode == "r")    {   read = true;                        }
            else if(mode == "w")    {   write = trunc = true;               }
            else if(mode == "a")    {   append = true;                      }
            else if(mode == "r+")   {   read = write = true;                }
            else if(mode == "w+")   {   read = write = trunc = true;        }
            else if(mode == "a+")   {   read = append = true;               }
            else
                return false;

            return true;
        }
    };
}

#endif
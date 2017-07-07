#ifndef LUSCH_UTIL_FILENAME_H_INCLUDED
#define LUSCH_UTIL_FILENAME_H_INCLUDED

#include <string>
#include <vector>

/*
    One area where Qt falls short in comparison to wxWidgets is the lack of a wxFileName equivalent.
    
    A class which abstracts and represents a file name, with directory parsing, relative/absolute conversions,
    etc.

    QDir *almost* does it, but is muddled by the fact that it actually touches the file system, modifies the current directory,
    and does a bunch of other shit that doesn't really belong -- and the directory stuff that's in it isn't as capable
    as wxFileName.

    So this class is my own mini version of wxFileName.
 */

namespace lsh
{

    class FileName
    {
    public:
        FileName() = default;
        FileName(const std::string& fullpath);
        FileName(const std::string& onlypath, const std::string& filename);
        ~FileName() = default;
        FileName(const FileName&) = default;
        FileName(FileName&&) = default;
        FileName& operator = (const FileName&) = default;
        FileName& operator = (FileName&&) = default;

        bool isAbsolute() const                     { return !volume.empty();   }
        bool makeAbsoluteWith(const FileName& base);
        bool makeRelativeTo(const FileName& base);
        bool beginsWithDoubleDot() const;

        std::string     getFullPath(bool use_native_slash = false) const    { return getPathOnly(use_native_slash) + getFileName(); }
        std::string     getFileName() const;
        std::string     getExt() const                                      { return ext;       }
        std::string     getFileTitle() const                                { return title;     }
        std::string     getPathOnly(bool use_native_slash = false) const;

        void            set(const std::string& pathonly, const std::string& filename);
        void            setFullPath(const std::string& fullpath);
        void            setFileName(const std::string& filename);
        void            setExt(const std::string& v)                        { ext = v;          }
        void            setFileTitle(const std::string& v)                  { title = v;        }
        void            setPathOnly(const std::string& pathname);
        void            clear();


    private:
        typedef std::vector<std::string>    path_t;
        std::string     volume;         // if empty, path is relative
        path_t          path;
        std::string     title;
        std::string     ext;            // if empty, filename contains no extension


        static path_t   simplifyDots(const path_t& x, bool keep_ups);

        
        static void         saneSlashes(std::string& str);
        static void         nativeSlashes(std::string& str);
        static std::string  extractVolume(std::string& str);
        static std::string  extractOneDir(std::string& str);
    };

}


#endif

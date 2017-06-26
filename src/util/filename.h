#ifndef LUSCH_UTIL_FILENAME_H_INCLUDED
#define LUSCH_UTIL_FILENAME_H_INCLUDED

#include <string>
#include <QString>

/*

    FileName is a very limited-use class.  It's mostly use by Project to resolve file paths.
    It allows for several relative paths to be chained, falling back to a given absolute path.

 */

namespace lsh
{

    class FileName
    {
    public:
        FileName();
        FileName(const std::string& tier0, const std::string& tier1, const std::string& tier2);
        FileName(const FileName&) = default;
        FileName& operator = (const FileName&) = default;
        ~FileName() = default;

        operator std::string () const           { return getPath();     }
        std::string getPath() const;
        
        void    setTier(int tier, const std::string& tierstr);

        static  std::string     makeAbsolute(const std::string& path, const std::string& root = std::string());
        static  std::string     makeRelativeTo(const std::string& path, const std::string& root = std::string());
        static  bool            isAbsolute(const std::string& path);

    private:
        QString     tiers[3];
        bool        absolute[3];
    };

}


#endif


#include <algorithm>
#include "filename.h"

namespace lsh
{
    FileName::FileName(const std::string& fullpath)
    {
        setFullPath(fullpath);
    }

    FileName::FileName(const std::string& onlypath, const std::string& filename)
    {
        setPathOnly(onlypath);
        setFileName(filename);
    }

    bool FileName::makeAbsoluteWith(const FileName& base)
    {
        if(isAbsolute())            return false;       // already absolute
        if(!base.isAbsolute())      return false;       // our base isn't absolute

        auto work = base.path;
        work.insert( work.end(), path.begin(), path.end() );

        work = simplifyDots(work, false);

        path = std::move(work);
        volume = base.volume;

        return true;
    }

    bool FileName::makeRelativeTo(const FileName& base)
    {
        if(!isAbsolute())           return false;       // already relative
        if(!base.isAbsolute())      return false;       // our base isn't absolute
        if(volume != base.volume)   return false;       // can only be relative if the volumes are the same

        path_t  work;

        // Phase 1:  look for the similar parts of the path
        auto steps = std::max(base.path.size(), path.size());
        std::size_t matching;
        for(matching = 0; (matching < steps) && (base.path[matching] == path[matching]); ++matching) {}

        // Phase 2:  the size of the base directory minus the size of matching directories is the number of ".."s we need
        if(matching < base.path.size())
        {
            work.insert( work.end(), base.path.size() - matching, ".." );
        }

        // Phase 3:  the size of this directory minus the size of the matching directories is the number of dirs we need to preserve
        if(matching < path.size())
        {
            work.insert( work.end(), path.begin() + matching, path.end() );
        }

        // 'work' is now the relative path
        volume.clear();
        path = std::move(work);

        return true;
    }

    std::string FileName::getFileName() const
    {
        if(ext.empty())     return title;
        else                return title + '.' + ext;
    }

    std::string FileName::getPathOnly(bool use_native_slash) const
    {
        std::string out = volume;

        for(auto& i : path)
        {
            out += i;
            out += '/';
        }

        if(use_native_slash)
            nativeSlashes(out);

        return out;
    }
    
    bool FileName::beginsWithDoubleDot() const
    {
        if(path.empty())    return false;
        return path.front() == "..";
    }
    
    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////

    void FileName::setPathOnly(const std::string& pathname)
    {
        std::string v = pathname;
        saneSlashes(v);

        volume = extractVolume(v);

        path.clear();
        while(!v.empty())
        {
            auto dir = extractOneDir(v);
            if(!dir.empty())
                path.emplace_back(std::move(dir));
        }

        path = simplifyDots(path, true);
    }

    void FileName::setFileName(const std::string& filename)
    {
        auto dot = filename.rfind('.');
        // No extension if there's no dot
        //  Or if the dot is the first character (Unix hidden file)
        //  Or if the dot is the last character (preserve the dot as part of the title)
        if(dot == filename.npos || dot == filename.size()-1 || dot == 0)
        {
            title = filename;
            ext.clear();
        }
        else
        {
            title = filename.substr(0,dot);
            ext = filename.substr(dot+1);
        }
    }

    void FileName::setFullPath(const std::string& fullpath)
    {
        auto v = fullpath;
        saneSlashes(v);

        auto slsh = v.rfind('/');
        if(slsh == v.npos)  // no slash, the whole thing is a file name
        {
            volume.clear();
            path.clear();
            setFileName(v);
        }
        else
        {
            setPathOnly(v.substr(0, slsh+1));
            setFileName(v.substr(slsh+1));
        }
    }

    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////

    auto FileName::simplifyDots(const path_t& x, bool keep_ups) -> path_t
    {
        std::size_t ups = 0;
        path_t out;

        out.reserve(x.size());

        for(auto& i : x)
        {
            if(i.empty() || i == ".")
                continue;
            else if(i == "..")
            {
                if(out.empty())     ++ups;
                else                out.pop_back();
            }
            else
                out.push_back(i);
        }

        if(keep_ups && ups > 0)
        {
            out.insert(out.begin(), ups, "..");
        }

        return out;
    }

    void FileName::saneSlashes(std::string& str)
    {
        for(auto& i : str)
        {
            if(i == '\\')       i = '/';
        }
    }

    void FileName::nativeSlashes(std::string& str)
    {
#ifdef WIN32
        for(auto& i : str)
        {
            if(i == '/')        i = '\\';
        }
#endif
    }

    std::string FileName::extractVolume(std::string& str)
    {
        std::string volume;
#ifdef WIN32
        // Windows volumes can be in the typical form:  "C:/" which is a single drive letter, followed
        //      by a colon, followed by the directory separator
        //
        // OR they can be a network path in the form: "//server/"
        if(str.size() >= 3)
        {
            if(str[1] == ':' && str[2] == '/')      // typical drive
            {
                volume = str.substr(0,3);
                str = str.substr(3);
            }
            else if(str[0] == '/' && str[1] == '/') // network path
            {
                auto slsh = str.find('/', 2);
                if(slsh != str.npos)
                {
                    volume = str.substr(0,slsh);
                    str = str.substr(slsh);
                }
            }
        }
#else
        // Unix is easy ... volume is '/' or it isn't there at all
        if(str.size() >= 1)
        {
            if(str[0] == '/')
            {
                volume = str.substr(0,1);
                str = str.substr(1);
            }
        }
#endif

        return volume;
    }

    std::string FileName::extractOneDir(std::string& str)
    {
        std::string out;

        auto slsh = str.find('/');
        if(slsh == str.npos)
        {
            out.clear();
            out.swap(str);
        }
        else
        {
            out = str.substr(0,slsh);
            str = str.substr(slsh+1);
        }

        return out;
    }
}
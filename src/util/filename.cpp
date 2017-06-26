
#include "filename.h"
#include <QDir>
#include "error.h"

//  TODO - maybe check for symbolic links... but whatever

namespace lsh
{
    FileName::FileName()
    {
        for(auto& i : absolute)         i = false;
    }

    FileName::FileName(const std::string& tier0, const std::string& tier1, const std::string& tier2)
    {
        setTier(0, tier0);
        setTier(1, tier1);
        setTier(2, tier2);
    }

    void FileName::setTier(int tier, const std::string& str)
    {
        if(tier < 0 || tier > 2)
            throw Error("Internal Error:  Bad tier number passed to FileName::setTier");

        tiers[tier] = QDir::fromNativeSeparators( QString::fromStdString(str) );
        absolute[tier] = QDir::isAbsolutePath(tiers[tier]);
    }

    std::string FileName::getPath() const
    {
        int i;
        for(i = 2; (i >= 0) && !absolute[i]; --i) {}        // find the last abolute path (that is our base)

        if(i < 0)       throw Error("Internal Error:  No absolute paths given to FileName.");

        QString out = tiers[i];
        for(++i; i < 3; ++i)
        {
            if(tiers[i].isEmpty())      continue;
            if(!out.endsWith('/'))      out += '/';
            out += tiers[i];
        }

        out = QDir::cleanPath( QDir::fromNativeSeparators(out) );

        if(out.endsWith('/'))
            out.chop(1);

        if(!QDir::isAbsolutePath(out))
            throw Error("Internal Error:  In filename, final result path is not absolute path.");

        return out.toStdString();
    }

}
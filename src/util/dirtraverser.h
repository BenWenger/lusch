#ifndef LUSCH_UTIL_DIRTRAVERSER_H_INCLUDED
#define LUSCH_UTIL_DIRTRAVERSER_H_INCLUDED

#include <memory>
#include <QIODevice>
#include <QString>

/*
 *  For whatever reason, it seems that no filesystem or archive libraries on the planet abstract
 *    the file system so you can have a common base class for filesystem/archives -- even though
 *    that's like the most obvious abstraction ever.  So I have to do this roundabout crap
 *    to extract them.
 *
 *  This class is very simplistic and not very generalized.  It assumes a very specific use case:
 *    - Files are read-only
 *    - Only one file from the dir will ever be opened at a time (QuaZip limitation)
 *    - You don't care about the directory structure, you just want a linear walk of all files
 *         in the directory.
 *    - You want to explore all subdirectories recursively (though still want them reported linearly)
 *
 *  NOTE:  On construction, the iterator points at nothing.  You must call next() at least once to
 *    examine the first file.
 */

namespace lsh
{
    class DirTraverser
    {
    public:
        virtual                             ~DirTraverser() {}
        virtual bool                        next() = 0;
        virtual QString                     getName() = 0;
        virtual QString                     getExt() = 0;
        virtual std::unique_ptr<QIODevice>  openFile(const QString& filename, bool binary) = 0;
    };
}

#endif

#include <QFile>
#include "dirtraverser_qdir.h"


namespace lsh
{
    DirTraverser_QDir::DirTraverser_QDir(const QString& rootdir)
        : dir(rootdir)
        , iter(rootdir, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden, QDirIterator::Subdirectories )
    {}

    bool DirTraverser_QDir::next()
    {
        if(!iter.hasNext())
            return false;

        iter.next();
        return true;
    }

    QString DirTraverser_QDir::getName()
    {
        return dir.relativeFilePath( iter.filePath() );
    }
    
    QString DirTraverser_QDir::getExt()
    {
        QFileInfo inf(iter.filePath());
        return inf.suffix().toLower();
    }
    
    std::unique_ptr<QIODevice> DirTraverser_QDir::openFile(const QString& filename, bool binary)
    {
        QString path = dir.absoluteFilePath(filename);
        QIODevice::OpenMode openmode = QIODevice::ReadOnly;
        if(!binary)
            openmode |= QIODevice::Text;

        auto file = std::make_unique<QFile>(path);
        file->open( openmode );

        return std::move(file);
    }
}

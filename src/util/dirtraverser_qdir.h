#ifndef LUSCH_UTIL_DIRTRAVERSER_QDIR_H_INCLUDED
#define LUSCH_UTIL_DIRTRAVERSER_QDIR_H_INCLUDED

#include <QString>
#include <QDir>
#include <QDirIterator>
#include "dirtraverser.h"


namespace lsh
{
    class DirTraverser_QDir : public DirTraverser
    {
    public:
        DirTraverser_QDir(const QString& rootdir);

        virtual bool                        next() override;
        virtual QString                     getName() override;
        virtual QString                     getExt() override;
        virtual std::unique_ptr<QIODevice>  openFile(const QString& filename, bool binary) override;

    private:
        QDir                    dir;
        QDirIterator            iter;
    };
}

#endif
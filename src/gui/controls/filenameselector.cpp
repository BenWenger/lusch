
#include "filenameselector.h"
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QFileDialog>
#include "log.h"

namespace lsh
{
    
    FileNameSelector::FileNameSelector(QWidget* parent, Flags flgs, const FileName& initial, const FileName& rootpath, const QString& browsemsg, const QString& filterstr)
        : QHBoxLayout(parent)
    {
        filters = filterstr;
        browseText = browsemsg;
        flags = flgs;
        root = rootpath;
        canBeRelative = rootpath.isAbsolute();
        parentWidget = parent;


        bool defaultToAbsolute = !!(flags & Flags::DefaultToAbsolute);

        FileName v = initial;
        if(canBeRelative)
        {
            if(v.isAbsolute() && !defaultToAbsolute)
                v.makeRelativeTo(rootpath);
            else if(!v.isAbsolute() && defaultToAbsolute)
                v.makeAbsoluteWith(rootpath);
        }

        addWidget( textBox = new QLineEdit( QString::fromStdString(v.getFullPath()), parent ) );
        addWidget( browseBtn = new QPushButton("...", parent) );
        connect( browseBtn, &QPushButton::clicked, this, &FileNameSelector::onBrowse );

        if(canBeRelative)
        {
            addWidget( absoluteChk = new QCheckBox("Abs", parent) );
            absoluteChk->setCheckState( defaultToAbsolute ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );

            connect( absoluteChk, &QCheckBox::stateChanged, this, &FileNameSelector::onCheck );
        }

        textBox->setMinimumWidth( 250 );
        browseBtn->setMinimumWidth( 25 );
        browseBtn->setMaximumWidth( 25 );
        browseBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    }


    ////////////////////////////////////
    void FileNameSelector::onCheck(int state)
    {
        if(!canBeRelative)      return;

        auto v = getValue();
        if(state == Qt::CheckState::Checked)
            v.makeAbsoluteWith(root);
        else
            v.makeRelativeTo(root);

        setValue(v);
    }

    void FileNameSelector::onBrowse(bool)
    {
        auto dir = getValue();

        dir.makeAbsoluteWith(root);
        QString fromdir = QString::fromStdString( dir.getFullPath(true) );

        QString result;
        if(flags & Flags::Directory)            result = QFileDialog::getExistingDirectory(parentWidget, browseText, fromdir);
        else if(flags & Flags::Write)           result = QFileDialog::getSaveFileName(parentWidget, browseText, fromdir, filters);
        else                                    result = QFileDialog::getOpenFileName(parentWidget, browseText, fromdir, filters);

        if(result.isEmpty())        return;

        dir.setFullPath( result.toStdString() );
        if(!getAbsolute())
        {
            dir.makeRelativeTo( root );
        }

        setValue(dir);
    }

    FileName FileNameSelector::getValue() const
    {
        FileName out;
        out.setFullPath( textBox->text().toStdString() );

        return out;
    }

    void FileNameSelector::setValue(const FileName& val)
    {
        textBox->setText( QString::fromStdString( val.getFullPath() ) );
    }

    bool FileNameSelector::getAbsolute() const
    {
        if(!canBeRelative || !absoluteChk)      return true;

        return absoluteChk->checkState() == Qt::CheckState::Checked;
    }

    void FileNameSelector::setAbsolute(bool abs)
    {
        if(!canBeRelative || !absoluteChk)      return;

        absoluteChk->setCheckState( abs ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
    }

}
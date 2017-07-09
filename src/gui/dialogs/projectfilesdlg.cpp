
#include "projectfilesdlg.h"
#include "core/project.h"
#include "gui/controls/filenameselector.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

namespace lsh
{

    bool ProjectFilesDlg::go(QWidget* parent, Project& project)
    {
        // weird edge case that might happen if the blueprint is weird/malformed
        if(project.getFileInfoArray().empty())
            return true;

        ProjectFilesDlg dlg(parent, project);
        return dlg.exec() == QDialog::Accepted;
    }

    ProjectFilesDlg::ProjectFilesDlg(QWidget* parent, Project& pj)
        : QDialog(parent)
        , project(&pj)
    {
        QVBoxLayout*        mainLayout;
        setLayout( mainLayout = new QVBoxLayout(this) );

        bool show_mandatory = false;    // *
        bool show_write     = false;    // +
        bool show_dir       = false;    // ~
        bool show_writedir  = false;    // (!)
        
        //  Build the form layout
        QFormLayout*        formLayout;// = new QFormLayout(this);
        mainLayout->addLayout( formLayout = new QFormLayout(this) );
        QString display;
        QString caption;
        for(auto& i : pj.getFileInfoArray())
        {
            // get display string
            if(!i.optional)     { display = "* ";       show_mandatory = true;      }
            else                display = "";

            display += i.displayName;

            if(i.directory)
            {
                if(i.writable)  { display += " (!)";    show_writedir = true;       }
                else            { display += " ~";      show_dir = true;            }
            }
            else if(i.writable) { display += " +";      show_write = true;          }

            // get flags
            int flags = 0;
            if(i.directory)             flags |= FileNameSelector::Flags::Directory;
            if(i.writable)              flags |= FileNameSelector::Flags::Write;
            if(i.fileName.isAbsolute()) flags |= FileNameSelector::Flags::DefaultToAbsolute;

            // caption
            if(i.directory)
            {
                if(i.writable)          caption = "Please select a (writable) directory";
                else                    caption = "Please select a (read only) directory";
            }
            else
            {
                if(i.writable)          caption = "Please select a (writable) file";
                else                    caption = "Please select a (read only) file";
            }

            // Build the FileNameSelector
            selectors.push_back( new FileNameSelector(this, FileNameSelector::Flags(flags), i.fileName, project->getProjectFileName(), caption) );
            formLayout->addRow(display, selectors.back());
        }

        // Show the footnote strings
        if(show_mandatory)      mainLayout->addWidget( new QLabel("* : This field is required", this) );
        if(show_write)          mainLayout->addWidget( new QLabel("+ : The blueprint will be able to delete/overwrite this file", this) );
        if(show_dir)            mainLayout->addWidget( new QLabel("~ : The blueprint will be able to read all files in this directory", this) );
        if(show_writedir)       mainLayout->addWidget( new QLabel("(!) : The blueprint will be able to modify/delete any files in this directory", this) );

        // Ok / cancel buttons
        QDialogButtonBox* box;
        mainLayout->addWidget( box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel) );
        connect( box, &QDialogButtonBox::accepted, this, &ProjectFilesDlg::accept );
        connect( box, &QDialogButtonBox::rejected, this, &ProjectFilesDlg::reject );
    }


    void ProjectFilesDlg::accept()
    {
        auto& dat = project->getFileInfoArray();

        auto count = std::min(dat.size(), selectors.size());            // these should match, but just in case...

        // Make sure all required elements are supplied
        for(std::size_t i = 0; i < count; ++i)
        {
            if(!dat[i].optional && selectors[i]->getValue().isEmpty())
            {
                QMessageBox::information(this, "Page incomplete", "Please fill all required fields.");
                return;
            }
        }

        // Otherwise, capture all the data
        for(std::size_t i = 0; i < count; ++i)
        {
            dat[i].fileName = selectors[i]->getValue();
        }

        QDialog::accept();
    }
}
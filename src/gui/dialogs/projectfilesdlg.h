
#ifndef LUSCH_GUI_DIALOGS_PROJECTFILESDLG_H_INCLUDED
#define LUSCH_GUI_DIALOGS_PROJECTFILESDLG_H_INCLUDED

#include <QDialog>

namespace lsh
{
    class Project;
    class FileNameSelector;

    class ProjectFilesDlg : public QDialog
    {
    public:
        static bool     go(QWidget* parent, Project& project);

    private:
        ProjectFilesDlg(QWidget* parent, Project& project);

        Project*                        project;
        std::vector<FileNameSelector*>  selectors;

    private:

        virtual void accept() override;
    };
}

#endif
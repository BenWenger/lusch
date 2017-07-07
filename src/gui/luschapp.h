#ifndef LUSCH_GUI_LUSCHAPP_H_INCLUDED
#define LUSCH_GUI_LUSCHAPP_H_INCLUDED

#include <QMainWindow>
#include "core/project.h"
#include "editortreemodel.h"
#include "core/programsettings.h"
#include "util/filename.h"

namespace lsh
{
    class LuschApp : public QMainWindow
    {
        Q_OBJECT

    public:
        LuschApp(QWidget *parent = Q_NULLPTR);

        bool promptIfDirty(const char* prompt);

    private:
        QMainWindow*        editorHost;

        QDockWidget*        dock_logger;
        QDockWidget*        dock_editortree;

        EditorTreeModel     editorTree;
        
        void        onLaunch(const QModelIndex& index);
        
        void        onNewProject();
        void        onOpenProject();
        void        onSaveProject();
        void        onExit()                { close();      }
        
        ////////////////////////////////////////////////
        FileName            exeFileName;
        FileName            programSettingsFileName;
        ProgramSettings     settings;
        Project             project;


        ////////////////////////////////////////////////
        QAction*    actNewProject;
        QAction*    actOpenProject;
        QAction*    actSaveProject;
        QAction*    actExit;

        void        buildActions();
        void        buildMenu();



        virtual void    closeEvent(QCloseEvent* evt) override;

        void        loadCommonFileNames();
        void        loadProgramSettings();
        void        saveProgramSettings();

        
        bool        fileDialog_Blueprint(FileName& bpAbsolute, FileName& bpRelative);
        bool        fileDialog_Project(FileName& path);
        bool        dialog_ProjectFiles(Project& prj) { return false; }      // TODO fill this in
    };

}


#endif
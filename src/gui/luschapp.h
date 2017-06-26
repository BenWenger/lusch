#ifndef LUSCH_GUI_LUSCHAPP_H_INCLUDED
#define LUSCH_GUI_LUSCHAPP_H_INCLUDED

#include <QMainWindow>
#include "core/project.h"
#include "editortreemodel.h"

namespace lsh
{
    class LuschApp : public QMainWindow
    {
        Q_OBJECT

    public:
        LuschApp(QWidget *parent = Q_NULLPTR);

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
        Project     project;


        ////////////////////////////////////////////////
        QAction*    actNewProject;
        QAction*    actOpenProject;
        QAction*    actSaveProject;
        QAction*    actExit;

        void        buildActions();
        void        buildMenu();



        virtual void    closeEvent(QCloseEvent* evt) override;
    };

}


#endif
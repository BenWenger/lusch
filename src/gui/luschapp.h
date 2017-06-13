#ifndef LUSCH_GUI_LUSCHAPP_H_INCLUDED
#define LUSCH_GUI_LUSCHAPP_H_INCLUDED

#include <QMainWindow>
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
        
        void        onDbg();
        void        onInf();
        void        onWrn();
        void        onErr();
        void        onLaunch(const QModelIndex& index);
    };

}


#endif
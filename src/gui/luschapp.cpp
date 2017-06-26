#include "luschapp.h"
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QMenuBar>
#include <QTreeView>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>


#include <QFileInfo>
#include "loggerwindow.h"
#include "core/blueprint.h"

#include "util/filename.h"
#include "util/safecall.h"


namespace lsh
{
    void LuschApp::onLaunch(const QModelIndex& index)
    {/*
        QString launchString = index.data(EditorTreeModel::LaunchRole).toString();

        if(!launchString.isEmpty())
            Log::inf("Launched:  " + launchString);

        QFileInfo inf("A/File/That/Doesnt/Exist.bat");
        Log::inf( inf.filePath() );
        */
        /*
        try
        {
            Log::inf( FileName( "C:\\what\\.\\\\is\\deleted\\..\\", "something/////////////\\\\", "waaaaat") );
        }
        catch(std::exception& e)
        {
            Log::err(e.what());
        }*/

        /*
        auto filename = QFileDialog::getOpenFileName(this, "Select blueprint file", QString(), "Blueprint Files (*.lshbp index.json);;All Files (*)", nullptr, QFileDialog::DontConfirmOverwrite);
        try
        {
            Blueprint bp;
            bp.load(filename);
        }
        catch(...)
        {
        }
        */
    }

#define MENU(str, code, func)     \
{   auto act = new QAction(str, this);      \
    act->setShortcut( Qt::Key_##code | Qt::CTRL ); \
    connect( act, &QAction::triggered, this, &LuschApp::func ); \
    fmenu->addAction(act);          }


    LuschApp::LuschApp(QWidget *parent)
        : QMainWindow(parent)
    {
        buildActions();
        buildMenu();

        //  The editor host is the central widget for this window
        editorHost = new QMainWindow(this);
        editorHost->setWindowFlags(Qt::Widget);
        setCentralWidget(editorHost);

        //  The logger is placed at the bottom by default
        dock_logger = new QDockWidget("Output", this);
        auto logger = new LoggerWindow(dock_logger);
        dock_logger->setWidget(logger);
        addDockWidget(Qt::BottomDockWidgetArea, dock_logger);

        //  The editor tree is placed on the left by default
        dock_editortree = new QDockWidget("Editors", this);
        auto treeview = new QTreeView(this);
        //treeview->setEnabled(false);
        treeview->setModel(&editorTree);
        treeview->setHeaderHidden(true);
        connect( treeview, &QAbstractItemView::activated, this, &LuschApp::onLaunch );
        dock_editortree->setWidget(treeview);
        addDockWidget(Qt::LeftDockWidgetArea, dock_editortree);


        ///////////////////////////////
        //  Temporary menu
        /*
        auto menubar = menuBar();
        auto fmenu = menubar->addMenu("&Temp");
        
        MENU( "Debug\tCtrl+1", 1, onDbg );
        MENU( "Info\tCtrl+2", 2, onInf );
        MENU( "Warning\tCtrl+3", 3, onWrn );
        MENU( "Error\tCtrl+4", 4, onErr );
        */
        /*
        auto act = new QAction("&Temp\tCtrl+T", this);
        act->setShortcut( Qt::Key_T | Qt::CTRL );
        connect(act, &QAction::triggered, this, &LuschApp::onTemp);

        fmenu->addAction(act);
        */
    /*
        subwindow->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    */
        /*
        QToolBar* bar = addToolBar("TB 1");
        bar->addAction("B 1");

        bar = addToolBar("TB 2");
        bar->addAction("B 2");
    
        bar = addToolBar("TB 3");
        bar->addAction("B 3");*/
        /*
        QTextEdit* txt = new QTextEdit(this);
        setCentralWidget(txt);
        */
    /*
        QDockWidget* dk = new QDockWidget("Dock 1", this);
        dk->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::LeftDockWidgetArea, dk);
    
        QDockWidget* dk2 = new QDockWidget("Dock 2", this);
        dk2->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dk2);
    
        QDockWidget* dk3 = new QDockWidget("Dock 3", this);
        dk3->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::RightDockWidgetArea, dk3);

    
        QDockWidget* idk = new QDockWidget("I Dock 1", this);
        idk->setAllowedAreas(Qt::AllDockWidgetAreas);
        subwindow->addDockWidget(Qt::LeftDockWidgetArea, idk);
    
        QDockWidget* idk2 = new QDockWidget("I Dock 2", this);
        idk2->setAllowedAreas(Qt::AllDockWidgetAreas);
        subwindow->addDockWidget(Qt::TopDockWidgetArea, idk2);
    
        QDockWidget* idk3 = new QDockWidget("I Dock 3", this);
        idk3->setAllowedAreas(Qt::AllDockWidgetAreas);
        subwindow->addDockWidget(Qt::RightDockWidgetArea, idk3);
        */
    }

    void LuschApp::buildActions()
    {
        auto makeAction = [&] (QAction*& act, const std::string& name, QKeySequence shortcut, void (LuschApp::*clbk)())
        {
            act = new QAction( tr(name.c_str()), this );
            act->setShortcut( shortcut );
            connect( act, &QAction::triggered, this, clbk );
        };
        
        makeAction( actNewProject,  "&New Project",     QKeySequence::New,          &LuschApp::onNewProject     );
        makeAction( actOpenProject, "&Open Project",    QKeySequence::Open,         &LuschApp::onOpenProject    );
        makeAction( actSaveProject, "&Save Project",    QKeySequence::Save,         &LuschApp::onSaveProject    );
        makeAction( actExit,        "E&xit",            QKeySequence::Quit,         &LuschApp::onExit           );
    }

    void LuschApp::buildMenu()
    {
        auto main = menuBar();
        
        auto menu_file = main->addMenu("&File");
        menu_file->addAction( actNewProject );
        menu_file->addAction( actOpenProject );
        menu_file->addAction( actSaveProject );
        menu_file->addSeparator();
        menu_file->addAction( actExit );

        actSaveProject->setEnabled(false);
    }

    void LuschApp::onNewProject()       { /* TODO   */  }
    void LuschApp::onOpenProject()      { /* TODO   */  }
    void LuschApp::onSaveProject()      { /* TODO   */  }

    void LuschApp::closeEvent(QCloseEvent* evt)
    {
        BEGIN_SAFE

        if( !project.promptIfDirty( this, "Save changes to project before exiting?" ) )
            evt->ignore();

        END_SAFE
    }

}

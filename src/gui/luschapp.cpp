#include "luschapp.h"
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QMenuBar>
#include <QTreeView>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QMessageBox>


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

    LuschApp::LuschApp(QWidget *parent)
        : QMainWindow(parent)
    {
        loadCommonFileNames();

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

        
        setGeometry(100, 100, 1200, 600);     // TODO this is temporary
        loadProgramSettings();
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

    void LuschApp::onNewProject()
    {
        BEGIN_SAFE

        // A new project will close our current project.  See if the user wants to save first
        if( !promptIfDirty("Save changes before creating a new project?") )
            return;
        
        //  Select a blueprint, and load it
        FileName    bpAbs, bpRel;                               // absolute and relative paths
        if(!fileDialog_Blueprint(bpAbs, bpRel))                 return;
        Blueprint bp;
        bp.load(bpAbs);


        //  Select a project, bind it to the blueprint
        FileName    projectPath;
        if(!fileDialog_Project(projectPath))                    return;

        Project pj;
        pj.newProject( projectPath, bpRel, std::move(bp) );

        //  Have the user select the files for the project
        if(!dialog_ProjectFiles(pj))                            return;

        //  At this point, project and blueprint are complete enough to be usable.
        project = std::move(pj);

        //  Lastly, do a proper import -- This is OK to fail
            BEGIN_SAFE
            project.doImport();
            END_SAFE
        
        // Now that everything is done, save the project file
        project.doSave();

        END_SAFE
    }

    void LuschApp::onOpenProject()      { /* TODO   */  }
    void LuschApp::onSaveProject()      { /* TODO   */  }

    void LuschApp::closeEvent(QCloseEvent* evt)
    {
        BEGIN_SAFE

        if( !promptIfDirty( "Save changes to project before exiting?" ) )
            evt->ignore();
        else
            saveProgramSettings();

        END_SAFE
    }

    bool LuschApp::promptIfDirty(const char* prompt)
    {
        if(!project.isDirty())
            return true;

        auto answer = QMessageBox::warning(this, "Are you sure?", prompt,
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                           QMessageBox::Yes );

        switch(answer)
        {
        case QMessageBox::No:       return true;
        case QMessageBox::Yes:      /* TODO save */ break;
        default:                    return false;
        }

        return !project.isDirty();
    }

    
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////

    void LuschApp::loadCommonFileNames()
    {
        exeFileName = QCoreApplication::applicationFilePath().toStdString();
        programSettingsFileName = exeFileName;
        programSettingsFileName.setFileName("lusch_settings.cfg");
        int something = 10;
    }

    void LuschApp::loadProgramSettings()
    {
        try
        {
            QFile file;
            file.setFileName( QString::fromStdString( programSettingsFileName.getFullPath(true) ) );
            if( file.open(QIODevice::ReadOnly | QIODevice::Text) )
                settings.fromJson( loadJsonFromFile(file) );
            ////////////////////////////////////////
            
            restoreGeometry(settings.mainWindowGeometry);
            restoreState   (settings.mainWindowState);
        }
        catch(...){}
    }
    
    void LuschApp::saveProgramSettings()
    {
        try
        {
            settings.mainWindowGeometry = saveGeometry();
            settings.mainWindowState    = saveState();

            ////////////////////////////////////////
            auto obj = settings.toJson();

            QFile file;
            file.setFileName( QString::fromStdString( programSettingsFileName.getFullPath(true) ) );
            if( file.open(QIODevice::WriteOnly | QIODevice::Truncate) )
                saveJsonToFile(obj, file);
        }
        catch(...){}
    }

    /////////////////////////////////////////////////////////

    bool LuschApp::fileDialog_Blueprint(FileName& bpAbsolute, FileName& bpRelative)
    {
        FileName root = settings.blueprintDir;
        root.makeAbsoluteWith( exeFileName );

        auto x = QFileDialog::getOpenFileName(this, tr("Choose a blueprint for this project."),
                                              QString::fromStdString(root.getFullPath(true)),
                                              tr("Lusch blueprint files (index.json)")
                                             );
        if(x.isEmpty())
            return false;
        bpAbsolute = x.toStdString();
        bpRelative = bpAbsolute;
        bpRelative.makeRelativeTo( bpAbsolute );
        if(bpRelative.beginsWithDoubleDot() || bpRelative.isAbsolute())            // is this not in the "blueprints" directory?
        {
            auto answer = QMessageBox::warning(this, "Blueprint not in expected directory",
                                           "This blueprint is not in Lusch's blueprint directory.  This will work, but is not recommended.  Are you sure you want to continue with this blueprint?",
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                           QMessageBox::Yes );
            if(answer != QMessageBox::Yes)      return false;
        }

        return true;
    }
    
    bool LuschApp::fileDialog_Project(FileName& path)
    {
        //////////////////////////////////////////////
        // Next, select a project path.  Default this directory to the last directory a project file was accessed from
        auto x = QFileDialog::getSaveFileName(this, tr("Choose a location to save your project."),
                                              QString::fromStdString(settings.lastProjectDir.getFullPath(true)),
                                              tr("Lusch project files (*.lshpj)")
                                             );

        if(x.isEmpty())
            return false;
        path = x.toStdString();
        settings.lastProjectDir.setPathOnly(path.getPathOnly());

        return true;
    }
}

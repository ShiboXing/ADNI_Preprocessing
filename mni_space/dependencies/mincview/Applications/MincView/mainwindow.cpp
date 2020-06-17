#include "mainwindow.h"
#include "application.h"
#include "scenemanager.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qkeycode.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qaccel.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qprogressdialog.h>
#include <qkeysequence.h>
#include <qsplitter.h>
#include <qsettings.h>

#include "aboutbicigns.h"
#include "qtutilities.h"
#include "imageobject.h"
#include "view.h"
#include "ignsconfig.h"
#include "viewmenufunctions.h"
#include "importmenufunctions.h"
#include "settingsmenufunctions.h"
#include "toolsmenufunctions.h"

const QString MainWindow::m_appName( tr("MincView") );


MainWindow::MainWindow( QWidget * parent, const char * name, WFlags f ) : QMainWindow( parent, name, f )
{
    setCaption( m_appName );
    
    SceneManager * man = Application::GetSceneManager();
    
    menuBar()->setSeparator( QMenuBar::Never );
    statusBar()->setHidden( TRUE );

    // Creates a file menu
    QPopupMenu * m_fileMenu = new QPopupMenu( this );
    menuBar()->insertItem( tr("&File"), m_fileMenu );
    m_fileMenu->insertItem( tr("&Open File"), this, SLOT( fileOpenFile() ) );
    m_fileMenu->insertSeparator();
    m_fileMenu->insertItem( tr("&Exit"), this, SLOT( ExitApp() ), CTRL+Key_Q );

    m_settingsMenuFunctions = new SettingsMenuFunctions;
    m_settingsMenuFunctions->SetManager(man);
    m_settingsMenu = new QPopupMenu( this );
    menuBar()->insertItem( tr("&Settings"), m_settingsMenu );
    m_settingsMenu->insertItem( tr("&Background Color"), this, SLOT( SetBackgroundColor() ) );
    
    // Creates a View menu
    m_viewMenuFunctions = new ViewMenuFunctions;
    m_viewMenuFunctions->SetManager(man);
    
    viewOutlineAction = new QAction( tr("&Outline"), QKeySequence("ALT+o"), this, "viewOutlineAction" );
    viewX_PlaneAction = new QAction( tr("&X Plane"), QKeySequence("Alt+x"), this, "viewX_PlaneAction" );
    viewY_PlaneAction = new QAction( tr("&Y Plane"), QKeySequence("Alt+y"), this, "viewY_PlaneAction" );
    viewZ_PlaneAction = new QAction( tr("&Z Plane"), QKeySequence("Alt+z"), this, "viewZ_PlaneAction" );
    view3D_PlaneAction = new QAction( tr("&Planes in 3D View"), QKeySequence("Alt+p"), this, "view3D_PlaneAction" );
    
    viewOutlineAction->setToggleAction( TRUE );
    viewX_PlaneAction->setToggleAction( TRUE );
    viewY_PlaneAction->setToggleAction( TRUE );
    viewZ_PlaneAction->setToggleAction( TRUE );
    view3D_PlaneAction->setToggleAction( TRUE );
    viewOutlineAction->setOn(TRUE);
    viewX_PlaneAction->setOn(TRUE);
    viewY_PlaneAction->setOn(TRUE);
    viewZ_PlaneAction->setOn(TRUE);
    view3D_PlaneAction->setOn(TRUE);
    
    view3D_Front = new QAction( tr("&Front"), QKeySequence("Shift+Alt+f"), this, "view3D_Front" );
    view3D_Left = new QAction( tr("&Left"), QKeySequence("Shift+Alt+l"), this, "view3D_Left" );
    view3D_Right = new QAction( tr("&Right"), QKeySequence("Shift+Alt+r"), this, "view3D_Right" );
    view3D_Back = new QAction( tr("&Back"), QKeySequence("Shift+Alt+b"), this, "view3D_Back" );
    view3D_Top = new QAction( tr("&Top"), QKeySequence("Shift+Alt+t"), this, "view3D_Top" );
    view3D_Bottom = new QAction( tr("Botto&m"), QKeySequence("Shift+Alt+m"), this, "view3D_Bottom" );
    viewAllObjectsOpacity = new QAction( tr("S&et Opacity for All Objects"), QKeySequence("Shift+Alt+o"), this, "viewAllObjectsOpacity" );
    
    m_viewMenu = new QPopupMenu( this );
    viewOutlineAction->addTo( m_viewMenu );
    viewX_PlaneAction->addTo( m_viewMenu );
    viewY_PlaneAction->addTo( m_viewMenu );
    viewZ_PlaneAction->addTo( m_viewMenu );
    view3D_PlaneAction->addTo( m_viewMenu );
    m_viewMenu->insertSeparator();
    view3D_Front->addTo( m_viewMenu );
    view3D_Left->addTo( m_viewMenu );
    view3D_Right->addTo( m_viewMenu );
    view3D_Back->addTo( m_viewMenu );
    view3D_Top->addTo( m_viewMenu );
    view3D_Bottom->addTo( m_viewMenu );
    m_viewMenu->insertSeparator();
    viewAllObjectsOpacity->addTo( m_viewMenu );
    
    m_viewModeSubMenu = new QPopupMenu(m_viewMenu);
    m_viewModeSubMenu->setCheckable(TRUE);
    m_viewModeSubMenu->insertItem(tr("&Only Selected Object in 2D Views"), this, SLOT (SetViewModeToSelectedObject()), 0, VIEW_SELECTED, SET_SELECTED);
    m_viewModeSubMenu->insertItem(tr("&Synchronize Cutting Planes"), this, SLOT (SetViewModeToSynchronizePlanes()), 0,  VIEW_SYNCHRONIZED, SET_SYNC);
    m_viewModeSubMenu->insertItem(tr("View &All Objects in 2D Views"), this, SLOT (SetViewModeToAllIn2D()), 0, VIEW_ALL_IN_2D, SET_ALL);
    m_viewModeSubMenu->setItemChecked(VIEW_SELECTED, TRUE);
    m_viewModeSubMenu->setItemChecked(VIEW_SYNCHRONIZED, FALSE);
    m_viewModeSubMenu->setItemChecked(VIEW_ALL_IN_2D, FALSE);
    connect( m_viewModeSubMenu, SIGNAL( aboutToShow() ), this, SLOT( SetSelectedViewMode() ) );
    m_viewMenu->insertSeparator();
    m_viewMenu->insertItem(tr("&View Mode"), m_viewModeSubMenu);
    
    menuBar()->insertItem( tr("&View"), m_viewMenu );
    
    connect( m_viewMenu, SIGNAL( aboutToShow() ), this, SLOT( EnableDisableViewMenu() ) );
    connect( viewOutlineAction, SIGNAL( toggled(bool) ), this, SLOT( ViewOutlineToggled(bool) ) );
    connect( viewX_PlaneAction, SIGNAL( toggled(bool) ), this, SLOT( ViewXPlaneToggled(bool) ) );
    connect( viewY_PlaneAction, SIGNAL( toggled(bool) ), this, SLOT( ViewYPlaneToggled(bool) ) );
    connect( viewZ_PlaneAction, SIGNAL( toggled(bool) ), this, SLOT( ViewZPlaneToggled(bool) ) );
    connect( view3D_PlaneAction, SIGNAL( toggled(bool) ), this, SLOT( View3DPlaneToggled(bool) ) );
    connect( view3D_Front, SIGNAL( activated() ), this, SLOT( View3DFront() ) );
    connect( view3D_Left, SIGNAL( activated() ), this, SLOT( View3DLeft() ) );
    connect( view3D_Right, SIGNAL( activated() ), this, SLOT( View3DRight() ) );
    connect( view3D_Back, SIGNAL( activated() ), this, SLOT( View3DBack() ) );
    connect( view3D_Top, SIGNAL( activated() ), this, SLOT( View3DTop() ) );
    connect( view3D_Bottom, SIGNAL( activated() ), this, SLOT( View3DBottom() ) );
    connect( viewAllObjectsOpacity, SIGNAL( activated() ), this, SLOT( SetAllObjectsOpacity() ) );
    
    m_importMenuFunctions = new ImportMenuFunctions;
    m_importMenuFunctions->SetManager(man);
    m_importMenu = new QPopupMenu( this );
    menuBar()->insertItem( tr("&Import"), m_importMenu );
    m_importMenu->insertItem( tr("Import &Obj"), this, SLOT( ImportObj() ), 0, IMP_OBJ );
    m_importMenu->insertItem( tr("Import &Minc"), this, SLOT( ImportMinc() ), 0, IMP_MINC );
    m_importMenu->insertSeparator();
    m_importMenu->insertItem( tr("&Remove Imported Obj/Minc"), this, SLOT( RemoveImportedObjOrMinc() ), 0, REM_IMP );
    connect( m_importMenu, SIGNAL( aboutToShow() ), this, SLOT( EnableDisableImportMenu() ) );
    
    m_toolsMenuFunctions = new ToolsMenuFunctions;
    m_toolsMenuFunctions->SetManager(man);
    m_toolsMenuFunctions->SetLastVisitedDirectory(Application::GetSettings()->LastVisitedDirectory);
    m_toolsMenu = new QPopupMenu( this );
    menuBar()->insertItem( tr("&Tools"), m_toolsMenu );
    m_toolsMenu->insertItem( tr("Enable &Picker"), this, SLOT( EnablePicker() ) );
    m_toolsMenu->insertItem( tr("V&olume Reconstruction"), this, SLOT( OpenVolumeReconstructionDialog() ) );
    m_toolsMenu->insertItem( tr("&Multiple File Viewer"), this, SLOT( OpenShowMultipleFilesDialog() ) );
    
    // Create a Help menu
    menuBar()->insertSeparator();
    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertItem( tr("&Help"), help );
    help->insertSeparator();
    help->insertItem( tr("&About"), this, SLOT(about()), Key_F1);
    help->insertItem( tr("About&Qt"), this, SLOT(aboutQt()));
    
    // Create Viewing window
    QWidget * dlg = man->CreateQuadViewWindow( this );
    setCentralWidget( dlg );
    dlg->showMaximized();
    
    // Create left dock window with tracker status.
    QDockWindow * LeftDock = new QDockWindow( this, "LeftDock" );
    moveDockWindow( LeftDock, Left );
    LeftDock->setResizeEnabled( TRUE );
    QSplitter *splitter = new QSplitter(Qt::Vertical, LeftDock, "SplitList");
    QWidget * objectList = man->CreateSimpleObjectListDialog( splitter );
    LeftDock->setWidget( splitter );
    objectList->show();
    
    QString iconPath = QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY + "/";
    QDir iconDir( iconPath );
    if( iconDir.exists( ) )
    {
        iconPath += IGNS_MINC_VIEW_ICON;
        QFile f(iconPath);
        if (f.exists())
            setIcon(QPixmap::fromMimeSource(iconPath));
    }
    else
    {
        iconPath.sprintf("%s/", IGNS_INSTALLATION_SUBDIRECTORY);
        iconDir.setPath(iconPath);
        if( iconDir.exists( ) )
        iconPath += IGNS_MINC_VIEW_ICON;
        QFile f(iconPath);
        if (f.exists())
            setIcon(QPixmap::fromMimeSource(iconPath));
    }
    
    statusBar()->message( tr("Ready"), 2000 );
    ReadMainWindowSettings();
    man->UpdateBackgroundColor();
}

MainWindow::~MainWindow()
{
    delete m_viewModeSubMenu;
    delete m_viewMenuFunctions;
    delete m_importMenuFunctions;
    delete m_settingsMenuFunctions;
    delete m_toolsMenuFunctions;
}

void MainWindow::about()
{
    AboutBICIgns *a = new AboutBICIgns(this, "AboutIbis", Qt::WDestructiveClose );
    a->Initialize("Minc View");
    a->show();   
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt( this, m_appName );
}

void MainWindow::ExitApp()
{
    WriteMainWindowSettings();
    qApp->closeAllWindows();
}

void MainWindow::fileOpenFile()
{
    // Get base directory for file dialog => last one used if it exists, otherwise home dir
    QString dir = Application::GetSettings()->LastVisitedDirectory;
    if( !QFile::exists( dir ) )
    {
        dir = QDir::homeDirPath();
    }
    
    QStringList res = QFileDialog::getOpenFileNames( "All valid files(*.mnc;*.obj;*.tag;);;Minc file (*.mnc);;Object file (*.obj);;Tag file (*.tag);;All files(*)", dir, qApp->mainWidget(), "open", "Open files" );
    QStringList filenames = res;
    QStringList cantOpen;
    QProgressDialog * progress = new QProgressDialog( "", "Cancel", filenames.size(), 0, 0, TRUE, Qt::WDestructiveClose );
    progress->show();
    int i = 0;
    for( QStringList::Iterator it = filenames.begin(); it != filenames.end(); ++it, ++i )
    {
        if( !Application::GetSceneManager()->OpenFile( *it ) )
        {
            cantOpen += (*it);
        }
        progress->setProgress( i );
        qApp->processEvents();
    }
    progress->close();
    delete progress;
    if( cantOpen.size() )
    {
        QString message;
        message += "The following file(s) could not be open:\n";
        message += cantOpen.join( "\n" );
        QMessageBox::warning( qApp->mainWidget(), "Error", message, 1, 0 );
    }
    
    if( filenames.size() )
    {
        QFileInfo info( *(filenames.begin()) );
        Application::GetSettings()->LastVisitedDirectory = (const char*)info.dirPath( TRUE );
    }
}

void MainWindow::OpenVolumeReconstructionDialog()
{
    m_toolsMenuFunctions->OpenVolumeReconstructionDialog(this);
}

void MainWindow::SetBackgroundColor()
{
    m_settingsMenuFunctions->SetBackgroundColor();
}

void MainWindow::ViewOutlineToggled(bool viewOn)
{
    m_viewMenuFunctions->ViewOutline(viewOn);
}

void MainWindow::ViewXPlaneToggled(bool viewOn)
{
    m_viewMenuFunctions->ViewPlane(viewOn, 0);
}

void MainWindow::ViewYPlaneToggled(bool viewOn)
{
    m_viewMenuFunctions->ViewPlane(viewOn, 1);
}

void MainWindow::ViewZPlaneToggled(bool viewOn)
{
    m_viewMenuFunctions->ViewPlane(viewOn, 2);
}

void MainWindow::View3DPlaneToggled(bool viewOn)
{
    m_viewMenuFunctions->View3DPlanes(viewOn);
}

void MainWindow::View3DFront( )
{
    m_viewMenuFunctions->ViewStandardPosition(SV_FRONT);
}

void MainWindow::View3DLeft( )
{
    m_viewMenuFunctions->ViewStandardPosition(SV_LEFT);
}

void MainWindow::View3DRight( )
{
    m_viewMenuFunctions->ViewStandardPosition(SV_RIGHT);
}

void MainWindow::View3DBack( )
{
    m_viewMenuFunctions->ViewStandardPosition(SV_BACK);
}

void MainWindow::View3DTop( )
{
    m_viewMenuFunctions->ViewStandardPosition(SV_TOP);
}

void MainWindow::View3DBottom( )
{
    m_viewMenuFunctions->ViewStandardPosition(SV_BOTTOM);
}
    
void MainWindow::SetAllObjectsOpacity()
{
    m_viewMenuFunctions->SetAllObjectsOpacity();
}

void MainWindow::ImportMinc( )
{
    m_importMenuFunctions->SetLastVisitedDirectory(Application::GetSettings()->LastVisitedDirectory);
    IMPORTERROR err = m_importMenuFunctions->ImportMinc();
    if (err == IMP_NO_ERR)
        return;
    if (err == IMP_ONLY_ONE)
    {
        QMessageBox::warning( 0, "Error: ", "You may import only one object.", 1, 0 );
    }
    else //err == IMP_NO_OBJECT
    {
        QMessageBox::warning( this, "Error: ", "Please selected an \"obj\" file from the list.", 1, 0 );
    }
}

void MainWindow::ImportObj( )
{
    m_importMenuFunctions->SetLastVisitedDirectory(Application::GetSettings()->LastVisitedDirectory);
    IMPORTERROR err = m_importMenuFunctions->ImportObj();
    if (err == IMP_NO_ERR)
        return;
    if (err == IMP_ONLY_ONE)
    {
        QMessageBox::warning( 0, "Error: ", "You may import only one object.", 1, 0 );
    }
    else //err == IMP_NO_OBJECT
    {
        QMessageBox::warning( this, "Error: ", "Please selected a minc file from the list.", 1, 0 );
    }
}

void MainWindow::RemoveImportedObjOrMinc( )
{
    m_importMenuFunctions->RemoveImportedObjOrMinc();
}

void  MainWindow::EnablePicker( )
{
    m_toolsMenuFunctions->EnablePicker(this);
}

void MainWindow::EnableDisableViewMenu()
{
    SceneManager * manager = Application::GetSceneManager();
    SceneObject *o = manager->GetCurrentObject();
    if (o)
    {
        if (o->GetObjectType() & MINC_TYPE)
        {
            ImageObject * object = reinterpret_cast<ImageObject*>(o);
            viewOutlineAction->setOn((bool)object->GetViewOutline());
            viewX_PlaneAction->setOn((bool)object->GetViewPlane(0));
            viewY_PlaneAction->setOn((bool)object->GetViewPlane(1));
            viewZ_PlaneAction->setOn((bool)object->GetViewPlane(2));
            view3D_PlaneAction->setOn((bool)object->GetView3DPlanes());
            EnableDisableMenuItems(m_viewMenu, 5, TRUE);
        }
    }
    else
    {
        viewOutlineAction->setOn(TRUE);
        viewX_PlaneAction->setOn(TRUE);
        viewY_PlaneAction->setOn(TRUE);
        viewZ_PlaneAction->setOn(TRUE);
        view3D_PlaneAction->setOn(TRUE);
        EnableDisableMenuItems(m_viewMenu, 5, FALSE);
    }
}

void MainWindow::EnableDisableMenuItems( QPopupMenu *menu, int num, bool flag )
{
    int i;
    for ( i = 0; i < num; i++)
    {
        menu->setItemEnabled(menu->idAt(i), flag);
    }
}

void MainWindow::EnableDisableImportMenu()
{
    SceneManager * manager = Application::GetSceneManager();
    if (manager->GetDataObjectsCount() > 0)
    {
        if (manager->FindImportedObject())
        {
            m_importMenu->setItemEnabled(IMP_OBJ, false);
            m_importMenu->setItemEnabled(IMP_MINC, false);
            m_importMenu->setItemEnabled(REM_IMP, true);
        }
        else
        {
            m_importMenu->setItemEnabled(REM_IMP, false);
            m_importMenu->setItemEnabled(IMP_OBJ, true);
            m_importMenu->setItemEnabled(IMP_MINC, true);
            SceneObject *tmpObj = manager->GetCurrentObject();
            if (tmpObj)
            {
                if (tmpObj->GetObjectType() & MINC_TYPE)
                    m_importMenu->setItemEnabled(IMP_MINC, false);
                if (tmpObj->GetObjectType() & OBJ_TYPE)
                    m_importMenu->setItemEnabled(IMP_OBJ, false);
            }
        }
    }
    else
    {
        m_importMenu->setItemEnabled(IMP_OBJ, false);
        m_importMenu->setItemEnabled(IMP_MINC, false);
        m_importMenu->setItemEnabled(REM_IMP, false);
    }
}

void MainWindow::OpenShowMultipleFilesDialog( )
{
    m_toolsMenuFunctions->OpenShowMultipleFilesDialog(this);
}

void MainWindow::SetViewModeToSelectedObject()
{
    m_viewMenuFunctions->ViewMode(VIEW_SELECTED);
}

void MainWindow::SetViewModeToSynchronizePlanes()
{
    m_viewMenuFunctions->ViewMode(VIEW_SYNCHRONIZED);
}

void MainWindow::SetViewModeToAllIn2D()
{
    m_viewMenuFunctions->ViewMode(VIEW_ALL_IN_2D);
}

void MainWindow::SetSelectedViewMode()
{
    SceneManager * manager = Application::GetSceneManager();
    m_viewModeSubMenu->setItemChecked(VIEW_SELECTED, FALSE);
    m_viewModeSubMenu->setItemChecked(VIEW_SYNCHRONIZED, FALSE);
    m_viewModeSubMenu->setItemChecked(VIEW_ALL_IN_2D, FALSE);
    switch (manager->GetViewMode())
    {
    case VIEW_SYNCHRONIZED:
        m_viewModeSubMenu->setItemChecked(VIEW_SYNCHRONIZED, TRUE);
        break;    case VIEW_ALL_IN_2D:
        m_viewModeSubMenu->setItemChecked(VIEW_ALL_IN_2D, TRUE);
        break;
    case VIEW_SELECTED:
    default:
        m_viewModeSubMenu->setItemChecked(VIEW_SELECTED, TRUE);
        break;
    }
}

void MainWindow::ReadMainWindowSettings()
{
    QString configDir = QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY;
    if(!QFile::exists(configDir))
    {
        configDir = QDir::homeDirPath();
    }
    QSettings settings;
    settings.setPath("bic.mni.mcgill.ca", "IGNS");
    settings.insertSearchPath(QSettings::Unix, configDir);
    settings.beginGroup( "/MainWindow" );
        settings.beginGroup("/Geometry");
        int x = settings.readNumEntry("/x", 100);
        int y = settings.readNumEntry("/y", 100);
        int w = settings.readNumEntry("/width", 800);
        int h = settings.readNumEntry("/height", 600);
        settings.endGroup();
    settings.endGroup();
    move(x, y);
    resize(w, h);
}

void MainWindow::WriteMainWindowSettings()
{
    QString configDir = QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY;
    if(!QFile::exists(configDir))
    {
        configDir = QDir::homeDirPath();
    }
    QSettings settings;
    settings.setPath("bic.mni.mcgill.ca", "IGNS");
    settings.insertSearchPath(QSettings::Unix, configDir);
    settings.beginGroup( "/MainWindow" );
        settings.beginGroup( "/Geometry" );
        settings.writeEntry("/x", x());
        settings.writeEntry("/y", y());
        settings.writeEntry("/width", width());
        settings.writeEntry("/height", height());
        settings.endGroup();
    settings.endGroup();
}

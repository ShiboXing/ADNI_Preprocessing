#ifndef __MainWindow_h_
#define __MainWindow_h_

#include <qmainwindow.h>
#include <qstring.h>

class QPopupMenu;
class QAction;
class SceneObject;
class ViewMenuFunctions;
class ImportMenuFunctions;
class SettingsMenuFunctions;
class ToolsMenuFunctions;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow( QWidget * parent = 0, const char * name = 0, WFlags f = WType_TopLevel );
    virtual ~MainWindow();
    
    enum VEW_SUBMENU_ITEMS {SET_SELECTED, SET_SYNC, SET_BLEND, SET_ALL};
    enum IMPORT_MENU_ITEMS {IMP_OBJ, IMP_MINC, REM_IMP};
    QAction* viewOutlineAction;
    QAction* viewX_PlaneAction;
    QAction* viewY_PlaneAction;
    QAction* viewZ_PlaneAction;
    QAction* view3D_PlaneAction;
    QAction* view3D_Front;
    QAction* view3D_Left;
    QAction* view3D_Right;
    QAction* view3D_Back;
    QAction* view3D_Top;
    QAction* view3D_Bottom;
    QAction* viewAllObjectsOpacity;

private slots:

    void about();
    void aboutQt();
    void ExitApp();
    void fileOpenFile();
    void SetBackgroundColor();
    void EnableDisableViewMenu( );
    void EnableDisableImportMenu( );
    void ViewOutlineToggled( bool );
    void ViewXPlaneToggled( bool );
    void ViewYPlaneToggled( bool );
    void ViewZPlaneToggled( bool );
    void View3DPlaneToggled( bool );
    void View3DFront( );
    void View3DLeft( );
    void View3DRight( );
    void View3DBack( );
    void View3DTop( );
    void View3DBottom( );
    void SetAllObjectsOpacity();
    void ImportMinc( );
    void ImportObj( );
    void RemoveImportedObjOrMinc( );
    void EnablePicker( );
    void SetViewModeToSelectedObject();
    void SetViewModeToSynchronizePlanes();
    void SetViewModeToAllIn2D();
    void SetSelectedViewMode();
    void OpenVolumeReconstructionDialog();
    void OpenShowMultipleFilesDialog( );

private:

    QPopupMenu * m_fileMenu;
    QPopupMenu * m_settingsMenu;
    QPopupMenu * m_viewMenu;
    QPopupMenu * m_viewModeSubMenu;
    QPopupMenu * m_importMenu;
    QPopupMenu * m_toolsMenu;
    
    ViewMenuFunctions *m_viewMenuFunctions;
    ImportMenuFunctions *m_importMenuFunctions;
    SettingsMenuFunctions *m_settingsMenuFunctions;
    ToolsMenuFunctions *m_toolsMenuFunctions;
    static const QString m_appName;
    
    void EnableDisableMenuItems( QPopupMenu *menu, int n,  bool flag );
    void ReadMainWindowSettings();
    void WriteMainWindowSettings();
};


#endif

#ifndef __SingleViewWindow_h_
#define __SingleViewWindow_h_

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QSpacerItem;
class vtkQtRenderWindow;
class SceneManager;
class View;

class SingleViewWindow : public QWidget
{
    Q_OBJECT

public:

    SingleViewWindow( QWidget * parent = 0, const char * name = 0, WFlags fl = 0 );
    virtual ~SingleViewWindow();

    virtual void SetSceneManager( SceneManager * man );
    
public slots:
    
    void WinNeedsRender();
    
    void ResetCameraButtonClicked();

protected:
    
    // Description:
    // Get the chance to release things before the window is closed (and maybe
    // destroyed if the Qt::WDestructiveClose flag is set.
    void closeEvent(  QCloseEvent * e );
    
    SceneManager * m_sceneManager;
    
    QVBoxLayout * m_generalLayout;
    
    QHBoxLayout * m_buttonBox;
    QPushButton * m_resetCamerasButton;
    QSpacerItem * m_buttonBoxSpacer; 

    vtkQtRenderWindow * m_vtkWindow;
};


#endif

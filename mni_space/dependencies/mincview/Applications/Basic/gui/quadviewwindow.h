#ifndef __QuadViewWindow_h_
#define __QuadViewWindow_h_

#include <qvariant.h>
#include <qwidget.h>
#include "vtkObjectCallback.h"

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QSpacerItem;
class QSplitter;
class vtkQtRenderWindow;
class SceneManager;
class View;
class vtkRenderWindowInteractor;

enum RenderMode { Immediate, OnSignal };

enum WINTYPES
{ TRANSVERSE_WIN,
  THREED_WIN,
  CORONAL_WIN,
  SAGITTAL_WIN
};

class QuadViewWindow : public QWidget
{
    Q_OBJECT

public:

    QuadViewWindow( QWidget * parent = 0, const char * name = 0, WFlags fl = 0 );
    virtual ~QuadViewWindow();

    virtual void SetSceneManager( SceneManager * man );
    
    void SetRenderMode( RenderMode mode ) { this->Mode = mode; }
    
    void ViewWindowButtonPressed( vtkObject * caller, unsigned long eventId, void * callData );
    
public slots:
    
    void Win0NeedsRender();
    void Win1NeedsRender();
    void Win2NeedsRender();
    void Win3NeedsRender();
    
    void RenderAll();
    
    // Description:
    // This slot should be connected to a signal that tells the graphics windows
    // when they should render. In the case where the render mode is OnSignal, this
    // slot will render the window for which the associated view needs render. In
    // the case where render mode is Immediate, this function will have no effect.
    void RenderSignal();
    
    void ResetCameraButtonClicked();
    void ZoomCameraButtonClicked();
    void ExpandViewButtonClicked();

    void PlaceCornerText();
    
protected:
    
    void WinNeedsRender( int winIndex );
    
    SceneManager * m_sceneManager;
    
    QVBoxLayout * m_generalLayout;
    
    QHBoxLayout * m_buttonBox;
    QPushButton * m_resetCamerasButton;
    QPushButton * m_zoomCamerasButton;
    QPushButton * m_expandViewButton;
    QSpacerItem * m_buttonBoxSpacer; 

    QSplitter * m_verticalSplitter;
    QSplitter * m_upperHorizontalSplitter;
    QSplitter * m_lowerHorizontalSplitter;

    static const QString ViewNames[4];
    int ViewNeedRender[4];
    vtkQtRenderWindow * m_vtkWindows[4];
    vtkRenderWindowInteractor *m_interactors[4];
    vtkObjectCallback<QuadViewWindow> * ViewWindowButtonPressedCallback;
    
    int currentViewExpanded;
    
    // Description: 
    // The render mode tells this class if graphics windows should render when
    // their associated View emits the signal Modified() (Immediate mode) or
    // if the Modified() signal should only set a flag that will tell the class
    // that the window needs to be rendered (OnSignal). In the later case, the
    // RenderSignal() slot will trigger rendering.
    RenderMode Mode;
};


#endif

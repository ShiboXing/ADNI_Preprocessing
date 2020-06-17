#ifndef __View_h_
#define __View_h_

#include <list>
#include "vtkObject.h"
#include <qobject.h>
#include "vtkReal.h"

class vtkInteractorStyle;
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkMulti3DWidget;
class vtkCellPicker;
class SceneManager;
class SceneObject;

enum VIEWTYPES {SAGITTAL_VIEW_TYPE, CORONAL_VIEW_TYPE, TRANSVERSE_VIEW_TYPE, THREED_VIEW_TYPE, OUTLINE_VIEW_TYPE}; //OUTLINE_VIEW_TYPE is used to define outline vilsibility in all views 

extern const char DefaultViewNames[4][20];

class View : public QObject, public vtkObject
{
   
Q_OBJECT

public:
    
    static View * New() { return new View; }
    
    View();
    ~View();
    
    void SetName( QString name ) { this->Name = name; }
    QString GetName() { return this->Name; }
    
    vtkGetMacro(Type,int);
    void SetType( int type );
    
    // Description:
    // Reset the active camera in the renderer of this view
    void ResetCamera();
    void ZoomCamera(double factor);
    
    vtkGetObjectMacro(Interactor,vtkRenderWindowInteractor);
    void SetInteractor( vtkRenderWindowInteractor * interactor );
    
    vtkGetObjectMacro(Renderer,vtkRenderer);
    void SetRenderer( vtkRenderer * renderer );
    
    vtkGetObjectMacro(Picker,vtkCellPicker);
    void SetPicker( vtkCellPicker * picker );
    
    vtkGetObjectMacro(InteractorStyle,vtkInteractorStyle);
    void SetInteractorStyle( vtkInteractorStyle * style );
    
    vtkGetObjectMacro(Manager,SceneManager);
    void SetManager( SceneManager * manager );
    
    void SetBackgroundColor( vtkReal * color );
    
    void ReleaseView();
    
signals:

    void Modified();
    
public slots:

    // Description:
    // Notify the view that something it contains needs render. The view is then
    // going to emit a Modified event.
    void NotifyNeedRender();
    
protected:
    
    void SetupAllObjects();
    void ReleaseAllObjects();
    
    QString Name;
    vtkRenderWindowInteractor * Interactor;
    vtkRenderer * Renderer;
    vtkCellPicker * Picker;
    vtkInteractorStyle * InteractorStyle;
    SceneManager * Manager;
    int Type;
};

#endif
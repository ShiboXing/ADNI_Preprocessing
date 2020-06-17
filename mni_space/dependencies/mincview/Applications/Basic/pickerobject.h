#ifndef TAG_PICKEROBJECT_h_
#define TAG_PICKEROBJECT_h_

#include "vtkReal.h"
#include "vtkMultiImagePlaneWidget.h"
#include "vtkInteractorObserver.h"
#include "sceneobject.h"
#include "imageobject.h"
#include "polydataobject.h"
#include "vtkInteractorStyle.h"
#include "pointsobject.h"
#include "movepointcommand.h"

class QWidget;
class QString;
class PickCommand;
class vtkActor2D;
class vtkTextMapper;
class vtkPoints;
class vtkActor;
class vtkCellPicker;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkPropPicker;
class vtkAssembly;
class View;
class vtkCallbackCommand;
class vtkSphereWithTextPointWidget;
class vtkListOfSphereWithTextPointWidgets;

enum PICKER_OBJECT_TYPE {UNDEFINED, SURFACE, OBJ};

class PickerObject : public SceneObject
{

    Q_OBJECT
            
public:

    static PickerObject * New() { return new PickerObject; }

    vtkSetMacro( PickerType, int );
    vtkGetMacro( PickerType, int );
    vtkGetObjectMacro( ImageObj, ImageObject );
    void SetImageObject( ImageObject * obj );
    vtkGetObjectMacro( PolyDataObj, PolyDataObject );
    vtkSetObjectMacro( PolyDataObj, PolyDataObject );
    vtkGetObjectMacro( ImagePlane, vtkMultiImagePlaneWidget );
    vtkSetObjectMacro( ImagePlane, vtkMultiImagePlaneWidget );
    vtkGetObjectMacro(InteractorStyle,vtkInteractorStyle);
    vtkSetObjectMacro(InteractorStyle,vtkInteractorStyle);

    vtkSetMacro(CurrentPointIndex,int);
    vtkGetMacro(CurrentPointIndex,int);
    vtkGetMacro(PointRadius,vtkReal);
    void SetPointRadius(vtkReal radius);
    
    PickCommand * GetPickObserver() {return pickObserver;}
    vtkCellPicker *GetPicker() {return picker;}
    PointsObject *GetSelectedPoints() {return selectedPoints;}
    vtkActor2D *GetTextActor() {return textActor;}
    vtkActor *GetPickActor() {return pickActor;}

    bool Init();
    void DeInit();

    //picking points 
    bool LoadPointsFromTagFile(QString fileName);
    void DisplayPoints();
    void HighlightPoint(int index);
    int SavePickedPoints(QString fileName);
    void SaveSelectedPoints( );
    void RestoreSelectedPoints( );
    void RemoveAllPoints( );
    int GetNumberOfPoints();
    void SavePointsForUndo(); 
    void UndoPicking(); 
    QString GetPointCoordsString(int idx);
    vtkReal* GetPointCoordinates(int idx);
    void GetPointCoordinates(int idx, vtkReal *pt);
    void SetPointCoordinates(int idx, vtkReal *pt);
    void UpdatePointList();

    bool ReadTagFile( QString & filename );
    void RemoveCrosshairFromViews();
    void SetPlanesPosition(int idx, vtkReal posX, vtkReal posY, vtkReal posZ);
    void EnableInteraction(int yes);
    void DisablePickerOnPlanes();
    void OverrideMouseActions();
    void RestoreMouseActions();

    virtual void PreDisplaySetup();
    virtual void Setup( View * view, vtkAssembly * placementHint ); 
    virtual void Release( View * view); 
    virtual QWidget * CreateSettingsDialog( QWidget * parent ); 
    virtual void HideShow();

    void PickerInteraction(unsigned long event);
    void PickPointOnPlane(vtkMultiImagePlaneWidget *plane);
    void ShowPointCoordinatesIn3DView(int id, vtkReal x, vtkReal y, vtkReal z);
    void HidePointCoordinatesIn3DView();
    void MovePoint(POINT_PROPERTIES *pt);
    void PrintListOfPoints(QString title, vtkListOfSphereWithTextPointWidgets *list);

    void SetRenderer(vtkRenderer * ren);
    void SetInteractor(vtkRenderWindowInteractor * iteract);
    
    virtual void Update();
public slots:

    void AddNewPoint(vtkReal *pt);
    void RefreshObject( );
    void RemovePoint();
            
signals:

    void PointAdded(POINT_PROPERTIES *pt);
    void PointMoved(POINT_PROPERTIES *pt);
    void PointSelected(POINT_PROPERTIES *pt);
    void UpdatePoints();
     
protected:

    int MouseMotion;
    int PickerType;
    bool InitDone;
    int CurrentPointIndex;
    vtkReal PointRadius;
    vtkReal TagLabelSize;
    
    ImageObject *ImageObj;
    PolyDataObject *PolyDataObj;
    vtkMultiImagePlaneWidget *ImagePlane;
    PickCommand* pickObserver;
    vtkRenderer * renderer;
    vtkRenderWindowInteractor * interactor;
    vtkCellPicker *picker;
    vtkActor2D *textActor;
    vtkTextMapper *textMapper;
    vtkActor *pickActor;
    QString objectName;
    vtkPropPicker *savePicker;
    View *selectedView;
    
    MovePointCommand *movePointCommand;
    vtkCallbackCommand *mousePickCommand;
    vtkInteractorStyle * InteractorStyle;
    PointsObject *selectedPoints;
    PointsObject *savedPoints;
    PointsObject *tempPoints;
    
    PickerObject();
    virtual ~PickerObject();
    
private: 

    void WriteTagFile( QString filename );
};

#endif //TAG_PICKEROBJECT_h_

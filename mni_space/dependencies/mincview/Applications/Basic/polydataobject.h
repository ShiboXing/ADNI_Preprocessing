#ifndef __PolyDataObject_h_
#define __PolyDataObject_h_

#include "sceneobject.h"
#include "serializer.h"


class vtkPolyData;
class vtkTransform;
class vtkActor;
class vtkProperty;
class vtkTubeFilter;

enum RenderingMode{ Solid = 0, Wireframe = 1, Both = 2 };

class PolyDataObject : public SceneObject
{
    
Q_OBJECT

public:
        
    static PolyDataObject * New() { return new PolyDataObject; }
    
    PolyDataObject();
    virtual ~PolyDataObject();
    
    void Serialize( Serializer * ser );
    
    vtkGetObjectMacro( PolyData, vtkPolyData );
    void SetPolyData( vtkPolyData * data );
    void SetProperty( vtkProperty * property );
    vtkGetObjectMacro( Property, vtkProperty );
    
    // Implementation of parent virtual method
    virtual void Setup( View * view, vtkAssembly * placementHint );
    virtual void PreDisplaySetup();
    virtual void Release( View * view );
    virtual void HideShow();
    
    virtual QWidget * CreateSettingsDialog( QWidget * parent );
    
    int GetRenderingMode() { return this->renderingMode; }
    void SetRenderingMode( int mode );
    void SetScalarsVisible( int use );
    vtkGetMacro( ScalarsVisible, int );
    double GetOpacity() { return this->opacity; }
    void SetOpacity( double opacity );
    vtkActor *Get3DViewActor();
    void GetColor(double color[3], int fromProperty = 1);
    void SetColor(double color[3]);
    vtkGetMacro( DTITracks, int );
    vtkSetMacro( DTITracks, int );
    void PrepareDTITracks();
    vtkGetMacro( DTITubeResolution, int );
    void SetDTITubeResolution( int res );
    vtkGetMacro( DTITubeRadius, double );
    void SetDTITubeRadius( double );
    void UpdateSettingsWidget();
    
public slots:

     void TransformationChanged(QString, int);
     void UndoLastTransformation(int);
     
signals:
    void RestoreSettingsWidget(int index);
    void ObjectViewChanged();
    
protected:
        
    vtkPolyData * PolyData;
    vtkProperty * Property;
    
    typedef std::map<View*,vtkActor*> PolyDataObjectViewAssociation;
    PolyDataObjectViewAssociation polydataObjectInstances;
    
    int       renderingMode;  // one of VTK_POINTS, VTK_WIREFRAME or VTK_SURFACE
    int       ScalarsVisible;  // Whether scalars in the PolyData are used to color the object or not.
    double    opacity;        // between 0 and 1
    double    objectColor[3];
    
    int DTITracks;
    vtkTubeFilter *DTITubeFilter;
    double DTITubeRadius;
    int DTITubeResolution;
    vtkPolyData * OriginalData;
};

ObjectSerializationHeaderMacro( PolyDataObject );

#endif

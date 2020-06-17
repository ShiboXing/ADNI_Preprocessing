#ifndef TAG_SCENEOBJECT_H
#define TAG_SCENEOBJECT_H

#include "vtkObject.h"
#include <qobject.h>
#include <qstring.h>
#include <map>
#include <vector>
#include <algorithm>
#include "serializer.h"
#include "mincwithobj.h"

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
// IGSTK headers
#include "igstkObject.h"
#include "igstkMacros.h"
#endif
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkAssembly;
class vtkProp3D;
class View;
class vtkLinearTransform;
class vtkTransform;
class vtkMatrix4x4;
class QWidget;
class SceneManager;

enum IGNS_OBJECTTYPE {
    SYSTEM_TYPE = 1, 
    PREOP_ROOT_TYPE = 2,
    INTRAOP_ROOT_TYPE = 4,
    PREOP_TYPE = 8,
    INTRAOP_TYPE = 16,
    MINC_TYPE = 32,
    OBJ_TYPE = 64, 
    REFERENCE_DATA_TYPE = 128,
    PICKER_TYPE = 256,
    IMPORTED_OBJECT_TYPE = 512,
    RESLICED_DATA_TYPE = 1024,
    POINTS_OBJ_TYPE = 2048,
    POINTER_TYPE = 4096,
    USPROBE_TYPE = 8192};

class SceneObject : public QObject, public vtkObject
{
    
Q_OBJECT

public:
        
    static SceneObject * New() { return new SceneObject; }
    
    SceneObject();
    virtual ~SceneObject();
    
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()
#endif
  
    void Serialize( Serializer * ser );
    QString GetName() { return this->Name; }
    void SetName( QString name );
    QString GetDataFileName() { return this->DataFileName; }
    void SetDataFileName( QString DataFileName );
    QString GetFullFileName() { return this->FullFileName; }
    void SetFullFileName( QString FullFileName ) {this->FullFileName = FullFileName;}
    QString GetTransformName() { return this->TransformName; }
    void SetTransformName( QString name );
    
    void SetRegData( vtkLinearTransform * regData );
    
    vtkGetObjectMacro( Transform, vtkTransform );
    vtkGetObjectMacro( RegTransform, vtkLinearTransform );
    virtual void SetTransform( vtkTransform * transform );
    virtual vtkMatrix4x4*GetTransformMatrix();
    virtual void SetTransformMatrix( vtkMatrix4x4 * matrix );
    void NotifyTransformChanged();
    
    virtual void Setup( View * view, vtkAssembly * placementHint );
    virtual void PreDisplaySetup();
    virtual void Release( View * view );
    virtual QWidget * CreateSettingsDialog( QWidget * parent );// { return 0; }
    virtual void HideShow();
        
    virtual void Update();
    virtual void UpdateChildren();
    
    void AddChild( SceneObject * child );
    int GetNumberOfChildren();
    SceneObject * GetChild( int index );
    SceneObject * GetChildOfType( int type );
    SceneObject * GetObject( QString & objectName );
    SceneObject * GetParent() { return this->Parent; }
    void DetachFromParent();
    
    int GetObjectManagedBySystem() {return ObjectType & SYSTEM_TYPE;}
    vtkSetMacro( ObjectType, int );
    vtkGetMacro( ObjectType, int );
    vtkSetMacro( ObjectHidden, bool );
    vtkGetMacro( ObjectHidden, bool );
    vtkGetMacro( NumberOfChildrenInLoadedScene, int );
    vtkSetMacro( ObjectReplacedInSynchronizedView, bool );
    vtkGetMacro( ObjectReplacedInSynchronizedView, bool );
    vtkGetObjectMacro( Manager, SceneManager );
    void SetObjectIsImported(bool);
    bool GetObjectIsImported( );
    
    void SetManager( SceneManager *manager ); 
      
    vtkGetObjectMacro( MincAndObj, MincWithObj );
    vtkSetObjectMacro( MincAndObj, MincWithObj );
    void RemoveMincAndObj();
    void SetRegistrationTransform();
    
    void RefreshObject();
    
    vtkAssembly * GetParentAssembly( View * view );
    std::vector<int> &GetAppliedTransforms();
    
public slots:

     virtual void TransformationChanged(QString);
     virtual void UndoLastTransformation();
      
signals:

    void NameChanged();
    void TransformSet();
    void Modified();
    
protected:
    
    void RemoveFromParent( vtkProp3D * prop, View * view );
    
    QString Name; // name to display on the list
    QString DataFileName; // just the name of the file
    QString FullFileName; // name of the data file including full path
    QString TransformName;
    //RegistrationData * RegData;
    vtkLinearTransform *RegTransform;
    vtkTransform *Transform;
    SceneObject * Parent;
    SceneManager *Manager;
    MincWithObj *MincAndObj;
    
    // The following vector is used to remember which actors were instanciated
    // for every view so we can remove them or add new objects as children
    typedef std::map<View*,vtkAssembly*> ViewAssociation;
    ViewAssociation Instances;
    
    typedef std::vector<SceneObject*> SceneObjectVec;
    SceneObjectVec Children;
    
    typedef std::vector<int> AppliedTransformsVec;
    AppliedTransformsVec AppliedTransforms;
    
    int ObjectType;
    bool ObjectHidden;
    bool ObjectReplacedInSynchronizedView;
private:
    int NumberOfChildrenInLoadedScene;
};

ObjectSerializationHeaderMacro( SceneObject );

#endif //TAG_SCENEOBJECT_H

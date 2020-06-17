#ifndef TAG_SCENEMANAGER_H
#define TAG_SCENEMANAGER_H

#include "vtkObject.h"
#include <qobject.h>
#include <qstring.h>
#include <vector>
#include <algorithm>
#include "vtkLookupTable.h"
#include "vtkReal.h"
#include "mincwithobj.h"
#include "sceneobject.h"
#include "expertmode.h"
#include "serializer.h"
#include "pickerobject.h"

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
// IGSTK headers
#include "igstkObject.h"
#include "igstkMacros.h"
#endif
class View;
class QWidget;
class ImageObject;
class PolyDataObject;
class PickerObject;
class vtkMultiImagePlaneWidget;
class UserTransformations;
class vtkRenderer;
class Reslicer;
class PointsObject;

#define PREOP_ROOT_OBJECT_NAME "Pre-operative"
#define INTRAOP_ROOT_OBJECT_NAME "Intra-operative"
// Description:
// This class is the main interface for all 3D display. It manages the hierarchy
// of 3D objects that compose the scene that can be displayed. It also manages
// the list of different views of the 3D scene that can be displayed in a window
// Furthermore, it is responsible for creating different Qt windows and/or window layouts.

enum STANDARDVIEW {SV_NONE, SV_FRONT, SV_BACK, SV_LEFT, SV_RIGHT, SV_TOP, SV_BOTTOM };
enum VIEWMODE {VIEW_SELECTED, VIEW_SYNCHRONIZED, VIEW_ALL_IN_2D};

class SceneManager : public QObject, public vtkObject
{

    Q_OBJECT
    
public:

    static SceneManager * New() { return new SceneManager; }

    SceneManager();
    ~SceneManager();
    
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()
#endif    
    void Serialize( Serializer * ser );
    
    int OpenFile( QString filename, SceneObject * attachTo = 0,
                  int fileType = PREOP_TYPE,
                  const QString & dataObjectName = "", 
                  const QString & transformName = ""  );

    void OpenMincFile( QString filename, SceneObject * attachTo = 0,
                       int fileType = PREOP_TYPE, 
                       const QString & dataObjectName = "",
                       const QString & transformName = ""  );
    void OpenObjFile( QString filename, SceneObject * attachTo = 0,
                      int fileType = PREOP_TYPE,
                      const QString & dataObjectName = "",
                      const QString & transformName = "" );

    int OpenVTKFile( QString filename, SceneObject * attachTo = 0,
                      int fileType = PREOP_TYPE,
                      const QString & dataObjectName = "",
                      const QString & transformName = "" );
                      
    void OpenTagFile( QString filename, SceneObject * attachTo = 0,
                      int fileType = PREOP_TYPE,
                      const QString & dataObjectName = "",
                      const QString & transformName = "" );
                      
    // Create different view windows and/or window layouts
    QWidget * CreateQuadViewWindow( QWidget * parent );
    QWidget * CreateObjectListDialog( QWidget * parent );
    QWidget * CreateSimpleObjectListDialog( QWidget * parent );
    QWidget * CreateSettingsDialog( QWidget * parent );

    // Description:
    // The three next functions are used to get a pointer to
    // one of the views of the scene. GetView(const char*) will
    // return a view by the name passed in parameter or 0 if
    // no such view exists. GetView(ViewType) will return the
    // first view it finds that is of 'type' and 0 if there
    // is no view of this type. GetOrCreateView() will return
    // a view of type 'type' if there is one, otherwise, it
    // will create one.
    View * CreateView( int type, QString name = QString::null );
    View * GetView( int type );
    View * GetView( const QString & name );
    vtkSetMacro( CurrentView, int );
    vtkGetMacro( CurrentView, int );
    void SetViewBackgroundColor( vtkReal * color );
    void UpdateBackgroundColor();
    vtkGetVector3Macro( ViewBackgroundColor, vtkReal );
    vtkRenderer *GetViewRenderer(int viewType);
    
    vtkGetMacro( ViewMode, int );
    void SetViewMode(int mode);
    void ViewCurrentlySelectedObjectOnlyIn2dViews();
    // Description:
    // Prepare images for synchronized cutting planes
    bool SetSynchronizeOn();
    // Description: 
    // remove synchronized views
    void SetSynchronizeOff(); 
    // Description:
    // reset planes of all images simoultaneously
    void AllImagesResetPlanes();
    // Description:
    // set opacity of all the objects in one shot
    void SetAllObjectsOpacity(double opacity);
    vtkSetMacro( GlobalOpacity, double );
    vtkGetMacro( GlobalOpacity, double );
    
    // Manage Objects
    int GetNumberOfObjects()         { return Objects.size(); }
    int GetDataObjectsCount()        { return DataObjectsCount; }
    int GetMRIDataObjectsCount()        { return MRIDataObjectsCount; }
    SceneObject * GetObject( int i ) { if( i < (int)Objects.size() && i >= 0 ) return Objects[ i ]; return 0; }
    void AddObject( SceneObject * object, SceneObject * attachTo = 0 );
    void RemoveObject( SceneObject * object , bool viewChange = true);
    void AddReconstructedVolumeObject( ImageObject * object, SceneObject * attachTo = 0 );
    SceneObject * GetObject( QString  objectName );
    SceneObject * FindImportedObject( );
    SceneObject * GetCurrentObject( );
    ImageObject * GetReferenceDataObject( );
    void SetReferenceDataObject( ImageObject * );
    SceneObject * GetUniqueTypedObject(int type);
    QString GetCurrentObjectName() { return this->CurrentObjectName; }
    void SetCurrentObject( QString  name );
    void HideShowAllImageObjectsOtherThanCurrentObjectIn2DViews(SceneObject *obj, bool show);
    void ObjectSelectedIn3D(vtkMultiImagePlaneWidget *plane);
    
    vtkGetObjectMacro( PickObj, PickerObject );
    PickerObject *MakePickerObject(ImageObject *obj);
    PickerObject *MakePickerObject(PolyDataObject *obj);
    void DestroyPickerObject();
    bool HasToDestroyPicker(SceneObject* obj);
        
    vtkGetObjectMacro( MincAndObj, MincWithObj );
    void AssociateMincWithObj(ImageObject* iObj, PolyDataObject *pObj);
    void DisAssociateMincWithObj(SceneObject* obj);

    // Description:
    // Function that is called by a window to using this manager. The
    // function is called just before displaying to turn every object on.
    // In particular, objects use this call to enable 3D widgets.
    void PreDisplaySetup();

    // Description:
    // Utility function to reset the cameras in all views.
    void ResetAllCameras();
    void ZoomAllCameras(double factor);
    void SetCameraPositionInAllViews( );
    void SetCameraPositionForObject(ImageObject *obj);
    void MoveCamera(ImageObject *imageObj, int planeIndex);

    // Description:
    // Utility function to detach all views from the render window.
    void ReleaseAllViews();
    
    //Description
    // set one of the standard views (front, back, left, right, top, bottom) in 3D window
    void SetStandardView(STANDARDVIEW type);
    // lookup tables
    void CreateLookupTable(const QString tableName, vtkReal range[2],vtkLookupTable *lut, bool alpha);
    int GetNumberOfTemplateLokupTables() const;
    const QString GetTemplateLookupTableName(int index);
    
    //Description
    // will put all the planes that have the same orientation at the same position
    void MoveAllPlanes(vtkMultiImagePlaneWidget *widget);
    
    //Description
    // for debugging 
    void ListObjectsByName(char *);  

    //Description
    // different objects may share the same transformation or have different transforms, that's whil the 
    //list of transformation is globally set in scene manager
    void SetUserTransforms( UserTransformations * tr );
    UserTransformations * GetUserTransforms( );
    vtkTransform * GetRegistrationTransform();
    bool GetRegistrationFlag();
    void SetExpertMode(ExpertMode *mode) {m_expertMode = mode;}
    ExpertMode *GetExpertMode() {return m_expertMode;}

    //Description
    // Create a PointsObject that will show saved pointer or mouse positions
    PointsObject * CreatePointsObject(const QString & name, SceneObject * attachTo = 0);

    //Description
    // Set SceneSave version found in scene file in order to control loaded variables
    void SetSceneSaveVersionFromFile(const QString &ver) {SceneSaveVersionFromFile = ver;}
    const QString GetSceneSaveVersionFromFile() {return SceneSaveVersionFromFile;}
    
public slots:

    void ObjectNameChangedSlot();
    void TransformChangedSlot();
        
signals:

     void ObjectAdded();
     void ObjectRemoved();
     void ObjectNameChanged();
     void ObjectTransformChanged();
     void ObjectShowHide();

protected:
    ImageObject *ReferenceDataObject;
    int ViewMode;
    int DataObjectsCount;
    int MRIDataObjectsCount;
    int ResliceObjectsCount;
    
    MincWithObj *MincAndObj; // this object is not added to any view, it is used to make association minc-obj
    PickerObject *PickObj;
    UserTransformations *UsedTransforms;
    Reslicer *ImageReslicer;
    void GetRidOfCrossHair();
    // Description:
    // Utility function to setup an object in all views
    void SetupInAllViews( SceneObject * object );
    void SetInitialPositionForAllCameras();
    void SetInitialCameraPosition(int viewTyep);

    void AddImageInSynchronizedView(ImageObject *image);
    
    // Views
    typedef std::vector<View*> ViewList;
    ViewList Views;
    int CurrentView;

    // Objects
    typedef std::vector<SceneObject*> ObjectList;
    ObjectList Objects;
    
    typedef std::map<View*,vtkAssembly*> ViewAssociation;
    ViewAssociation Instances;
    
    QString CurrentObjectName;
    vtkReal ViewBackgroundColor[3];

    double GlobalOpacity;

    QString SupportedSceneSaveVersion;    
    QString SceneSaveVersionFromFile;
    
private:
    
    ExpertMode *m_expertMode;
};

ObjectSerializationHeaderMacro( SceneManager );

#endif //TAG_SCENEMANAGER_H

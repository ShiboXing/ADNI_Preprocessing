#ifndef TAG_IMAGEOBJECT_H
#define TAG_IMAGEOBJECT_H

#include "sceneobject.h"
#include "vtkObjectCallback.h"
#include "vtkReal.h"
#include "vtkVolumeProperty.h"
#include "vtkMatrix4x4.h"
#include "serializer.h"

class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkImageData;
class vtkMultiImagePlaneWidget;
class vtkLookupTable;
class vtkOutlineFilter;
class vtkActor;
class vtkLODProp3D;
class vtkTransform;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class GeneratedSurface;
class Volume;
class vtkMoveCameraCallback;
class vtkObjectSelectedIn3DCallback;
class vtkSaveMousePositionCallback;
class vtkMINCImageAttributes2;

class ImageObject : public SceneObject
{
    
Q_OBJECT

public:
        
    static ImageObject * New() { return new ImageObject; }
    
    ImageObject();
    virtual ~ImageObject();
    
    void Serialize( Serializer * ser );
    
    vtkGetObjectMacro( Image, vtkImageData );
    void SetImage( vtkImageData * image );
    const vtkImageData* const GetImage( vtkImageData * image ) const;
    
    vtkGetObjectMacro( AssociatedObject, ImageObject );
    vtkSetObjectMacro( AssociatedObject, ImageObject );
    
    // Implementation of parent virtual method
    virtual void Setup( View * view, vtkAssembly * placementHint );
    virtual void PreDisplaySetup();
    virtual void Release( View * view );
    virtual QWidget * CreateSettingsDialog( QWidget * parent );
    virtual void HideShow();
    virtual void HideShowIn2DOnly(bool show, View * view);
    
    void SetViewOutline( int isOn );
    int GetViewOutline();
    void SetViewPlane( int planeIndex, int isOn );
    int GetViewPlane( int planeIndex );
    void SetView3DPlanes( int viewOn );
    int GetView3DPlanes();
    void SetViewVolume( int isOn );
    int GetViewVolume();
    void SetViewSurface( int surfaceOn );
    int GetViewSurface();
    void ResetViewError( );
    int GetViewError();
    double GetSurfaceContourValue();
    double GetSurfaceOpacityValue();
    void GetUsedWindowLevel(double *level);
    void SetSurfaceContourValue(double cv);
    void SetSurfaceOpacityValue(double opacity);
    vtkGetObjectMacro( VolumeProperty, vtkVolumeProperty );
    vtkSetObjectMacro( VolumeProperty, vtkVolumeProperty );
    vtkGetObjectMacro( ObjectSurface, GeneratedSurface );
    vtkTransform *GetCurrentTransform();
    vtkMatrix4x4 *GetCurrentTransformationMatrix();
    vtkMultiImagePlaneWidget *GetPlane(int index) {return Planes[index];}
      
    void ResetPlanes();
    void MovePlanes(ImageObject *modelImage);
    
    vtkGetObjectMacro(Lut, vtkLookupTable);
    void SetLut(vtkLookupTable *lut);
    int ChooseColorTable(int index);
    void GetLutRange(vtkReal *range);
    void SetLutRange(vtkReal *range);
    int GetLutIndex() {return lutIndex;}
    void GetImageScalarRange(vtkReal *range);
    void SetAlphaAndRGBObservers(vtkPiecewiseFunction *alphaFunc, vtkColorTransferFunction *rgbFunc);
    
    void SetUsedWindowLevel(int window, int level); 
    
    void SetAllPlanesOpacity(double opacity);
    void GetAllPlanesOpacity(double opacity[3]);
    void SetOnePlaneOpacity(int index, double opacity);
    
    //Gaussian smoothing of the generated surfaceOn
    int GetGaussianSmoothingFlag();
    void SetGaussianSmoothingFlag(int flag);
    double GetRadiusFactor();
    void SetRadiusFactor(double radius);
    void SetStandardDeviation(double deviation);
    double GetStandardDeviation();
    void EmitHidePlaneSignal(int viewType, int show);
            
    vtkGetObjectMacro( DirectionCosines, vtkMatrix4x4 );
    vtkSetObjectMacro( DirectionCosines, vtkMatrix4x4 );
    
    vtkGetObjectMacro( IntraopFinalAppliedTransform, vtkTransform );
    void SetIntraopFinalAppliedTransformMatrix(vtkMatrix4x4 *mat, int index = 0);
    vtkMatrix4x4 *GetIntraopFinalAppliedTransformationMatrix();

    vtkGetObjectMacro( Attributes, vtkMINCImageAttributes2 );
    void SetMINCImageAttributes(vtkMINCImageAttributes2 *att);
    void GetMINCHeaderString(QString & header);

    void SaveMousePosition(double pos[3]);
    void DisableSavingMousePosition();
    void EnableSavingMousePosition();
    
    void UpdateSettingsWidget();

public slots:

     void TransformationChanged(QString, int);
     void UndoLastTransformation(int);
     
signals:
    void HidePlane(int viewType, int show);
    void ObjectViewChanged(int viewType, int show);
    void RestoreSettingsWidget(int index);
    void UpdateSettings();
    
protected:
    
    void Setup3DRepresentation( View * view, vtkAssembly * placementHint ); 
    void Release3DRepresentation( View * view );
    void Setup2DRepresentation( int i, View * view, vtkAssembly * placementHint );
    void Release2DRepresentation( int i, View * view );
    void ComputeInitialTransform(vtkTransform *);
    void CreateLut();
    void CreatePlane( int index );
        
    vtkImageData * Image;
    vtkMINCImageAttributes2 *Attributes;
    vtkMatrix4x4 *DirectionCosines;
    vtkTransform *IntraopFinalAppliedTransform;
    vtkMultiImagePlaneWidget * Planes[3];
    vtkLookupTable * Lut;
    vtkOutlineFilter * OutlineFilter;
    vtkVolumeProperty * VolumeProperty;
    GeneratedSurface *ObjectSurface;
    Volume *volume;
    
    // Associated object is a resampled ImageObject created for the synchronized view
    // When associated object is transformed or removed the corresponding object
    //has to be transformed or removed
    
    ImageObject *AssociatedObject;
    
    int viewOutline;
    int outlineWasVisible;
    int viewPlanes[3];
    int view3DPlanes;
    int viewVolume;
    int viewSurface;
    int viewError;
    int lutIndex;
    int planeVisibleIn2D[3];

    // variables used when saving/loading scenes
    double planeOpacity[3];
    double surfaceContour;
    double surfaceOpacity;
    int gaussianSmoothing;
    double radiusFactor;
    double standardDeviation;
   
 // currentImageWindowSetting corresponds to contrast and is set with windowSlider in ImageObjectSettingDialog
 // the variable is used to preserve last setting when the ImageObjectSettingDialog is not in view
    int currentImageWindowSetting;
 // currentImageLevelSetting corresponds to brightness and is set with levelSlider in ImageObjectSettingDialog
 // the variable is used to preserve last setting when the ImageObjectSettingDialog is not in view
    int currentImageLevelSetting;
    
    struct PerViewElements
    {
        PerViewElements();
        ~PerViewElements();
        vtkAssembly * placementHint;
        vtkActor * outlineActor;
        vtkLODProp3D * lodProp3D;
    };
    
    typedef std::map<View*,PerViewElements*> ImageObjectViewAssociation;
    ImageObjectViewAssociation imageObjectInstances;

    void Refresh3DView();
    vtkObjectCallback<ImageObject> * TransferFunctionModifiedCallback;
    vtkMoveCameraCallback *MoveCameraCallback;
    vtkObjectSelectedIn3DCallback *ObjectSelectedIn3DCallback;
    vtkSaveMousePositionCallback *SaveMousePositionCallback;
};

ObjectSerializationHeaderMacro( ImageObject );

#endif //TAG_IMAGEOBJECT_H


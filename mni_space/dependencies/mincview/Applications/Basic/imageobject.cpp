#include <sstream>
#include <qfileinfo.h>
#include "imageobject.h"
#include "vtkOutlineFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkMultiImagePlaneWidget.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "view.h"
#include "vtkAssembly.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkLODProp3D.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProperty.h"
#include "vtkTransform.h"
#include "vtkLinearTransform.h"
#include "vtkLODActor.h"
#include "vtkIgnsEvents.h"
#include "vtkMINCImageAttributes2.h"
#include "vtkMINC.h"
#include "vtkSmartPointer.h"

#include "imageobjectsettingsdialog.h"
#include "scenemanager.h"
#include "generatedsurface.h"
#include "volume.h"
#include "usertransformations.h"
#include "savepointdialog.h"
#include "pickerobject.h"

ObjectSerializationMacro( ImageObject );

class vtkMoveCameraCallback : public vtkCommand
{
public:
    static vtkMoveCameraCallback *New()
    { return new vtkMoveCameraCallback; }
    virtual void Execute(vtkObject *caller, unsigned long event, void* caldata)
    {
        vtkMultiImagePlaneWidget *widget = reinterpret_cast<vtkMultiImagePlaneWidget*>(caller);
        if (manager)
        {
            SceneObject *obj = manager->GetCurrentObject();
            if (obj->GetObjectType() & MINC_TYPE)
                manager->MoveCamera(reinterpret_cast<ImageObject*>(obj), widget->GetPlaneType());
            if (manager->GetViewMode() == VIEW_SYNCHRONIZED)
                manager->MoveAllPlanes(widget);
        }
    }
    SceneManager *manager;
protected:
    
    vtkMoveCameraCallback() : manager(0){}
    ~vtkMoveCameraCallback() {}
};

class vtkObjectSelectedIn3DCallback : public vtkCommand
{
public:
    static vtkObjectSelectedIn3DCallback *New()
    { return new vtkObjectSelectedIn3DCallback; }
    virtual void Execute(vtkObject *caller, unsigned long event, void* caldata)
    {
        vtkMultiImagePlaneWidget *widget = reinterpret_cast<vtkMultiImagePlaneWidget*>(caller);
        if (manager)
        {
            manager->ObjectSelectedIn3D(widget);
        }
    }
    SceneManager *manager;
protected:
    
    vtkObjectSelectedIn3DCallback() : manager(0) {}
    ~vtkObjectSelectedIn3DCallback() {}
};

class vtkSaveMousePositionCallback : public vtkCommand
{
public:
    static vtkSaveMousePositionCallback *New()
    { return new vtkSaveMousePositionCallback; }
    virtual void Execute(vtkObject *caller, unsigned long event, void* caldata)
    {
        if (manager)
        {
            int windowNum = manager->GetCurrentView();
            int currentPlaneIndex = 0;
            switch (windowNum)
            {
            case 0: //TRANSVERSE_VIEW_TYPE, z fix
                currentPlaneIndex = 2;
                break;
            case 2: //CORONAL_VIEW_TYPE, y fix
                currentPlaneIndex = 1;
                break;
            case 3: //SAGITTAL_VIEW_TYPE, x fix
                currentPlaneIndex = 0;
                break;
            case 1: // THREED_VIEW_TYPE
            default:
                return;
            }
            if (iObj)
            {
                double worldCoords[3] = {0.0,0.0,0.0};
                vtkMultiImagePlaneWidget *plane = iObj->GetPlane(currentPlaneIndex);
                plane->GetCurrentWorldPosition(worldCoords);
                iObj->SaveMousePosition(worldCoords);
            }
        }
    }
    ImageObject *iObj;
    SceneManager *manager;
protected:
    
    vtkSaveMousePositionCallback(): iObj(0),manager(0){}
    ~vtkSaveMousePositionCallback() {}
};

ImageObject::PerViewElements::PerViewElements()
{
    this->outlineActor = 0;    
    this->lodProp3D = 0;
    this->placementHint = 0;   
}   

ImageObject::PerViewElements::~PerViewElements()
{
    this->outlineActor->Delete();
    this->lodProp3D = 0;
    this->placementHint = 0;   
}   

ImageObject::ImageObject()
{
    this->Image = 0;
    for (int i = 0; i < 3; i++)
    {
        this->Planes[i] = 0;
        this->viewPlanes[i] = 1;
        this->planeVisibleIn2D[i] = 1;
        this->planeOpacity[i] = 1.0;
    }
    this->Lut = 0;
    this->OutlineFilter = vtkOutlineFilter::New();
    this->viewOutline = 1;
    this->outlineWasVisible = 1;
    this->view3DPlanes = 1;
    this->viewVolume = 0;
    this->viewSurface = 0;
    this->VolumeProperty = 0;
    this->ObjectSurface = 0;
    this->volume = 0;
    this->viewError = 0;
    this->lutIndex = 0;
    this->currentImageWindowSetting = 0;
    this->currentImageLevelSetting = 0;
    this->surfaceContour = 0.0;
    this->surfaceOpacity = 1.0;
    this->gaussianSmoothing = 0;
    this->radiusFactor = 1.0;
    this->standardDeviation = 1.2;
    this->AppliedTransforms.clear();
    this->MoveCameraCallback = vtkMoveCameraCallback::New();
    this->ObjectSelectedIn3DCallback = vtkObjectSelectedIn3DCallback::New();
    this->TransferFunctionModifiedCallback = 0;
    this->SaveMousePositionCallback = vtkSaveMousePositionCallback::New();
    this->SaveMousePositionCallback->iObj = this;
    this->AssociatedObject = 0;
    this->IntraopFinalAppliedTransform = vtkTransform::New();
    this->IntraopFinalAppliedTransform->Identity();
    this->Attributes = 0;
    this->DirectionCosines = 0;
}

ImageObject::~ImageObject()
{
    int i;
    if( this->Image )
    {
        this->Image->UnRegister( this );
    }
    for(i = 0; i < 3; i++ )
    {
        if (this->Planes[i])
        {
            this->Planes[i]->RemoveAllObservers();
            this->Planes[i]->Delete();
        }
    }
    if( this->Lut )
    {
        this->Lut->Delete();
    }
    if( this->Attributes )
    {
        this->Attributes->Delete();
    }
    this->OutlineFilter->Delete();
    if (this->VolumeProperty)
        this->VolumeProperty->Delete();
    if (this->ObjectSurface)
    {
        this->ObjectSurface->RemoveSurfaceView();
        this->ObjectSurface->Delete();
    }
    if (this->volume)
        this->volume->Delete();
    if (this->TransferFunctionModifiedCallback)
        this->TransferFunctionModifiedCallback->Delete();
    if (this->MoveCameraCallback)
        this->MoveCameraCallback->Delete();
    if (this->ObjectSelectedIn3DCallback)
        this->ObjectSelectedIn3DCallback->Delete();
    if (this->SaveMousePositionCallback)
        this->SaveMousePositionCallback->Delete();
    if (this->IntraopFinalAppliedTransform)
        this->IntraopFinalAppliedTransform->Delete();
}

void ImageObject::Serialize( Serializer * ser )
{
    SceneObject::Serialize(ser);
    ::Serialize( ser, "ViewOutline", this->viewOutline );
    ::Serialize( ser, "viewPlanes", this->viewPlanes, 3 );
    ::Serialize( ser, "View3DPlanes", this->view3DPlanes );
    ::Serialize( ser, "ViewSurface", this->viewSurface );
    ::Serialize( ser, "LutIndex", this->lutIndex );
    ::Serialize( ser, "PlanesOpacity", this->planeOpacity, 3 );
    ::Serialize( ser, "ImageWindow", this->currentImageWindowSetting );
    ::Serialize( ser, "ImageLevel", this->currentImageLevelSetting );
    ::Serialize( ser, "SurfaceContour", this->surfaceContour );
    ::Serialize( ser, "SurfaceOpacity", this->surfaceOpacity );
    ::Serialize( ser, "SurfaceGaussianSmoothing", this->gaussianSmoothing );
    ::Serialize( ser, "SurfaceRadiusFactor", this->radiusFactor );
    ::Serialize( ser, "SurfaceStandardDeviation", this->standardDeviation );
}

void ImageObject::SetImage( vtkImageData * image )
{
    if( this->Image == image )
    {
        return;
    }

    if( this->Image )
    {
        this->Image->UnRegister( this );
    }
    this->Image = image;
    if( this->Image )
    {
        this->Image->Register( this );
    }

    this->OutlineFilter->SetInput( this->Image );

    this->CreateLut();
}

void ImageObject::Setup( View * view, vtkAssembly * placementHint )
{
    switch( view->GetType() )
    {
        case SAGITTAL_VIEW_TYPE:
            this->Setup2DRepresentation( 0, view, placementHint );
            break;
        case CORONAL_VIEW_TYPE:
            this->Setup2DRepresentation( 1, view, placementHint );
            break;
        case TRANSVERSE_VIEW_TYPE:
            this->Setup2DRepresentation( 2, view, placementHint );
            break;
        case THREED_VIEW_TYPE:
            this->Setup3DRepresentation( view, placementHint );
            break;
    }
    if (this->ObjectType & INTRAOP_TYPE || this->ObjectType & RESLICED_DATA_TYPE)
    {
        this->SetViewOutline(false); // because of this the outline actors will not be used
    }
    connect( this, SIGNAL( Modified() ), view, SLOT( NotifyNeedRender() ) );
}

void ImageObject::PreDisplaySetup()
{
    for( int i = 0; i < 3; i++ )
    {
        if( this->Planes[i] && this->viewPlanes[i] )
        {
            this->Planes[i]->EnabledOn();
            if (i == 0)
            {
                double level[2];
                this->GetUsedWindowLevel(level);
                currentImageWindowSetting = (int)level[0];
                currentImageLevelSetting = (int)level[1];
            }
        }
    }
    this->MoveCameraCallback->manager = this->Manager;
    this->ObjectSelectedIn3DCallback->manager = this->Manager;
    this->SaveMousePositionCallback->manager = this->Manager;
    this->EnableSavingMousePosition();
}

void ImageObject::Release( View * view )
{
    switch( view->GetType() )
    {
        case SAGITTAL_VIEW_TYPE:
            this->Release2DRepresentation( 0, view );
            break;
        case CORONAL_VIEW_TYPE:
            this->Release2DRepresentation( 1, view );
            break;
        case TRANSVERSE_VIEW_TYPE:
            this->Release2DRepresentation( 2, view );
            break;
        case THREED_VIEW_TYPE:
            this->Release3DRepresentation( view );
            break;
    }
    this->disconnect( view, SLOT( NotifyNeedRender() ) );
}

QWidget * ImageObject::CreateSettingsDialog( QWidget * parent )
{
    ImageObjectSettingsDialog * res = new ImageObjectSettingsDialog( parent, "ImageObjectSettingsDialog", Qt::WDestructiveClose );
    res->SetImageObject( this );
    res->SetWindowAndLevel( this->currentImageWindowSetting, this->currentImageLevelSetting );
    if (this->ObjectType & INTRAOP_TYPE)
        res->SetAppliedTransforms(this->AppliedTransforms);
    connect( res, SIGNAL( TransformationChanged(QString, int) ), this, SLOT(TransformationChanged(QString, int)) );
    connect( this, SIGNAL( RestoreSettingsWidget(int) ), res, SLOT(ShowLastUsedWidget(int)) );
    connect( this, SIGNAL( HidePlane(int, int) ), res, SLOT(UpdatePlaneSettings(int, int)) );
    connect( this, SIGNAL( ObjectViewChanged(int, int) ), res, SLOT(UpdatePlaneSettings(int, int)) );
    connect( this, SIGNAL( UpdateSettings() ), res, SLOT(UpdateUI()) );
    connect( res, SIGNAL( UndoTransformation(int) ), this, SLOT(UndoLastTransformation(int)) );
    return res;
}

void ImageObject::UpdateSettingsWidget()
{
    emit UpdateSettings();
}

void ImageObject::SetViewOutline( int isOn )
{
    if( this->viewOutline == isOn )
    {
        return;
    }

    this->viewOutline = isOn;

    if( isOn )
    {
        ImageObjectViewAssociation::iterator it = this->imageObjectInstances.begin();
        for( ; it != this->imageObjectInstances.end(); ++it )
        {
            if( (*it).second->outlineActor )
            {
                if ((*it).first->GetType() == THREED_VIEW_TYPE)
                    (*it).second->outlineActor->VisibilityOn();
                this->outlineWasVisible = 1;
            }
        }
    }
    else
    {
        ImageObjectViewAssociation::iterator it = this->imageObjectInstances.begin();
        for( ; it != this->imageObjectInstances.end(); ++it )
        {
            if( (*it).second->outlineActor )
            {
                (*it).second->outlineActor->VisibilityOff();
                this->outlineWasVisible = 0;
            }
        }
    }

    emit Modified();
    emit ObjectViewChanged(OUTLINE_VIEW_TYPE, isOn);
}

int ImageObject::GetViewOutline()
{
    return this->viewOutline;
}

void ImageObject::SetViewPlane( int planeIndex, int isOn )
{
    this->Planes[ planeIndex ]->SetEnabled( isOn );
    this->viewPlanes[ planeIndex ] = isOn;
    emit Modified();
    emit ObjectViewChanged(planeIndex, isOn);
}

int ImageObject::GetViewPlane( int planeIndex )
{
    return this->viewPlanes[ planeIndex ];
}

void ImageObject::SetViewVolume( int isOn )
{
    if( this->viewVolume == isOn )
    {
        return;
    }

    this->viewVolume = isOn;
    if (isOn)
    {
        this->volume = Volume::New();
        this->volume->DisplayVolume(this);
        vtkLODProp3D * lodProp = this->volume->GetVolumeProp3D();
        ImageObjectViewAssociation::iterator itAssociations = this->imageObjectInstances.begin();
        for( ; itAssociations != this->imageObjectInstances.end(); ++itAssociations )
        {
            if ( (*itAssociations).first->GetType() == THREED_VIEW_TYPE )
            {
                (*itAssociations).second->lodProp3D = lodProp;
                if( (*itAssociations).second->placementHint )
                    (*itAssociations).second->placementHint->AddPart( lodProp );
                else
                    (*itAssociations).first->GetRenderer()->AddViewProp((*itAssociations).second->lodProp3D );
            }
        }
    }
    else
    {
        ImageObjectViewAssociation::iterator itAssociations = this->imageObjectInstances.begin();
        for( ; itAssociations != this->imageObjectInstances.end(); ++itAssociations )
        {
            if ( (*itAssociations).second->lodProp3D )
            {
                if( (*itAssociations).second->placementHint )
                    (*itAssociations).second->placementHint->RemovePart( (*itAssociations).second->lodProp3D );
                else
                    (*itAssociations).first->GetRenderer()->RemoveViewProp( (*itAssociations).second->lodProp3D );
                (*itAssociations).second->lodProp3D = 0;
            }
        }
        if (this->VolumeProperty)
        {
            this->VolumeProperty->Delete();
            this->VolumeProperty = 0;
        }
        if (this->volume)
        {
            this->volume->Delete();
            this->volume = 0;
        }
        if (this->TransferFunctionModifiedCallback)
        {
            this->TransferFunctionModifiedCallback->Delete();
            this->TransferFunctionModifiedCallback = 0;
        }
    }
    emit Modified();
}

int ImageObject::GetViewVolume()
{
    return this->viewVolume;
}

void ImageObject::SetViewSurface( int surfaceOn )
{
    if( this->viewSurface == surfaceOn )
    {
        return;
    }
    View *v = this->Manager->GetView(THREED_VIEW_TYPE);
    if (v)
    {
        if (surfaceOn)
        {
            vtkReal sp[3];
            this->Image->GetSpacing(sp);
            if (sp[0] < 0 || sp[1] < 0 || sp[2] < 0)
            {
                this->viewError = 1;
                this->viewSurface = 0;
                return;
            }
            this->ObjectSurface = GeneratedSurface::New();
            this->ObjectSurface->SetRenderer(v->GetRenderer());
            this->ObjectSurface->SetInteractor(v->GetInteractor());
            this->ObjectSurface->SetImageObject( this );
            this->ObjectSurface->DisplaySurface();
            this->surfaceContour = this->ObjectSurface->GetContourValue();
            this->surfaceOpacity = 1.0;
        }
        else
        {
            if (this->ObjectSurface)
            {
                if (this->Manager->HasToDestroyPicker((SceneObject*)this))
                    this->Manager->DestroyPickerObject();
                this->ObjectSurface->RemoveSurfaceView();
                this->ObjectSurface->Delete();
                this->ObjectSurface = 0;
                this->surfaceContour = 0.0;
                this->surfaceOpacity = 1.0;
            }
        }
        this->viewSurface = surfaceOn;
        v->NotifyNeedRender();
    }
}

int ImageObject::GetViewSurface()
{
    return this->viewSurface;
}

void ImageObject::ResetViewError( )
{
    this->viewError = 0;
}

int ImageObject::GetViewError()
{
    return this->viewError;
}

void ImageObject::ResetPlanes()
{
    int extent[6];
    this->Image->GetExtent( extent );
    for( int i = 0; i < 3; i++ )
    {
        int length = extent[ 2 * i + 1 ] - extent[ 2 * i ] + 1;
        this->Planes[ i ]->SetPlaneOrientation( i );
        this->Planes[ i ]->SetSliceIndex( length / 2 );
        this->Manager->MoveCamera(this, i);
    }
    this->Manager->GetView(SAGITTAL_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
    this->Manager->GetView(CORONAL_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
    this->Manager->GetView(TRANSVERSE_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
    emit Modified();
}


void ImageObject::Setup3DRepresentation( View * view, vtkAssembly * placementHint )
{
    vtkAssembly *intraopAssembly = 0;
    if (this->ObjectType & INTRAOP_TYPE)
    {
       intraopAssembly = vtkAssembly::New();
       view->GetRenderer()->AddViewProp( intraopAssembly );
       if (this->Transform)
            intraopAssembly->SetUserTransform(Transform);
    }
    
    for( int i = 0; i < 3; i++ )
    {
        if( this->Planes[ i ] == 0 )
        {
            this->CreatePlane( i );
        }
        this->Planes[i]->AddInteractor( view->GetInteractor() );
        if (this->ObjectType & INTRAOP_TYPE)
        {
            this->Planes[i]->AddRenderer( view->GetRenderer(), intraopAssembly);
        }
        else
            this->Planes[i]->AddRenderer( view->GetRenderer(), placementHint );
        this->Planes[i]->SetPicker( this->Planes[i]->GetNumberOfRenderers() - 1, view->GetPicker() );
    }

    // add an outline to the view to make sure camera includes the whole volume
    vtkPolyDataMapper * outMapper = vtkPolyDataMapper::New();
    outMapper->SetInput( this->OutlineFilter->GetOutput() );
    vtkActor * outActor = vtkActor::New();
    outActor->SetMapper( outMapper );
    outMapper->Delete();
    if( this->viewOutline )
    {
        if (this->ObjectType & INTRAOP_TYPE)
        {
            intraopAssembly->AddPart( outActor );
        }
        else
        {
            if( placementHint )
                placementHint->AddPart( outActor );
            else
                view->GetRenderer()->AddActor( outActor );
        }
    }

    // Add the actor to the map of instances we keep
    PerViewElements * elem = new PerViewElements;
    elem->outlineActor = outActor;
    if (this->ObjectType & INTRAOP_TYPE)
        elem->placementHint = intraopAssembly;
    else
        elem->placementHint = placementHint;
    this->imageObjectInstances[ view ] = elem;
}

void ImageObject::Release3DRepresentation( View * view )
{
    // release the planes
    for( int i = 0; i < 3; i++ )
    {
        if( this->Planes[i] )
        {
            this->Planes[i]->RemoveInteractor( view->GetInteractor() );
            this->Planes[i]->RemoveRenderer( view->GetRenderer() );
        }
    }
    if (this->viewSurface)
    {
        if (this->ObjectSurface)
        {
            this->ObjectSurface->RemoveSurfaceView();
            this->ObjectSurface->Delete();
            this->ObjectSurface = 0;
        }
    }
    // release the outline
    ImageObjectViewAssociation::iterator itAssociations = this->imageObjectInstances.find( view );
    if( itAssociations != this->imageObjectInstances.end() )
    {
        PerViewElements * perView = (*itAssociations).second;

        if (this->ObjectType & INTRAOP_TYPE)
        {
            if (perView->placementHint)
            {
                perView->placementHint->RemovePart( perView->outlineActor );
                if ( perView->lodProp3D )
                {
                    perView->placementHint->RemovePart(perView->lodProp3D);
                }
                perView->placementHint->Delete();
                perView->placementHint = 0;
            }
            else if ( perView->lodProp3D )
            {
                view->GetRenderer()->RemoveViewProp(perView->lodProp3D);
            }
        }
        else
        {
            this->RemoveFromParent( perView->outlineActor, view );
            if ( perView->lodProp3D )
                this->RemoveFromParent( perView->lodProp3D , view );
        }
        delete perView;
        this->imageObjectInstances.erase( itAssociations );
    }
}

int ImageObject::GetView3DPlanes( )
{
    return this->view3DPlanes;
}

void ImageObject::SetView3DPlanes( int viewOn )
{
    if (viewOn == view3DPlanes )
        return;
    int i;
    ImageObjectViewAssociation::iterator itAssociations = this->imageObjectInstances.begin();
    View *v = 0;
    vtkAssembly *a = 0;
    for( ; itAssociations != this->imageObjectInstances.end() && v == 0; ++itAssociations )
    {
        if ( (*itAssociations).first->GetType() == THREED_VIEW_TYPE )
        {
            v = (*itAssociations).first;
            a = (*itAssociations).second->placementHint;
        }
    }
    if (v)
    {
        if (!viewOn)
        {
            for( i = 0; i < 3; i++ )
            {
                if( this->Planes[i] )
                {
                    this->Planes[i]->RemoveInteractor( v->GetInteractor() );
                    this->Planes[i]->RemoveRenderer( v->GetRenderer() );
                }
            }
            view3DPlanes = 0;
        }
        else
        {
            for( i = 0; i < 3; i++ )
            {
                if( this->Planes[i] )
                {
                    this->Planes[i]->AddInteractor( v->GetInteractor() );
                    this->Planes[i]->AddRenderer( v->GetRenderer(), a );
                    this->Planes[i]->SetPicker( this->Planes[i]->GetNumberOfRenderers() - 1, v->GetPicker() );
                    this->Planes[i]->SetEnabled(0); // this is cheating, but I do not know how to make planes visible
                    this->Planes[i]->SetEnabled(1);
                }
            }
            view3DPlanes = 1;
        }
        v->NotifyNeedRender();
        emit ObjectViewChanged(THREED_VIEW_TYPE, viewOn);
    }
}

void ImageObject::Setup2DRepresentation( int viewType, View * view, vtkAssembly * placementHint )
{
    if( this->Planes[ viewType ] == 0 )
    {
        this->CreatePlane( viewType );
    }
    vtkAssembly *intraopAssembly = 0;
    if (this->ObjectType & INTRAOP_TYPE)
    {
       intraopAssembly = vtkAssembly::New();
       view->GetRenderer()->AddViewProp( intraopAssembly );
       if (this->Transform)
            intraopAssembly->SetUserTransform(Transform);
    }
    this->Planes[viewType]->AddInteractor( view->GetInteractor() );
    if (this->ObjectType & INTRAOP_TYPE)
    {
        this->Planes[viewType]->AddRenderer( view->GetRenderer(), intraopAssembly);
    }
    else
        this->Planes[viewType]->AddRenderer( view->GetRenderer(), placementHint );
    this->Planes[viewType]->SetPicker( this->Planes[viewType]->GetNumberOfRenderers() - 1, view->GetPicker() );

    // add an outline to the view to make sure camera includes the whole volume
    vtkPolyDataMapper * outMapper = vtkPolyDataMapper::New();
    outMapper->SetInput( this->OutlineFilter->GetOutput() );
    vtkActor * outActor = vtkActor::New();
    outActor->SetMapper( outMapper );
    outMapper->Delete();
    if( this->viewOutline )
    {
        if (this->ObjectType & INTRAOP_TYPE)
        {
            intraopAssembly->AddPart( outActor );
        }
        else
        {
            if (placementHint)
                placementHint->AddPart(outActor);
            else
                view->GetRenderer()->AddActor( outActor );
        }
    }
    // make the outline invisible in 2D, don't remove it yet, I may need it later
    outActor->VisibilityOff();

    // Add the actor to the map of instances we keep
    PerViewElements * elem = new PerViewElements;
    elem->outlineActor = outActor;
    if (this->ObjectType & INTRAOP_TYPE)
        elem->placementHint = intraopAssembly;
    else
        elem->placementHint = placementHint;
    this->imageObjectInstances[ view ] = elem;
}

void ImageObject::Release2DRepresentation( int viewType, View * view )
{
    // release the planes
    if( this->Planes[viewType] )
    {
        this->Planes[viewType]->RemoveInteractor( view->GetInteractor() );
        this->Planes[viewType]->RemoveRenderer( view->GetRenderer() );
    }

    // release the outline
    ImageObjectViewAssociation::iterator itAssociations = this->imageObjectInstances.find( view );
    if( itAssociations != this->imageObjectInstances.end() )
    {
        PerViewElements * perView = (*itAssociations).second;

        // delete the vtkAssembly we have
        this->RemoveFromParent( perView->outlineActor, view );
        if (this->ObjectType & INTRAOP_TYPE)
        {
            perView->placementHint->Delete();
            perView->placementHint= 0;
        }
        delete perView;
        this->imageObjectInstances.erase( itAssociations );
    }
}

void ImageObject::CreateLut()
{
    if( !this->Image )
    {
        vtkErrorMacro( << "Image must be set before trying to create LUT." );
        return;
    }

    vtkReal range[2];
    this->Image->GetScalarRange( range );
    this->Lut = vtkLookupTable::New();
    this->Lut->SetTableRange(range[0],range[1]);
    this->Lut->SetHueRange( 1, 1 );
    this->Lut->SetSaturationRange( 0, 0 );
    this->Lut->SetValueRange( 0, 1 );
    this->Lut->Build();
}


void ImageObject::CreatePlane( int viewType )
{            
    if( this->Planes[ viewType ] == 0 )
    {
        int extent[6];
        this->Image->GetExtent( extent );
        int length = extent[ 2 * viewType + 1 ] - extent[ 2 * viewType ] + 1;
        this->Planes[viewType] = vtkMultiImagePlaneWidget::New();
        this->Planes[viewType]->ControlDisablesOn();
        this->Planes[viewType]->TextureInterpolateOff();
        this->Planes[viewType]->SetInput( this->Image );
        this->Planes[viewType]->SetPlaneOrientation( viewType );
        this->Planes[viewType]->SetSliceIndex( length / 2 );
        this->Planes[viewType]->SetLeftButtonAction( vtkMultiImagePlaneWidget::SLICE_MOTION_ACTION );
        this->Planes[viewType]->SetRightButtonAction( vtkMultiImagePlaneWidget::CURSOR_ACTION );
        this->Planes[viewType]->SetMiddleButtonAction( vtkMultiImagePlaneWidget::WINDOW_LEVEL_ACTION );
        this->Planes[viewType]->UserControlledLookupTableOn();
        this->Planes[viewType]->SetLookupTable( this->Lut );
        this->Planes[viewType]->DisplayTextOn ();
        this->Planes[viewType]->SetResliceInterpolateToLinear();
        this->Planes[viewType]->SetPlaneType(viewType);
        this->Planes[viewType]->AddObserver(vtkCommand::InteractionEvent, this->MoveCameraCallback);
        this->Planes[viewType]->AddObserver( vtkCommand::UserEvent+IGNS_PLANE_SELECTED, this->ObjectSelectedIn3DCallback);
    }
}

void ImageObject::SetLut(vtkLookupTable *lut)
{
    if (this->Lut != lut)
    {
        if (this->Lut != NULL)
        {
            this->Lut->UnRegister(0);
        }
        this->Lut = lut;
        this->Planes[0]->SetLookupTable(lut);
        this->Planes[1]->SetLookupTable(lut);
        this->Planes[2]->SetLookupTable(lut);
        if (this->Lut != NULL)
        {
            this->Lut->Register(0);
        }
    }
    emit Modified();
}

int ImageObject::ChooseColorTable(int index)
{
    if (this->lutIndex == index)
        return 0; 
    this->lutIndex = index;
    QString tableName = this->Manager->GetTemplateLookupTableName( index );
    vtkReal range[2];
    this->Image->GetScalarRange( range );
    vtkLookupTable *lut = vtkLookupTable::New();
    this->Manager->CreateLookupTable( tableName, range, lut, this->ObjectType & RESLICED_DATA_TYPE);
    this->SetLut(lut);
    if (this->ObjectSurface)
        this->ObjectSurface->SetContourLut(this->Lut);
    lut->Delete();
    // control associated reslice color
    if (this->AssociatedObject && (this->AssociatedObject->GetObjectType() & RESLICED_DATA_TYPE))
    {
        this->AssociatedObject->ChooseColorTable(this->lutIndex);
    }
    emit Modified();
    return 1;
}

void ImageObject::SetAlphaAndRGBObservers(vtkPiecewiseFunction *alphaFunc, vtkColorTransferFunction *rgbFunc)
{
    this->TransferFunctionModifiedCallback = vtkObjectCallback<ImageObject>::New();
    this->TransferFunctionModifiedCallback->SetCallback( this, &ImageObject::Refresh3DView );
    alphaFunc->AddObserver(vtkCommand::ModifiedEvent, this->TransferFunctionModifiedCallback, 0);
    rgbFunc->AddObserver(vtkCommand::ModifiedEvent, this->TransferFunctionModifiedCallback, 0);
}

void ImageObject::Refresh3DView()
{
    View *view3D = this->Manager->GetView(THREED_VIEW_TYPE);
    view3D->NotifyNeedRender();
}

double ImageObject::GetSurfaceContourValue()
{
    if (this->ObjectSurface)
        this->surfaceContour = this->ObjectSurface->GetContourValue();
    return this->surfaceContour;
}

void ImageObject::SetSurfaceContourValue(double cv)
{
    if (cv > 0 && this->ObjectSurface)
    {
        this->ObjectSurface->SetContourValue(cv);
        this->surfaceContour = cv;
        emit Modified();
    }
}

double ImageObject::GetSurfaceOpacityValue()
{
    if (this->ObjectSurface)
        this->surfaceOpacity = this->ObjectSurface->GetOpacityValue();
    return this->surfaceOpacity;
}

void ImageObject::SetSurfaceOpacityValue(double opacity)
{
    if (this->ObjectSurface)
    {
        this->ObjectSurface->SetOpacityValue(opacity);
        this->surfaceOpacity = opacity;
        emit Modified();
    }
}

vtkTransform * ImageObject::GetCurrentTransform()
{
    if (this->Parent && this->Parent->GetTransform())
        return (this->Parent->GetTransform());
    else
        return this->Transform;
}

vtkMatrix4x4 *ImageObject::GetCurrentTransformationMatrix()
{
    if (this->ObjectType & INTRAOP_TYPE)
    {
        ImageObjectViewAssociation::iterator it = this->imageObjectInstances.begin();
        for( ; it != this->imageObjectInstances.end(); ++it )
        {
            if ((*it).first->GetType() == THREED_VIEW_TYPE)
            {
                vtkAssembly *parentAssembly = (*it).second->placementHint;
                if (parentAssembly)
                {
                    if (parentAssembly->GetUserTransform())
                        return (parentAssembly->GetUserTransform()->GetMatrix());
                    else
                        return (parentAssembly->GetMatrix());
                }
                else if (this->Transform)
                    return this->Transform->GetMatrix();
                else
                    return 0;
            }
        }
    }
    if (this->Parent && this->Parent->GetTransform())
        return (this->Parent->GetTransform()->GetMatrix());
    else if (this->Transform)
        return this->Transform->GetMatrix();
    else
        return 0;
}

void ImageObject::GetUsedWindowLevel(double *level)
{
    if (!level)
        return;
    if (this->Planes[0])
    {
        this->Planes[0]->GetWindowLevel(level);
        currentImageWindowSetting = (int)level[0];
        currentImageLevelSetting = (int)level[1];
    }
    else
    {
        level[0] = currentImageWindowSetting;
        level[1] = currentImageLevelSetting;
    }
}

void ImageObject::SetUsedWindowLevel(int window, int level) 
{
    currentImageWindowSetting = window;
    currentImageLevelSetting = level;
    for(int i = 0; i < 3; i++ )
    {
        this->Planes[i]->SetWindowLevel((vtkReal)window, (vtkReal)level);
    }
    // control associated reslice window and level
    if (this->AssociatedObject && (this->AssociatedObject->GetObjectType() & RESLICED_DATA_TYPE))
    {
        vtkReal range[2], range1[2], scale;
        this->Image->GetScalarRange(range);
        this->AssociatedObject->GetImageScalarRange(range1);
        scale = (range1[1]-range1[0])/(range[1]-range[0]);
        this->AssociatedObject->SetUsedWindowLevel(window*scale, level*scale);
    }
    emit Modified();
}
    
void ImageObject::SetAllPlanesOpacity(double opacity)
{
    vtkProperty *prop = 0;
    
    for(int i = 0; i < 3; i++ )
    {
        planeOpacity[i] = opacity;
        prop = this->Planes[i]->GetTexturePlaneProperty();
        if (prop)
            prop->SetOpacity(opacity);
    }
    // control associated reslice opacity
    if (this->AssociatedObject && (this->AssociatedObject->GetObjectType() & RESLICED_DATA_TYPE))
    {
        this->AssociatedObject->SetAllPlanesOpacity(opacity);
    }
    emit Modified();
}

void ImageObject::GetAllPlanesOpacity(double opacity[3])
{
    vtkProperty *prop = 0;

    for(int i = 0; i < 3; i++ )
    {
        if (this->Planes[i])
        {
            prop = this->Planes[i]->GetTexturePlaneProperty();
            if (prop)
            {
                opacity[i] = prop->GetOpacity();
                planeOpacity[i] = opacity[i];
            }
        }
        else
        opacity[i] = planeOpacity[i];
    }
}

void ImageObject::SetOnePlaneOpacity(int index, double opacity)
{
    if (index < 0 || index > 2)
        return;
    planeOpacity[index] = opacity;
    vtkProperty *prop = this->Planes[index]->GetTexturePlaneProperty();
    if (prop)
    {
        prop->SetOpacity(opacity);
        emit Modified();
    }
    // control associated reslice opacity
    if (this->AssociatedObject && (this->AssociatedObject->GetObjectType() & RESLICED_DATA_TYPE))
    {
        this->AssociatedObject->SetOnePlaneOpacity(index, opacity);
    }
}

void ImageObject::GetLutRange(vtkReal *range)
{
    if (this->Lut != 0)
    {
        this->Lut->GetTableRange(range);
    }
    else
    {
        range[0] = 0.0;
        range[1] = 0.0;
    }
}

void ImageObject::SetLutRange(vtkReal *range)
{
    if (this->Lut != 0 && range != 0)
    {
        this->Lut->SetTableRange(range);
        emit Modified();
    }
}

void ImageObject::GetImageScalarRange(vtkReal *range)
{
    this->Image->GetScalarRange( range );
}

void ImageObject::HideShow()
{
    int i;
    if(this->ObjectReplacedInSynchronizedView && this->AssociatedObject)
    {
        this->AssociatedObject->SetObjectHidden(this->ObjectHidden);
        this->AssociatedObject->HideShow();
    }
    if(this->ObjectHidden || this->ObjectReplacedInSynchronizedView)
    {
        for( i = 0; i < 3; i++ )
        {
            if( this->Planes[i] )
            {
                this->Planes[i]->SetEnabled(0);
            }
        }
        if (this->viewOutline)
        {
            this->SetViewOutline(0);
            this->outlineWasVisible = 1;
        }
        else
            this->outlineWasVisible = 0;
        if (this->ObjectSurface)
        {
            this->ObjectSurface->RemoveSurfaceView();
        }
        if (this->viewVolume)
        {
            ImageObjectViewAssociation::iterator itAssociations = this->imageObjectInstances.begin();
            for( ; itAssociations != this->imageObjectInstances.end(); ++itAssociations )
            {
                if ( (*itAssociations).first->GetType() == THREED_VIEW_TYPE )
                {
                    if( (*itAssociations).second->placementHint )
                        (*itAssociations).second->placementHint->RemovePart( (*itAssociations).second->lodProp3D );
                    else
                        (*itAssociations).first->GetRenderer()->RemoveViewProp( (*itAssociations).second->lodProp3D );
                    (*itAssociations).second->lodProp3D = 0;
                }
            }
        }
    }
    else
    {
        for( i = 0; i < 3; i++ )
        {
            if( this->Planes[i] )
            {
                this->Planes[i]->SetEnabled(1);
            }
            if (this->outlineWasVisible)
                this->SetViewOutline(1);
        }
        if (this->ObjectSurface)
        {
            this->ObjectSurface->ShowSurfaceView();
        }
        if (this->viewVolume)
        {
            vtkLODProp3D * lodProp = this->volume->GetVolumeProp3D();
            ImageObjectViewAssociation::iterator itAssociations = this->imageObjectInstances.begin();
            for( ; itAssociations != this->imageObjectInstances.end(); ++itAssociations )
            {
                if ( (*itAssociations).first->GetType() == THREED_VIEW_TYPE )
                {
                    (*itAssociations).second->lodProp3D = lodProp;
                    if( (*itAssociations).second->placementHint )
                        (*itAssociations).second->placementHint->AddPart( lodProp );
                    else
                        (*itAssociations).first->GetRenderer()->AddViewProp((*itAssociations).second->lodProp3D );
                }
            }
        }
    }
}

//Gaussian smoothing of the generated surfaceOn
int ImageObject::GetGaussianSmoothingFlag()
{
    if (this->ObjectSurface)
        this->gaussianSmoothing = this->ObjectSurface->GetGaussianSmoothingFlag();
    return this->gaussianSmoothing;
}

void ImageObject::SetGaussianSmoothingFlag(int flag)
{
    if (this->ObjectSurface)
    {
        this->ObjectSurface->SetGaussianSmoothingFlag(flag);
        this->gaussianSmoothing = flag;
    }
}

double ImageObject::GetRadiusFactor()
{
    if (this->ObjectSurface)
        this->radiusFactor = this->ObjectSurface->GetRadiusFactor();
    return this->radiusFactor;
}

void ImageObject::SetRadiusFactor(double radius)
{
    if (this->ObjectSurface)
    {
        this->ObjectSurface->SetRadiusFactor(radius);
        this->radiusFactor = radius;
        emit Modified();
    }
}

double ImageObject::GetStandardDeviation()
{
    if (this->ObjectSurface)
        this->standardDeviation = this->ObjectSurface->GetStandardDeviation();
    return this->standardDeviation;
}

void ImageObject::SetStandardDeviation(double deviation)
{
    if (this->ObjectSurface)
    {
        this->ObjectSurface->SetStandardDeviation(deviation);
        this->standardDeviation = deviation;
        emit Modified();
    }
}

void ImageObject::TransformationChanged(QString transformName, int index)
{
    this->TransformName = transformName;
    vtkTransform *newTransform = this->Manager->GetUserTransforms()->GetUserTransform(transformName);
    int type = this->Manager->GetUserTransforms()->GetTransformType(transformName);
    if (type == ABSOLUTE_TRANSFORM)
    {
        this->IntraopFinalAppliedTransform->SetMatrix(newTransform->GetMatrix());
    }
    else
    {
        this->IntraopFinalAppliedTransform->Concatenate(newTransform);
    }
    if (this->ObjectType & INTRAOP_TYPE)
    {
        ImageObjectViewAssociation::iterator it = this->imageObjectInstances.begin();
        for( ; it != this->imageObjectInstances.end(); ++it )
        {
            vtkAssembly *intraopAssembly = (*it).second->placementHint;
            if( intraopAssembly )
            {
                    intraopAssembly->SetUserTransform(this->IntraopFinalAppliedTransform);
            }
        }
        this->AppliedTransforms.push_back(this->Manager->GetUserTransforms()->GetTransformIndex(this->TransformName));
        if (!this->Manager->GetUserTransforms()->TransformIsPredefined(transformName))
        {
            this->Manager->GetUserTransforms()->IncrementReferenceCounter(transformName);
        }
        emit TransformSet();
        this->Manager->GetView(SAGITTAL_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
        this->Manager->GetView(CORONAL_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
        this->Manager->GetView(TRANSVERSE_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
        emit Modified();
        emit RestoreSettingsWidget(index);
    }
}

void ImageObject::UndoLastTransformation(int index)
{
    if (AppliedTransforms.size() > 1 && this->ObjectType & INTRAOP_TYPE)
    {
        QString transformName;
        int lastTransformIndex = this->AppliedTransforms.back();
        transformName = this->Manager->GetUserTransforms()->GetTransformName(lastTransformIndex);
        vtkMatrix4x4 *lastTransformMatrix = this->Manager->GetUserTransforms()->GetTransformMatrix(lastTransformIndex);
        vtkMatrix4x4 *invertedLastTransformMatrix = vtkMatrix4x4::New();
        invertedLastTransformMatrix->Identity();
        vtkMatrix4x4::Invert(lastTransformMatrix, invertedLastTransformMatrix);
        this->IntraopFinalAppliedTransform->Concatenate(invertedLastTransformMatrix);
        ImageObjectViewAssociation::iterator it = this->imageObjectInstances.begin();
        for( ; it != this->imageObjectInstances.end(); ++it )
        {
            vtkAssembly *intraopAssembly = (*it).second->placementHint;
            if( intraopAssembly )
            {
                intraopAssembly->SetUserTransform(this->IntraopFinalAppliedTransform);
            }
        }
        invertedLastTransformMatrix->Delete();
        this->AppliedTransforms.pop_back();
        if (this->Manager->GetViewMode() == VIEW_SYNCHRONIZED)
        {
            this->Manager->AllImagesResetPlanes();
        }
        this->SetTransformName(this->Manager->GetUserTransforms()->GetTransformName(this->AppliedTransforms.back()));
        if (!this->Manager->GetUserTransforms()->TransformIsPredefined(transformName))
        {
            this->Manager->GetUserTransforms()->DecrementReferenceCounter(transformName);
        }
        emit TransformSet();
        this->Manager->GetView(SAGITTAL_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
        this->Manager->GetView(CORONAL_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
        this->Manager->GetView(TRANSVERSE_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
        emit Modified();
        emit RestoreSettingsWidget(index);
    }
}

void ImageObject::HideShowIn2DOnly(bool show, View * view)
{
    int viewType = view->GetType();
    if (viewType != THREED_VIEW_TYPE && this->Planes[viewType] )
    {
        if (show)
        {
            if (!this->planeVisibleIn2D[viewType])
            {
                this->Planes[viewType]->AddInteractor( view->GetInteractor() );
                this->Planes[viewType]->AddRenderer( view->GetRenderer(), this->GetParentAssembly(view));
                this->Planes[viewType]->SetPicker( this->Planes[viewType]->GetNumberOfRenderers() - 1, view->GetPicker() );
                this->Planes[viewType]->SetEnabled(0); // this is cheating, but I do not know how to make planes visible
                this->Planes[viewType]->SetEnabled(1);
                this->planeVisibleIn2D[viewType] = 1;
                if (this->outlineWasVisible)
                    this->SetViewOutline(1);
            }
        }
        else
        {
            this->Planes[viewType]->RemoveInteractor( view->GetInteractor() );
            this->Planes[viewType]->RemoveRenderer( view->GetRenderer() );
            this->planeVisibleIn2D[viewType] = 0;
            this->SetViewOutline(0);
        }
    }
}

void ImageObject::EmitHidePlaneSignal(int viewType, int show)
{
    emit HidePlane(viewType, show);
}

void ImageObject::MovePlanes(ImageObject *modelImage)
{
    int i;
    vtkMultiImagePlaneWidget *modelPlane;
    vtkReal origin[3], point1[3], point2[3];
    for (i = 0; i < 3; i++)
    {
        modelPlane = modelImage->GetPlane(i);
        modelPlane->GetOrigin(origin);
        modelPlane->GetPoint1(point1);
        modelPlane->GetPoint2(point2);
        this->Planes[i]->SetOrigin(origin);
        this->Planes[i]->SetPoint1(point1);
        this->Planes[i]->SetPoint2(point2);
        this->Planes[i]->UpdatePlacement();
    }
}

void ImageObject::SetMINCImageAttributes(vtkMINCImageAttributes2 *att)
{
    if (!this->Attributes)
    {
        this->Attributes = vtkMINCImageAttributes2::New();
    }
    this->Attributes->ShallowCopy(att);
}

void ImageObject::GetMINCHeaderString(QString & header)
{
    if (!this->Attributes)
    {
        header = "No info available.\n";     
    }
    else
    {
        std::ostrstream info;
        this->Attributes->PrintFileHeader(info);
        header = info.str();
    }
}

void ImageObject::SetIntraopFinalAppliedTransformMatrix(vtkMatrix4x4 *mat, int index)
{
    if (mat && this->ObjectType & INTRAOP_TYPE)
    {
        IntraopFinalAppliedTransform->SetMatrix(mat);
        ImageObjectViewAssociation::iterator it = this->imageObjectInstances.begin();
        for( ; it != this->imageObjectInstances.end(); ++it )
        {
            vtkAssembly *intraopAssembly = (*it).second->placementHint;
            if( intraopAssembly )
            {
                intraopAssembly->SetUserTransform(this->IntraopFinalAppliedTransform);
            }
        }
        this->AppliedTransforms.push_back(index);
        this->Manager->GetView(SAGITTAL_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
        this->Manager->GetView(CORONAL_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
        this->Manager->GetView(TRANSVERSE_VIEW_TYPE)->GetRenderer()->ResetCameraClippingRange();
        emit Modified();
    }
}

vtkMatrix4x4 * ImageObject::GetIntraopFinalAppliedTransformationMatrix()
{
    if (IntraopFinalAppliedTransform)
        return IntraopFinalAppliedTransform->GetMatrix();
    return 0;
}

void ImageObject::SaveMousePosition(double pos[3])
{
    QFileInfo info( this->FullFileName );
    QString saveDirectory = info.dirPath( true );
    SavePointDialog *dialog = new SavePointDialog;
    dialog->SetSaveDirectory(saveDirectory);
    dialog->SetManager(this->Manager);
    // we need points in MRI space, so after registration, we have to apply inverse of registration
    if (this->Manager->GetRegistrationFlag())
    {
        double *newPos;;
        vtkSmartPointer< vtkTransform > tmp = vtkSmartPointer< vtkTransform >::New();
        tmp->Identity();
        tmp->SetInput(((this->Manager->GetUserTransforms())->GetRegistrationTransform())->GetLinearInverse());
        newPos = tmp->TransformDoublePoint(pos[0], pos[1], pos[2]);
        dialog->SetCurrentPointCoords(newPos);
    }
    else
        dialog->SetCurrentPointCoords(pos);
    dialog->show();
}

void ImageObject::DisableSavingMousePosition()
{
    for (int j = 0; j < 3; j++)
    {
        View * view = this->Manager->GetView(j);
        view->GetInteractor()->RemoveObserver(this->SaveMousePositionCallback);
    }
}

void ImageObject::EnableSavingMousePosition()
{
    for (int j = 0; j < 3; j++)
    {
        View * view = this->Manager->GetView(j);
        view->GetInteractor()->AddObserver(vtkCommand::UserEvent+IGNS_SAVE_MOUSE_POSITION, this->SaveMousePositionCallback);
    }
}


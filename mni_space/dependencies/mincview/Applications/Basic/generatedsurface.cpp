#include <iostream>

#include <qdir.h>
#include <qfiledialog.h>

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkContourFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkProperty.h"
#include "vtkPolyData.h" 
#include "vtkLODActor.h"
#include "vtkImageResample.h"
#include "vtkTransform.h"
#include "vtkAppendPolyData.h"
#include "vtkDepthSortPolyData.h"
#include "vtkImageGaussianSmooth.h"

#include "generatedsurface.h"
#include "imageobject.h"

vtkCxxSetObjectMacro( GeneratedSurface, Transform, vtkTransform );

GeneratedSurface::GeneratedSurface()
{
    this->object = 0;
    this->renderer = 0;
    this->interactor = 0;
    this->contourExtractor = vtkContourFilter::New();
    this->contourActor = vtkLODActor::New();
    this->contourMapper = vtkPolyDataMapper::New();
    this->objectName = "";
    this->contourLut = vtkLookupTable::New();
    this->minScalar = 0.0;
    this->maxScalar = 0.0;
    this->opacity = 1.0;
    this->maxContourVal = 0.0;
    this->Transform = 0;
    this->radius = DEFAULT_RADIUS;
    this->standardDeviation = DEFAULT_STANDARD_DEVIATION;
    this->gaussianSmoothing = 0;
}

GeneratedSurface::~GeneratedSurface()
{
    if (this->object)
        this->object->UnRegister(this);
    if (this->renderer)
        this->renderer->UnRegister( this );
    if (this->interactor)
        this->interactor->UnRegister( this );
    if (this->Transform)
        this->Transform->UnRegister(this);
    if (this->contourLut)
        this->contourLut->Delete();
    if (this->contourMapper)
        this->contourMapper->Delete();
    if (this->contourActor)
        this->contourActor->Delete();
    if (this->contourExtractor)
        this->contourExtractor->Delete();
}
    
void GeneratedSurface::SetImageObject( ImageObject *obj )
{
    if( this->object == obj )
        return;
    if (this->object)
        this->object->UnRegister(this);
    this->object = obj;
    if (obj)
    {
        this->object->Register(this);
        this->SetContourLut(obj->GetLut());
        this->SetSurfaceObjectName(obj->GetName());
        if (obj->GetObjectType() & PREOP_TYPE)
            this->SetTransform( obj->GetCurrentTransform() );
        else
            this->SetTransform( obj->GetIntraopFinalAppliedTransform() );
        GetImageMinMax(obj->GetImage(), &this->minScalar, &this->maxScalar);
        if (this->minScalar == 0.0  &&  this->maxScalar == 0.0)
            maxContourVal = VTK_UNSIGNED_CHAR_MAX/5.0; // this should not happen, both are 0 if image scalar type is not unsigned char nor unsigned short
        else
            maxContourVal = this->maxScalar/5.0; //the best seems to be 20% of max, tested on Colin27
    }
}

void GeneratedSurface::SetRenderer( vtkRenderer *ren )
{
    if( this->renderer == ren )
        return;
    if( this->renderer )
        this->renderer->UnRegister( this );
    this->renderer = ren;
    if( this->renderer )
        this->renderer->Register( this );
}

void GeneratedSurface::SetInteractor( vtkRenderWindowInteractor *iren )
{
    if( this->interactor == iren )
        return;
    if( this->interactor )
        this->interactor->UnRegister( this );
    this->interactor = iren;
    if( this->interactor )
        this->interactor->Register( this );
}

//Color table may be changed on the fly
void GeneratedSurface::SetContourLut(vtkLookupTable *imageLut)
{
    vtkReal temp[2];
    this->contourLut->SetTableRange(minScalar, maxScalar); 
    imageLut->GetHueRange(temp); 
    this->contourLut->SetHueRange(temp);  
    imageLut->GetSaturationRange(temp); 
    this->contourLut->SetSaturationRange(temp);  
    imageLut->GetValueRange(temp); 
    this->contourLut->SetValueRange(temp);  
    this->contourLut->Build();
    this->contourMapper->SetLookupTable(this->contourLut);
}

void GeneratedSurface::SetSurfaceObjectName( QString name )
{
    this->objectName = name;
}

void GeneratedSurface::DisplaySurface()
{
    vtkReal sp[3];
    vtkImageData *image = this->object->GetImage();
    image->GetSpacing(sp);
    
    vtkImageResample * resampler = vtkImageResample::New();
    resampler->SetInput( image );
    resampler->SetAxisMagnificationFactor( 0, 0.5 );
    resampler->SetAxisMagnificationFactor( 1, 0.5 );
    resampler->SetAxisMagnificationFactor( 2, 0.5 );
      
    if (this->gaussianSmoothing)
    {
        vtkImageGaussianSmooth *GaussianSmooth = vtkImageGaussianSmooth::New();
        GaussianSmooth->SetInput(resampler->GetOutput());
        GaussianSmooth->SetStandardDeviation(this->standardDeviation);
        GaussianSmooth->SetRadiusFactor(this->radius);
        GaussianSmooth->Update();
        contourExtractor->SetInput( GaussianSmooth->GetOutput() );
        GaussianSmooth->Delete();
    }
    else
    {
        contourExtractor->SetInput( resampler->GetOutput() );
    }
    contourExtractor->SetValue( 0, maxContourVal );

    resampler->Delete();
    
    if (this->Transform)
        this->contourActor->SetUserTransform(this->Transform);
    if ( this->opacity < 1.0 )
    {
        // sorting polygons is very slow
        vtkAppendPolyData *contourData = vtkAppendPolyData::New();
        contourData->SetInput(contourExtractor->GetOutput());
        
        // The dephSort object is set up to generate scalars representing
        // the sort depth.  A camera is assigned for the sorting. The camera
        // define the sort vector (position and focal point).
        vtkCamera *camera = this->renderer->GetActiveCamera();
        vtkDepthSortPolyData *depthSort = vtkDepthSortPolyData::New();
        depthSort->SetInput(contourData->GetOutput());
        depthSort->SetDirectionToBackToFront();
        depthSort->SetCamera( camera );
        this->contourMapper->SetInput( depthSort->GetOutput() );
        this->contourActor->SetMapper( this->contourMapper );
        this->contourActor->SetNumberOfCloudPoints(VTK_INT_MAX);
        // If an Prop3D is supplied, then its transformation matrix is taken
        // into account during sorting.
        depthSort->SetProp3D( this->contourActor );
        contourData->Delete();
        depthSort->Delete();
    }
    else
    {
        this->contourMapper->SetInput( contourExtractor->GetOutput() );
        this->contourActor->SetMapper( this->contourMapper );
        this->contourActor->SetNumberOfCloudPoints(VTK_INT_MAX);
    }
    vtkProperty *contourProp = contourActor->GetProperty();
    contourProp->SetOpacity(this->opacity);
    this->renderer->AddActor( this->contourActor );
}

void GeneratedSurface::RemoveSurfaceView()
{
    this->renderer->RemoveActor( this->contourActor );
}

void GeneratedSurface::ShowSurfaceView()
{
    if (this->contourActor)
        this->renderer->AddActor( this->contourActor );
}

void GeneratedSurface::GetImageMinMax( vtkImageData * image, double *minval, double *maxval )
{
    // compute image max and min
    int * dims = image->GetDimensions();
    int size = dims[0] * dims[1] * dims[2];
    int scalarType = image->GetScalarType();
    int i;
    if (scalarType == VTK_UNSIGNED_CHAR)
    {
        unsigned char minScalar = VTK_UNSIGNED_CHAR_MAX-1;
        unsigned char maxScalar = VTK_UNSIGNED_CHAR_MIN;
        unsigned char * scalarPtr = (unsigned char*)image->GetScalarPointer();
        for( i = 0; i < size; i++ )
        {
            if( *scalarPtr < minScalar )
                minScalar = *scalarPtr;
            if( *scalarPtr > maxScalar )
                maxScalar = *scalarPtr;
            ++scalarPtr;
        }
        *maxval = maxScalar;
        *minval = minScalar;
    }
    else if (scalarType == VTK_UNSIGNED_SHORT)
    {
        unsigned short min = VTK_UNSIGNED_SHORT_MAX-1;
        unsigned short max = VTK_UNSIGNED_SHORT_MIN;
        unsigned short * ptr = (unsigned short*)image->GetScalarPointer();
        for( i = 0; i < size; i++ )
        {
            if( *ptr < min )
                min = *ptr;
            if( *ptr > max )
                max = *ptr;
            ++ptr;
        }
        *maxval = max;
        *minval = min;
    }
    else // no support
    {
        *maxval = 0.0;
        *minval = 0.0;
    }
}

vtkReal GeneratedSurface::GetContourValue()
{
    return this->maxContourVal;
}

void GeneratedSurface::SetContourValue(vtkReal cv)
{
    if (cv == this->maxContourVal)
        return;
    if (cv > 0 && cv != this->maxContourVal)
    {
        this->maxContourVal = cv;
        this->RemoveSurfaceView();
        this->DisplaySurface();
    }
}

double GeneratedSurface::GetOpacityValue()
{
    return this->opacity;
}

void GeneratedSurface::SetOpacityValue(double op)
{
    if (opacity < 0.0)
        opacity = 0.0;
    if (opacity > 1.0)
        opacity = 1.0;
    if (op == this->opacity)
        return;
    
    this->opacity = op;    
    this->RemoveSurfaceView();
    this->DisplaySurface();
}

//Gaussian smoothing of the gener surfaceOn
void GeneratedSurface::SetGaussianSmoothingFlag(int flag)
{
    gaussianSmoothing = flag;
    this->RemoveSurfaceView();
    this->DisplaySurface();
}

void GeneratedSurface::SetRadiusFactor(double radiusFactor) 
{
    radius = radiusFactor;
    this->RemoveSurfaceView();
    this->DisplaySurface();
}

void GeneratedSurface::SetStandardDeviation(double deviation) 
{
    standardDeviation = deviation;
    this->RemoveSurfaceView();
    this->DisplaySurface();
}

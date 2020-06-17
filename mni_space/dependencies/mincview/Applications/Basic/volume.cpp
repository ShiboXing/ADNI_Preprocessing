// This code comes from Simon's example vtkMinc/DisplayMincVolumeLOD
// I mapped the Image range to 0-255 interval used in the example.
// This seems to depend to much on the data set - colin27.mnc
// With non-isometric (1x1x1.5) data sets only transverse slices can form a 3D image.
#include <iostream>

#include "vtkImageData.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkVolumeTextureMapper2D.h"
#include "vtkImageResample.h"
#include "vtkLODProp3D.h"
#include "vtkObjectCallback.h"
#include "vtkLinearTransform.h"
#include "vtkTransform.h"
#include "imageobject.h"
#include "volume.h"

Volume::Volume()
{
    this->object = 0;
    this->lodProp3D = 0;
}

Volume::~Volume()
{
    if (this->lodProp3D)
        this->lodProp3D->Delete();
}

void Volume::DisplayVolume(ImageObject *data)
{
    if (!data || this->object == data)
        return;
    this->object = data;
    //===================================================================
    // Create the volume properties
    //===================================================================

    vtkReal range[2];
    vtkImageData *Image = this->object->GetImage();
    Image->GetScalarRange( range );
    // Create transfer mapping scalar value to opacity
    // 
    vtkPiecewiseFunction * alphaFunc = vtkPiecewiseFunction::New();
    alphaFunc->AddPoint(range[0], 0.0);
    alphaFunc->AddPoint((range[1]-range[0]), 1.0);
    
    // Create transfer mapping scalar value to color
    // we cannot add new points, so I split the image range into 5 equal segments
    // later we have EditVolumeTransferFunctionWidget that also uses 5 RGB points
    // and build a gray scale function
    // the image may come up rather dark
    vtkColorTransferFunction * rgbFunc = vtkColorTransferFunction::New();
    rgbFunc->AddRGBPoint(range[0], 0.0, 0.0, 0.0); 
    rgbFunc->AddRGBPoint((range[1]-range[0])/5.0, 0.2, 0.2, 0.2);
    rgbFunc->AddRGBPoint((range[1]-range[0])*2.0/5.0, 0.4, 0.4, 0.4);
    rgbFunc->AddRGBPoint((range[1]-range[0])*3.0/5.0, 0.6, 0.6, 0.6);
    rgbFunc->AddRGBPoint((range[1]-range[0])*4.0/5.0, 0.8, 0.8, 0.8);
    rgbFunc->AddRGBPoint((range[1]-range[0]), 1.0, 1.0, 1.0);
    

    // The property describes how the data will look
    vtkVolumeProperty *volumeProperty = vtkVolumeProperty::New();
    volumeProperty->SetColor(rgbFunc);
    volumeProperty->SetScalarOpacity(alphaFunc);
    //===================================================================
    // Create the texture plane mapper for level 2
    //===================================================================

    vtkVolumeTextureMapper2D * textureMapper = vtkVolumeTextureMapper2D::New();
    textureMapper->SetInput( Image );
    //===================================================================
    // Create the texture plane mapper with subsampled image for level 3
    //===================================================================
    vtkImageResample * resampler = vtkImageResample::New();
    resampler->SetInput( Image );
    resampler->SetAxisMagnificationFactor( 0, 0.5 );
    resampler->SetAxisMagnificationFactor( 1, 0.5 );
    resampler->SetAxisMagnificationFactor( 2, 0.5 );
    vtkVolumeTextureMapper2D * textureMapperSub = vtkVolumeTextureMapper2D::New();
    textureMapperSub->SetInput( resampler->GetOutput() );
    resampler->Delete();
    //===================================================================
    // Create the LOD actor that will hold all levels of details of the
    // image and decide which one to display.
    //===================================================================

    this->lodProp3D = vtkLODProp3D::New();
    this->lodProp3D->AddLOD( textureMapper, volumeProperty, 0.0 );
    this->lodProp3D->AddLOD( textureMapperSub, volumeProperty, 0.0 );
    vtkLinearTransform *tmp = vtkTransform::SafeDownCast(this->object->GetCurrentTransform());
    this->lodProp3D->SetUserTransform( tmp );
    this->lodProp3D->PickableOff();
    textureMapper->Delete();
    textureMapperSub->Delete();
    this->object->SetAlphaAndRGBObservers(alphaFunc, rgbFunc);
    this->object->SetVolumeProperty(volumeProperty);
    volumeProperty->Delete();
    alphaFunc->Delete();
    rgbFunc->Delete();
}


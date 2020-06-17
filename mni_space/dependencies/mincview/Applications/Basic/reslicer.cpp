#include <iostream>

#include "vtkImageData.h"
#include "vtkImageReslice.h"
#include "vtkTransform.h"
#include "vtkImageMapToColors.h"
#include "vtkLookupTable.h"
#include "vtkImageShiftScale.h"
#include "scenemanager.h"
#include "imageobject.h"
#include "reslicer.h"

Reslicer::Reslicer():
    m_baseForResliceObject(0),
    m_baseForResliceImage(0)
{
}

Reslicer::~Reslicer()
{
    m_baseForResliceObject->UnRegister(0);
}

void Reslicer::ResliceImageData(ImageObject *data, vtkImageData *reslicedImage)
{
    if (!m_baseForResliceObject || !data || !reslicedImage)
        return;
    vtkTransform *baseTransform = m_baseForResliceObject->GetCurrentTransform();
    vtkImageData *image = data->GetImage();
    vtkTransform *identityTransform = vtkTransform::New();
    identityTransform->Identity();
    vtkTransform *concatTransform = vtkTransform::New();
    concatTransform->Identity();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    vtkMatrix4x4 *tmpMat;
#endif    
    if (baseTransform )//&& data->GetObjectType() & INTRAOP_TYPE)
    {
        concatTransform->PostMultiply();
        concatTransform->Concatenate(baseTransform);
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        tmpMat = baseTransform->GetMatrix();
        igstkLogMacro( DEBUG, "base transform :" << "\n" <<
        tmpMat->Element[0][0] << ", " << 
        tmpMat->Element[0][1] << ", " <<
        tmpMat->Element[0][2] << ", " << 
        tmpMat->Element[0][3] << "\n" << 
        tmpMat->Element[1][0] << ", " << 
        tmpMat->Element[1][1] << ", " <<
        tmpMat->Element[1][2] << ", " <<
        tmpMat->Element[1][3] << "\n" <<
        tmpMat->Element[2][0] << ", " << 
        tmpMat->Element[2][1] << ", " <<
        tmpMat->Element[2][2] << ", " <<
        tmpMat->Element[2][3] << 
        endl);
        tmpMat = concatTransform->GetMatrix();
        igstkLogMacro( DEBUG, "concat transform :" << "\n" <<
        tmpMat->Element[0][0] << ", " << 
        tmpMat->Element[0][1] << ", " <<
        tmpMat->Element[0][2] << ", " << 
        tmpMat->Element[0][3] << "\n" << 
        tmpMat->Element[1][0] << ", " << 
        tmpMat->Element[1][1] << ", " <<
        tmpMat->Element[1][2] << ", " <<
        tmpMat->Element[1][3] << "\n" <<
        tmpMat->Element[2][0] << ", " << 
        tmpMat->Element[2][1] << ", " <<
        tmpMat->Element[2][2] << ", " <<
        tmpMat->Element[2][3] << 
        endl);
#endif    
    }
    vtkMatrix4x4 *mat = 0;
    if (data->GetObjectType() & INTRAOP_TYPE)
    {
        mat = data->GetCurrentTransformationMatrix();
        if (mat)
        {
            mat->Invert();
            concatTransform->Concatenate(mat);
        }
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "inverted obj. transform :" << "\n" <<
        mat->Element[0][0] << ", " << 
        mat->Element[0][1] << ", " <<
        mat->Element[0][2] << ", " << 
        mat->Element[0][3] << "\n" << 
        mat->Element[1][0] << ", " << 
        mat->Element[1][1] << ", " <<
        mat->Element[1][2] << ", " <<
        mat->Element[1][3] << "\n" <<
        mat->Element[2][0] << ", " << 
        mat->Element[2][1] << ", " <<
        mat->Element[2][2] << ", " <<
        mat->Element[2][3] << 
        endl);
        tmpMat = concatTransform->GetMatrix();
        igstkLogMacro( DEBUG, "concat transform :" << "\n" <<
        tmpMat->Element[0][0] << ", " << 
        tmpMat->Element[0][1] << ", " <<
        tmpMat->Element[0][2] << ", " << 
        tmpMat->Element[0][3] << "\n" << 
        tmpMat->Element[1][0] << ", " << 
        tmpMat->Element[1][1] << ", " <<
        tmpMat->Element[1][2] << ", " <<
        tmpMat->Element[1][3] << "\n" <<
        tmpMat->Element[2][0] << ", " << 
        tmpMat->Element[2][1] << ", " <<
        tmpMat->Element[2][2] << ", " <<
        tmpMat->Element[2][3] << 
        endl);
#endif    
    }
        
    vtkReal scalarRange[2];
    data->GetImageScalarRange( scalarRange );
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    igstkLogMacro( DEBUG, "ResliceImageData input scalar range =  " << scalarRange[0] << ", " << scalarRange[1] << endl);
#endif    
    double scale = ((scalarRange[1]-scalarRange[0])/scalarRange[1])*(VTK_UNSIGNED_CHAR_MAX/scalarRange[1]);
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    igstkLogMacro( DEBUG, "ResliceImageData scale = " << scale << endl);
#endif    
   
    vtkImageShiftScale *scaledImage = vtkImageShiftScale::New();
    scaledImage->SetInput(image);
    scaledImage->SetScale(scale);
    scaledImage->SetOutputScalarTypeToUnsignedChar();
                
    vtkImageReslice *reslice = vtkImageReslice::New();
    reslice->SetInput(scaledImage->GetOutput());
    reslice->SetInformationInput(m_baseForResliceImage);
    reslice->SetInterpolationModeToCubic();
    if (data->GetObjectType() & PREOP_TYPE)
        reslice->SetResliceTransform(identityTransform);
    else
        reslice->SetResliceTransform(concatTransform);
    reslice->Update();
    reslice->GetOutput()->GetScalarRange( scalarRange );
    
    reslicedImage->DeepCopy(reslice->GetOutput());
    scaledImage->Delete();
    reslice->Delete();
    identityTransform->Delete();
    concatTransform->Delete();
}

vtkImageData *Reslicer::MakeBaseForReslice(ImageObject *data)
{
    if (!data)
        return 0;
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    igstkLogMacro( DEBUG, "MakeBaseForReslice: " << data->GetName() << endl);
#endif    
    if (m_baseForResliceObject)
    {
        m_baseForResliceObject->UnRegister(0);
        m_baseForResliceImage->UnRegister(0);
    }

    m_baseForResliceObject = data;
    m_baseForResliceObject->Register(0);
    vtkImageData *image = data->GetImage();
    double level[2];
    data->GetUsedWindowLevel(level);
    
    vtkReal scalarRange[2];
    data->GetImageScalarRange(scalarRange );
    double scale = ((scalarRange[1]-scalarRange[0])/scalarRange[1])*(VTK_UNSIGNED_CHAR_MAX/scalarRange[1]);
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    igstkLogMacro( DEBUG, "base for reslice scalar range = " << scalarRange[0] << ", " << scalarRange[1] << endl);
    igstkLogMacro( DEBUG, "base for reslice scale = " << scale << endl);
    igstkLogMacro( DEBUG, "base scalar type = " << image->GetScalarTypeAsString() << endl);
#endif
    vtkLookupTable *lut = vtkLookupTable::New();
    lut->DeepCopy(data->GetLut());
    lut->SetTableRange(0, VTK_UNSIGNED_CHAR_MAX);
    lut->Build();

    vtkImageShiftScale *scaledImage = vtkImageShiftScale::New();
    scaledImage->SetInput(image);
    scaledImage->SetScale(scale);
    scaledImage->SetOutputScalarTypeToUnsignedChar();
    
    vtkImageMapToColors *baseImage = vtkImageMapToColors::New();
    baseImage->SetInput(scaledImage->GetOutput());
    baseImage->SetLookupTable(lut);
    baseImage->SetOutputFormatToRGBA();

    m_baseForResliceImage = baseImage->GetOutput();
    m_baseForResliceImage->Register(0);
    
    lut->Delete();
    scaledImage->Delete();
    baseImage->Delete();
    
    return  m_baseForResliceImage;
}
    
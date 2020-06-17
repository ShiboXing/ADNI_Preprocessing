#include "vtkReal.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"
#ifdef USE_MNI_MINC_READER
#else
#include "vtkImageShiftScale.h"
#endif
#include "MINCReader.h"


MINCReader * MINCReader::New()
{
    return new MINCReader();
}


MINCReader::MINCReader()
{
#ifdef USE_MNI_MINC_READER
    m_dataType = VTK_UNSIGNED_SHORT;
    m_icv = vtkMINCIcv::New();
    m_icv->SetDataType(m_dataType);
    m_icv->SetXDimDir(MI_ICV_POSITIVE);
    m_icv->SetYDimDir(MI_ICV_POSITIVE);
    m_icv->SetZDimDir(MI_ICV_POSITIVE);
    
    m_icv->DoRangeOn();
    m_icv->DoNormOn();
    
    m_reader = vtkMINCReader::New();
    m_reader->SetIcv(m_icv);
#else
    m_reader = vtkMINCImageReader2::New();
#endif    
}


MINCReader::~MINCReader()
{
#ifdef USE_MNI_MINC_READER
    m_icv->Delete();
#endif
    m_reader->Delete(); 
}

void MINCReader::SetFileName(const char * name)
{
    m_reader->SetFileName(name);
}

int MINCReader::CanReadFile(const char * name)
{
    return m_reader->CanReadFile(name);
}

void MINCReader::Update()
{
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
#ifdef USE_MNI_MINC_READER
    m_reader->SetLogger(this->GetLogger());
#endif
#endif
    m_reader->Update();
}

void MINCReader::SetDataType(int type)
{
    m_dataType = type;
#ifdef USE_MNI_MINC_READER
    m_icv->SetDataType(m_dataType);
#endif
}

vtkImageData *MINCReader::GetOutput()
{
#ifdef USE_MNI_MINC_READER
    return m_reader->GetOutput();
#else    
    vtkMINCImageAttributes2 *attributes = m_reader->GetImageAttributes();
    static vtkImageData *inputImage = m_reader->GetOutput();
    int dataType = attributes->GetDataType();
    if (dataType != m_dataType) // shifting may not work well
    {
        vtkImageShiftScale  * shiftedImage = vtkImageShiftScale::New();
        shiftedImage->SetInput(inputImage);
        double scale = this->ComputeScale(dataType);
        shiftedImage->SetShift(0.0);
        shiftedImage->SetScale(scale);
        shiftedImage->SetOutputScalarTypeToUnsignedShort();
        shiftedImage->Update();
        inputImage->DeepCopy(shiftedImage->GetOutput());
        shiftedImage->Delete();
    }
    return inputImage;
#endif        
}


vtkMatrix4x4* MINCReader::GetDirectionCosines()
{
    return m_reader->GetDirectionCosines();
}
    
vtkMINCImageAttributes2 *MINCReader::GetImageAttributes()
{
    return m_reader->GetImageAttributes();
}

int MINCReader::CreatedWithVtkMINCWriter()
{
    vtkMINCImageAttributes2 *attributes = m_reader->GetImageAttributes();
    const char *history = 0;
    if (attributes)
    {
        history = attributes->GetAttributeValueAsString(MI_EMPTY_STRING, MIhistory);
        if (history && strstr(history, "vtkMINCImageWriter"))
            return 0;
    }
    return 1;
}

vtkMatrix4x4* MINCReader::GetTransformationMatrix()
{
    return m_reader->GetTransform()->GetMatrix();
}

unsigned long MINCReader::GetErrorCode()
{
#ifdef USE_MNI_MINC_READER
    return (unsigned long)(m_reader->GetReaderStatus());
#else    
    return m_reader->GetErrorCode();
#endif        
}

double MINCReader::ComputeScale(int dataType)
{
    const int arbitrary_max_range = 4080.0; //16 * 255
    double scale = 1.0;
    switch( dataType )
        {
    case VTK_CHAR:
        scale = arbitrary_max_range / VTK_CHAR_MAX;
        break;
    case VTK_UNSIGNED_CHAR:
        scale = arbitrary_max_range / VTK_UNSIGNED_CHAR_MAX;
        break;
    case VTK_SHORT:
        scale = arbitrary_max_range / VTK_SHORT_MAX;
        break;
    case VTK_INT:
        scale = arbitrary_max_range / VTK_INT_MAX;
        break;
    case VTK_UNSIGNED_INT:
        scale = arbitrary_max_range / VTK_UNSIGNED_INT_MAX;
        break;
    case VTK_LONG:
        scale = arbitrary_max_range / VTK_LONG_MAX;
        break;
    case VTK_UNSIGNED_LONG:
        scale = arbitrary_max_range / VTK_UNSIGNED_LONG_MAX;
        break;
    case VTK_FLOAT:
        scale = arbitrary_max_range / VTK_FLOAT_MAX;
        break;
    case VTK_DOUBLE:
        scale = arbitrary_max_range / VTK_DOUBLE_MAX;
        break;
    default:
    case VTK_UNSIGNED_SHORT:
        scale = 1.0;
        break;
    }
    return scale;
}

void MINCReader::SetUseTransformForStartAndStep(unsigned int n)
{
#ifdef USE_MNI_MINC_READER
    m_reader->SetUseTransformForStartAndStep(n);
#endif
}



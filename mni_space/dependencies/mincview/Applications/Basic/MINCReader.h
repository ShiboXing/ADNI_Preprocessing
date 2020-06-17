#ifndef __READABLE_MINC_h_
#define __READABLE_MINC_h_


/* temporary in order to avoid setting too many compiler flags while other MINC readers do not work properly */
#ifndef USE_MNI_MINC_READER
#define USE_MNI_MINC_READER
  #endif
/*end temporary change */
 
#ifdef USE_MNI_MINC_READER
#include "vtkMINCIcv.h"
#include "vtkMINCReader.h"
#else
#include "vtkMINCImageReader2.h"
#endif
#include "vtkMINCImageAttributes2.h"
#include "vtkMINC.h"
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
// IGSTK headers
#include "igstkObject.h"
#include "igstkMacros.h"
#endif

class vtkMatrix4x4;
class vtkImageData;

class MINCReader : public vtkObject
{

public:

    static MINCReader* New();

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()
#endif    
    int  CanReadFile(const char *);
    void SetFileName(const char *);
    
    void Update();

    void SetDataType(int);
    int GetDataType() {return m_dataType;}

    unsigned long GetErrorCode();
    
    vtkImageData *GetOutput();
    vtkMatrix4x4* GetDirectionCosines();
    vtkMatrix4x4* GetTransformationMatrix();
    
    void SetUseTransformForStartAndStep(unsigned int);
    vtkMINCImageAttributes2 *GetImageAttributes();
    int CreatedWithVtkMINCWriter();
    
protected:

    MINCReader();
    ~MINCReader();

    int m_dataType;
#ifdef USE_MNI_MINC_READER
    vtkMINCIcv *m_icv;
    vtkMINCReader *m_reader;
#else
    vtkMINCImageReader2 *m_reader;
#endif

    double ComputeScale(int dataType);
    
private:

    MINCReader(const MINCReader&);  // Not implemented.
    void operator=(const MINCReader&);  // Not implemented.
};

#endif //__READABLE_MINC_h_




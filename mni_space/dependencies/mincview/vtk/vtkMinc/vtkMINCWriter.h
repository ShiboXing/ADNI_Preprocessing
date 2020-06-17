#ifndef VTKMINCWRITER_H
#define VTKMINCWRITER_H

#include <stdio.h>

#include "vtkReal.h"
#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkMINC2.h"

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
// IGSTK headers
#include "igstkObject.h"
#include "igstkMacros.h"
#endif

class vtkMINCWriter : public vtkObject
{

public:

    //BTX
    char const * GetClassName() const;
    //ETX
    static vtkMINCWriter * New() { return new vtkMINCWriter; }

    
   vtkMINCWriter();
    ~vtkMINCWriter();
    
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()
#endif    
    void Write();

    vtkSetObjectMacro(Input,vtkImageData);
    vtkSetObjectMacro(Transform,vtkMatrix4x4);
    vtkSetStringMacro(Filename);
    
    vtkSetMacro(UseTransformForStartAndStep, unsigned int);
    vtkGetMacro(UseTransformForStartAndStep, unsigned int);

     void SetAdditionalMINCAttributes(STUDY_ATTRIBUTES*, ACQUISITION_ATTRIBUTES*, PATIENT_ATTRIBUTES*, const char*); 

static    void vtkMatrixToStartStepAndDirCosine( vtkMatrix4x4 * mat, double start[3], double step[3], double dirCosine[3][3] );

protected:
    STUDY_ATTRIBUTES Study;
    ACQUISITION_ATTRIBUTES Acquisition;
    PATIENT_ATTRIBUTES Patient;
    char *Comment;
private:

    // Description:
    // if 1, the input transform will be used to compute start
    // and step of the file. If 0, the start and step will be
    // taken from the input image. 
    unsigned int UseTransformForStartAndStep;
    
    vtkImageData * Input;
    vtkMatrix4x4 * Transform;
    char         * Filename;
    void WriteAdditionalMINCAttributes(int mincFileId);
};


#endif

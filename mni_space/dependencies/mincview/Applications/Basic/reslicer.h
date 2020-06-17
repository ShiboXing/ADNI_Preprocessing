/* 
    This class will create a volume in the 3D view
*/
#ifndef TAG_RESLICER_H_
#define TAG_RESLICER_H_

#include "vtkReal.h"
#include "vtkObject.h"

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
// IGSTK headers
#include "igstkObject.h"
#include "igstkMacros.h"
#endif
class vtkImageData;
class ImageObject;

class Reslicer : public vtkObject
{
public:
    
    static Reslicer *New() { return new Reslicer; }
    void Delete() { delete this; }
    
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()
#endif    
    void ResliceImageData(ImageObject *data, vtkImageData *reslicedImage);
    vtkImageData *MakeBaseForReslice(ImageObject *data);
    
protected:
    ImageObject *m_baseForResliceObject;
    vtkImageData *m_baseForResliceImage;
private:
    Reslicer();
    ~Reslicer();
};
    
#endif //TAG_RESLICER_H_

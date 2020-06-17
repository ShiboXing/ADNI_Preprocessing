/* 
    This class will create a volume in the 3D view
*/
#ifndef TAG_VOLUME_H_
#define TAG_VOLUME_H_

#include "vtkObject.h"
#include "vtkReal.h"

class ImageObject;
class vtkLODProp3D;

class Volume : public vtkObject
{
public:
    
    static Volume *New() { return new Volume; }
    void Delete() { delete this; }
    
    Volume();
    ~Volume();
    
    void DisplayVolume(ImageObject *data);
    vtkLODProp3D *GetVolumeProp3D() {return lodProp3D;}
     
protected:
    ImageObject *object;
    vtkLODProp3D *lodProp3D;
};
    
#endif //TAG_VOLUME_H_

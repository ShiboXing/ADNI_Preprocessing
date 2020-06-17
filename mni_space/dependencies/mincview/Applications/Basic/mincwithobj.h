#ifndef TAG_MINCWITHOBJ_h_
#define TAG_MINCWITHOBJ_h_

#include "vtkObject.h"

class ImageObject;
class PolyDataObject;
class SceneObject;

class MincWithObj : public vtkObject
{
public:
        
    static MincWithObj * New() { return new MincWithObj; }
    
    MincWithObj();
    ~MincWithObj();

    vtkGetObjectMacro( ImageObj, ImageObject );
    void SetImageObj( ImageObject *obj);
    vtkGetObjectMacro( PolyDataObj, PolyDataObject );
    void SetPolyDataObj( PolyDataObject *obj);
    
    SceneObject * GetPolyDataObjAsSceneObject();
    SceneObject * GetImageObjAsSceneObject();
    
protected:
    ImageObject *ImageObj;
    PolyDataObject *PolyDataObj;
};

#endif //TAG_MINCWITHOBJ_h_

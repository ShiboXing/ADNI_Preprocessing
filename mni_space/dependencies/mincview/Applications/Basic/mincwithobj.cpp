#include "mincwithobj.h"
#include "imageobject.h"
#include "polydataobject.h"

vtkCxxSetObjectMacro(MincWithObj, ImageObj, ImageObject);
vtkCxxSetObjectMacro(MincWithObj, PolyDataObj, PolyDataObject);

MincWithObj::MincWithObj()
{
    ImageObj = 0;
    PolyDataObj = 0;
}

MincWithObj::~MincWithObj()
{
    if (ImageObj)
        ImageObj->UnRegister(this);
    if (PolyDataObj)
        PolyDataObj->UnRegister(this);
}

SceneObject * MincWithObj::GetPolyDataObjAsSceneObject()
{
    return (SceneObject *)PolyDataObj;
}

SceneObject * MincWithObj::GetImageObjAsSceneObject()
{
    return (SceneObject *)ImageObj;
}


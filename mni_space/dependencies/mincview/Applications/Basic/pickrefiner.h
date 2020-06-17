#ifndef TAG_PICKREFINER_H_
#define TAG_PICKREFINER_H_

#include "vtkObject.h"
#include "vtkReal.h"

class vtkCallbackCommand;
class vtkMultiImagePlaneWidget;
class ImageObject;
class SceneManager;
class PointsObject;

class PickRefiner : public vtkObject
{

public:

    static PickRefiner * New() { return new PickRefiner; }

    PickRefiner();
    virtual ~PickRefiner();

    void SetImageObject(ImageObject *obj);
    void SetPlanesPosition(int idx, vtkReal posX, vtkReal posY, vtkReal posZ);
    void EnablePickerOnPlanes();
    void DisablePickerOnPlanes();
    void ShowCrosshair();
    void HideCrosshair();
    void PickPoint();
    void OverrideMouseActions();
    void RestoreMouseActions();
    void SetPoints(PointsObject *pts);

protected:

    ImageObject *imageObj;
    SceneManager *manager;
    vtkMultiImagePlaneWidget *viewPlanes[3];
    vtkCallbackCommand * pickerCommandObject;
    int currentPointIndex;
    bool interactionEnabled;
    PointsObject *points;
};

#endif //TAG_PICKREFINER_H_

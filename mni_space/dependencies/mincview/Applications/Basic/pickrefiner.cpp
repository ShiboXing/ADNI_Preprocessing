#include "pickrefiner.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderer.h"
#include "vtkMultiImagePlaneWidget.h"
#include "vtkImageData.h"
#include "vtkCoordinate.h"
#include "vtkViewport.h"
#include "view.h"

#include "imageobject.h"
#include "scenemanager.h"
#include "vtkIgnsEvents.h"
#include "pointsobject.h"

void PickRefinerInteractionCallback( vtkObject* vtkNotUsed(object), 
                                       unsigned long event, 
                                       void* clientdata, 
                                       void* vtkNotUsed(calldata) )
{
    PickRefiner * self = (PickRefiner*)clientdata;
    switch( event )
    {
    case vtkCommand::UserEvent+IGNS_MOVE_2D_POINT:
        self->PickPoint( );
        break;
    }
}

PickRefiner::PickRefiner()
{
    imageObj = 0;
    manager = 0;

    this->pickerCommandObject = vtkCallbackCommand::New();
    this->pickerCommandObject->SetClientData((void*)this); 
    this->pickerCommandObject->SetCallback(PickRefinerInteractionCallback);
    this->pickerCommandObject->SetAbortFlagOnExecute(1);
    this->currentPointIndex = -1;
    this->interactionEnabled = false;
    this->points = 0;
}

PickRefiner::~PickRefiner()
{
    if( this->imageObj )
    {
        this->imageObj->UnRegister( this );
    }
    if (this->manager != NULL) 
    { 
        this->manager->UnRegister(this);
    }  
    if (this->points != NULL)
    { 
        this->points->UnRegister(this);
    }  
    this->pickerCommandObject->Delete(); 
}

void PickRefiner::SetImageObject( ImageObject * obj )
{
    if( this->imageObj == obj )
    {
        return;
    }
    
    if( this->imageObj )
    {
        this->imageObj->UnRegister( this );
    }
    this->imageObj = obj;
    if( this->imageObj )
    {
        this->imageObj->Register( this );
    }
    for (int i = 0; i < 3; i++)
    {
        this->viewPlanes[i] = this->imageObj->GetPlane(i);
    }  
    if (this->manager != NULL) 
    { 
        this->manager->UnRegister(this); 
    }

    this->manager = this->imageObj->GetManager();                                          
    if (this->manager != NULL) 
    { 
        this->manager->Register(this); 
    }     
}
   
void PickRefiner::SetPoints(PointsObject *pts)
{
    if (this->points == pts)
        return;
    if (this->points)
        this->points->UnRegister(0);
    this->points = pts;
    if (this->points)
        this->points->Register(0);
}

void PickRefiner::SetPlanesPosition(int idx, vtkReal posX, vtkReal posY, vtkReal posZ)
{
    if (!this->interactionEnabled)
        this->OverrideMouseActions();
    this->viewPlanes[0]->SetSlicePosition(posX);
    this->viewPlanes[1]->SetSlicePosition(posY);
    this->viewPlanes[2]->SetSlicePosition(posZ);
    for (int j = 0; j < 3; j++)
    {
        View * v = this->manager->GetView(j);
        vtkCoordinate* coord = vtkCoordinate::New();
        coord->SetCoordinateSystemToWorld();
        coord->SetValue(posX, posY, posZ);
        int *dispPos = coord->GetComputedDisplayValue(v->GetRenderer());
        this->viewPlanes[j]->ExternalUpdateCursor(dispPos, v->GetRenderer());
        this->manager->GetView(j)->GetRenderer()->ResetCameraClippingRange();
        this->manager->GetView(j)->NotifyNeedRender();
        this->viewPlanes[j]->ExternalUpdateCursor(dispPos, v->GetRenderer());//TODO this is a patch which forces showing cursor
    }
    this->currentPointIndex = idx;
}
void PickRefiner::EnablePickerOnPlanes()
{
    this->OverrideMouseActions();
}

void PickRefiner::OverrideMouseActions()
{
    for (int i = 0; i < 3; i++)
    {
        this->viewPlanes[i]->AddObserver( vtkCommand::UserEvent+IGNS_MOVE_2D_POINT, this->pickerCommandObject, 0.5);
        this->viewPlanes[i]->SetLeftButtonAction( vtkMultiImagePlaneWidget::CURSOR_ACTION ); //picking on left button
        this->viewPlanes[i]->SetRightButtonAction( vtkMultiImagePlaneWidget::WINDOW_LEVEL_ACTION); // only resizing on right button
    }
    this->interactionEnabled = true;
}

void PickRefiner::RestoreMouseActions()
{ //back to default set in imageobject.cpp CreatePlane()
    for (int i = 0; i < 3; i++)
    {
        this->viewPlanes[i]->RemoveObserver( this->pickerCommandObject );
        this->viewPlanes[i]->SetLeftButtonAction( vtkMultiImagePlaneWidget::SLICE_MOTION_ACTION );
        this->viewPlanes[i]->SetRightButtonAction( vtkMultiImagePlaneWidget::CURSOR_ACTION );
    }
    this->interactionEnabled = false;
}

void PickRefiner::DisablePickerOnPlanes()
{
    this->points->ActivateCursorOnAllPlanes(0);
    this->currentPointIndex = -1;

}

void PickRefiner::HideCrosshair()

{
    this->points->ActivateCursorOnAllPlanes(0);
    this->RestoreMouseActions();
    emit Modified();
}

void PickRefiner::ShowCrosshair()
{
    this->points->ActivateCursorOnAllPlanes(1);
    this->OverrideMouseActions();
    emit Modified();
}

void PickRefiner::PickPoint()
{
    if (currentPointIndex < 0)
        return;
    int windowNum = this->manager->GetCurrentView( );
    int currentPlaneIndex = 0;
    vtkReal worldCoords[3];
    switch (windowNum)
    {
    case 0: //TRANSVERSE_VIEW_TYPE, z fix
        currentPlaneIndex = 2;
        break;
    case 2: //CORONAL_VIEW_TYPE, y fix
         currentPlaneIndex = 1;
        break;
    case 3: //SAGITTAL_VIEW_TYPE, x fix
        currentPlaneIndex = 0;
        break;
    case 1: // THREED_VIEW_TYPE
    default:
        return;
    }
    this->viewPlanes[currentPlaneIndex]->GetCurrentWorldPosition(worldCoords);
    this->SetPlanesPosition(currentPointIndex, worldCoords[0], worldCoords[1], worldCoords[2]);
    if (this->points)
        this->points->SetPointCoordinatesAndUpdate(currentPointIndex, worldCoords);
    if(this->manager->GetPickObj())
    {
        this->manager->GetPickObj()->UpdatePointList();
        this->manager->GetPickObj()->DisplayPoints();
        this->manager->GetPickObj()->ShowPointCoordinatesIn3DView(currentPointIndex+1, worldCoords[0], worldCoords[1], worldCoords[2]);
    }
}

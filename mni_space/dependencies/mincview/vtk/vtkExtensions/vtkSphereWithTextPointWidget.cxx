/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSphereWithTextPointWidget.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSphereWithTextPointWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkMath.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include "vtkTransform.h"
#include "vtkFollower.h"
#include "vtkIgnsEvents.h"

vtkCxxRevisionMacro(vtkSphereWithTextPointWidget, "$Revision: 1.9 $");
vtkStandardNewMacro(vtkSphereWithTextPointWidget);

vtkSphereWithTextPointWidget::vtkSphereWithTextPointWidget()
{
    this->State = vtkSphereWithTextPointWidget::Start;
    this->EventCallbackCommand->SetCallback(vtkSphereWithTextPointWidget::ProcessEvents);
  
    // Represent the point
    this->Sphere = vtkSphereSource::New();
    Sphere->SetRadius(0.25);
    this->Mapper = vtkPolyDataMapper::New();
    this->Mapper->SetInput(this->Sphere->GetOutput());
    this->Actor = vtkActor::New();
    this->Actor->SetMapper(this->Mapper);
    this->Actor->PickableOn();
    
    // Define the point coordinates
    double bounds[6];
    bounds[0] = -0.5;
    bounds[1] = 0.5;
    bounds[2] = -0.5;
    bounds[3] = 0.5;
    bounds[4] = -0.5;
    bounds[5] = 0.5;

    // Initial creation of the widget, serves to initialize it
    this->PlaceWidget(bounds);

    //Manage the picking stuff
    this->CursorPicker = vtkCellPicker::New();
    this->CursorPicker->PickFromListOn();
    this->CursorPicker->AddPickList(this->Actor);
    this->CursorPicker->SetTolerance(0.005); //need some fluff

    // Set up the initial properties
    this->CreateDefaultProperties();

    // Constraints not set
    this->ConstraintAxis = -1;

    // Override superclass'
    this->PlaceFactor = 1.0;
  
    // The size of the hot spot
    this->HotSpotSize = 0.05;
    this->WaitingForMotion = 0;
  
    this->Label = vtkVectorText::New();
    this->Label->SetText(" ");
    this->LabelActor = vtkFollower::New();
    this->LabelMapper = vtkPolyDataMapper::New();
    this->LabelMapper->SetInput(this->Label->GetOutput());
    this->LabelActor->SetMapper(this->LabelMapper);
    this->LabelActor->SetScale( INITIAL_TAG_LABEL_SCALE, INITIAL_TAG_LABEL_SCALE, INITIAL_TAG_LABEL_SCALE );
    this->LabelActor->VisibilityOff();
    this->KeyPressActivationOff();
    this->Transform = 0;
    this->HighlightedFromOutside = 0;
    this->PointIndex = 0;
    this->Priority = 1.0;
}

vtkSphereWithTextPointWidget::~vtkSphereWithTextPointWidget()
{
    this->Actor->Delete();
    this->Mapper->Delete();
    this->Sphere->Delete();
  
    this->CursorPicker->Delete();
  
    this->Property->Delete();
    this->SelectedProperty->Delete();
    this->DisabledProperty->Delete();
  
    this->Label->Delete();
    this->LabelActor->Delete();
    this->LabelMapper->Delete();
    if( this->Transform )
        this->Transform->UnRegister( this );
}

void vtkSphereWithTextPointWidget::Update()
{
    this->Sphere->Update();
    this->UpdateLabelPosition();
}

void vtkSphereWithTextPointWidget::SetEnabled(int enabling)
{
    if ( ! this->Interactor )
    {
        vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
        return;
    }

    if ( enabling ) //-----------------------------------------------------------
    {
        vtkDebugMacro(<<"Enabling point widget");

        if ( this->Enabled ) //already enabled, just return
        {
            return;
        }
    
        if ( ! this->CurrentRenderer )
        {
            this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
            this->Interactor->GetLastEventPosition()[0],
            this->Interactor->GetLastEventPosition()[1]));
            if (this->CurrentRenderer == NULL)
            {
            return;
            }
        }

        this->Enabled = 1;

        // listen for the following events
        vtkRenderWindowInteractor *i = this->Interactor;
        i->AddObserver(vtkCommand::MouseMoveEvent, this->EventCallbackCommand, this->Priority);
        i->AddObserver(vtkCommand::LeftButtonPressEvent, this->EventCallbackCommand, this->Priority);
        i->AddObserver(vtkCommand::LeftButtonReleaseEvent, this->EventCallbackCommand, this->Priority);

        // Add the point
        this->LabelActor->SetCamera( this->CurrentRenderer->GetActiveCamera() );
        this->CurrentRenderer->AddActor(this->Actor);
        this->CurrentRenderer->AddActor(this->LabelActor);
        this->ShowLabel(true);
        this->Highlight(this->HighlightedFromOutside);
        this->Sphere->Update();

        this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    }
  
    else //disabling----------------------------------------------------------
    {
        vtkDebugMacro(<<"Disabling point widget");

        if ( ! this->Enabled ) //already disabled, just return
        {
            return;
        }
    
        this->Enabled = 0;

        // don't listen for events any more
        this->Interactor->RemoveObserver(this->EventCallbackCommand);

        // turn off the point
        this->CurrentRenderer->RemoveActor(this->Actor);
        this->CurrentRenderer->RemoveActor(this->LabelActor);

        this->InvokeEvent(vtkCommand::DisableEvent,NULL);
        this->SetCurrentRenderer(NULL);
    }

    this->Interactor->Render();
}

void vtkSphereWithTextPointWidget::SetPickable(int yes)
{
    if (yes)
    {
        this->Actor->PickableOn();
        this->Actor->SetProperty(this->Property);
        this->LabelActor->SetProperty(this->Property);
    }
    else
    {
        this->Actor->PickableOff();
        this->Actor->SetProperty(this->DisabledProperty);
        this->LabelActor->SetProperty(this->DisabledProperty);
    }
}

int vtkSphereWithTextPointWidget::GetPickable()
{
    return this->Actor->GetPickable();
}

void vtkSphereWithTextPointWidget::ProcessEvents(vtkObject* vtkNotUsed(object),
                                   unsigned long event,
                                   void* clientdata, 
                                   void* vtkNotUsed(calldata))
{
    vtkSphereWithTextPointWidget* self = reinterpret_cast<vtkSphereWithTextPointWidget *>( clientdata );

    switch(event)
    {
    case vtkCommand::LeftButtonPressEvent:
        self->OnLeftButtonDown();
        break;
    case vtkCommand::LeftButtonReleaseEvent:
        self->OnLeftButtonUp();
        break;
    case vtkCommand::MouseMoveEvent:
        self->OnMouseMove();
        break;
    }
}

void vtkSphereWithTextPointWidget::PrintSelf(ostream& os, vtkIndent indent)
{
    os << indent << "vtkSphereWithTextPointWidget begin: " << "\n";
    os << indent << "Point index: " << this->PointIndex << "\n";
    os << indent << "Point label: " << this->GetLabel() << "\n";
    os << indent << "InitialBounds: " <<  InitialBounds[0] << ", " << InitialBounds[1] << "\n";
    os << indent << "InitialBounds: " <<  InitialBounds[2] << ", " << InitialBounds[3] << "\n";
    os << indent << "InitialBounds: " <<  InitialBounds[4] << ", " << InitialBounds[5] << "\n";

    if ( this->Property )
    {
        os << indent << "Property: " << this->Property << "\n";
    }
    else
    {
        os << indent << "Property: (none)\n";
    }
    if ( this->SelectedProperty )
    {
        os << indent << "Selected Property: " << this->SelectedProperty << "\n";
    }
    else
    {
        os << indent << "Selected Property: (none)\n";
    }
    if ( this->DisabledProperty )
    {
        os << indent << "Disabled Property: " << this->DisabledProperty << "\n";
    }
    else
    {
        os << indent << "DisabledProperty: (none)\n";
    }

    if ( this->Sphere )
    {
        double pos[3];
        this->Sphere->GetCenter(pos);
        os << indent << "Point Position: " <<  pos[0] << ", " << pos[1] << ", " << pos[2] << "\n";
        os << indent << "Point Size: " << this->Sphere->GetRadius() << "\n";
    }
    else
    {
        os << indent << "Point: (none)\n";
    }
    os << indent << "Hot Spot Size: " << this->HotSpotSize << "\n";
    os << indent << "vtkSphereWithTextPointWidget Superclass: " << "\n";
    this->Superclass::PrintSelf(os,indent);
    os << indent << "vtkSphereWithTextPointWidget end: " << "\n";
}

void vtkSphereWithTextPointWidget::Highlight(int highlight)
{
    if ( highlight )
    {
        this->Actor->SetProperty(this->SelectedProperty);
        this->LabelActor->SetProperty(this->SelectedProperty);
        this->CursorPicker->GetPickPosition(this->LastPickPosition);
        this->ValidPick = 1;
    }
    else
    {
        if (this->GetPickable())
        {
            this->LabelActor->SetProperty(this->Property);
            this->Actor->SetProperty(this->Property);
        }
        else
        {
            this->LabelActor->SetProperty(this->DisabledProperty);
            this->Actor->SetProperty(this->DisabledProperty);
        }
    }
}

void vtkSphereWithTextPointWidget::ExternalHighlight(int highlight)
{
    if ( highlight )
    {
        this->HighlightedFromOutside = 1;
        this->Actor->SetProperty(this->SelectedProperty);
        this->LabelActor->SetProperty(this->SelectedProperty);
    }
    else
    {
        this->HighlightedFromOutside = 0;
        if (this->GetPickable())
        {
            this->LabelActor->SetProperty(this->Property);
            this->Actor->SetProperty(this->Property);
        }
        else
        {
            this->LabelActor->SetProperty(this->DisabledProperty);
            this->Actor->SetProperty(this->DisabledProperty);
        }
    }
}

void vtkSphereWithTextPointWidget::SetOpacity(double op)
{
    this->Property->SetOpacity(op);
    this->SelectedProperty->SetOpacity(op);
    this->DisabledProperty->SetOpacity(op);
}

void vtkSphereWithTextPointWidget::SetPropertyColor(double color[3])
{
    Property->SetColor(color[0], color[1], color[2]);
}

void vtkSphereWithTextPointWidget::SetSelectedPropertyColor(double color[3])
{
    SelectedProperty->SetColor(color[0], color[1], color[2]);
}

void vtkSphereWithTextPointWidget::SetDisabledPropertyColor(double color[3])
{
    DisabledProperty->SetColor(color[0], color[1], color[2]);
}

int vtkSphereWithTextPointWidget::DetermineConstraintAxis(int constraint, double *x)
{
    // Look for trivial cases
    if ( ! this->Interactor->GetShiftKey() )
    {
        return -1;
    }
    else if ( constraint >= 0 && constraint < 3 )
    {
        return constraint;
    }
  
    // Okay, figure out constraint. First see if the choice is
    // outside the hot spot
    if ( ! this->WaitingForMotion )
    {
        double p[3], d2, tol;
        this->CursorPicker->GetPickPosition(p);
        d2 = vtkMath::Distance2BetweenPoints(p,this->LastPickPosition);
        tol = this->HotSpotSize*this->InitialLength;
        if ( d2 > (tol*tol) )
        {
            this->WaitingForMotion = 0;
            return this->CursorPicker->GetCellId();
        }
        else
        {
            this->WaitingForMotion = 1;
            this->WaitCount = 0;
            return -1;
        }
    }
    else if ( this->WaitingForMotion && x ) 
    {
        double v[3];
        this->WaitingForMotion = 0;
        v[0] = fabs(x[0] - this->LastPickPosition[0]);
        v[1] = fabs(x[1] - this->LastPickPosition[1]);
        v[2] = fabs(x[2] - this->LastPickPosition[2]);
        return ( v[0]>v[1] ? (v[0]>v[2]?0:2) : (v[1]>v[2]?1:2));
    }
    else
    {
        return -1;
    }
}

void vtkSphereWithTextPointWidget::OnLeftButtonDown()
{
    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];

    // Okay, make sure that the pick is in the current renderer
    if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
        this->State = vtkSphereWithTextPointWidget::Outside;
        return;
    }
  
    vtkAssemblyPath *path;
    this->CursorPicker->Pick(X,Y,0.0,this->CurrentRenderer);
    path = this->CursorPicker->GetPath();
    if ( path != NULL )
    {
        this->State = vtkSphereWithTextPointWidget::Moving;
        this->Highlight(1);
        this->ConstraintAxis = this->DetermineConstraintAxis(-1,NULL);
    }
    else
    {
        this->State = vtkSphereWithTextPointWidget::Outside;
        this->Highlight(this->HighlightedFromOutside);
        this->ConstraintAxis = -1;
        return;
    }
  
    this->EventCallbackCommand->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    this->InvokeEvent(vtkCommand::UserEvent+IGNS_HIGHLIGHT_POINT ,NULL);
    this->Interactor->Render();
}

void vtkSphereWithTextPointWidget::OnLeftButtonUp()
{
    if ( this->State == vtkSphereWithTextPointWidget::Outside ||
         this->State == vtkSphereWithTextPointWidget::Start )
    {
        return;
    }

    this->State = vtkSphereWithTextPointWidget::Start;
    this->Highlight(this->HighlightedFromOutside);
    this->Update();

    this->EventCallbackCommand->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    this->InvokeEvent(vtkCommand::UserEvent+IGNS_MOVE_3D_POINT,NULL);
    this->Interactor->Render();
}

void vtkSphereWithTextPointWidget::OnMouseMove()
{
    // See whether we're active
    if ( this->State == vtkSphereWithTextPointWidget::Outside || 
         this->State == vtkSphereWithTextPointWidget::Start )
    {
        return;
    }
  
    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];

    // Do different things depending on state
    // Calculations everybody does
    double focalPoint[4], pickPoint[4], prevPickPoint[4];
    double z;

    vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
    if ( !camera )
    {
        return;
    }

    // Compute the two points defining the motion vector
    this->ComputeWorldToDisplay(this->LastPickPosition[0], this->LastPickPosition[1],
                                this->LastPickPosition[2], focalPoint);
    z = focalPoint[2];
    this->ComputeDisplayToWorld(double(this->Interactor->GetLastEventPosition()[0]),double(this->Interactor->GetLastEventPosition()[1]),
                                z, prevPickPoint);
    this->ComputeDisplayToWorld(double(X), double(Y), z, pickPoint);

    // Process the motion
    if ( this->State == vtkSphereWithTextPointWidget::Moving )
    {
        if ( !this->WaitingForMotion || this->WaitCount++ > 3 )
        {
            this->ConstraintAxis = 
            this->DetermineConstraintAxis(this->ConstraintAxis,pickPoint);
            this->MoveFocus(prevPickPoint, pickPoint);
        }
        else
        {
            return; //avoid the extra render
        }
    }
    // Interact, if desired
    this->EventCallbackCommand->SetAbortFlag(1);
    this->InvokeEvent(vtkCommand::InteractionEvent,NULL);
    this->Interactor->Render();
}

void vtkSphereWithTextPointWidget::MoveFocus(double *p1, double *p2)
{
    //Get the motion vector
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];

    double center[3];
    this->Sphere->GetCenter(center);
    if ( this->ConstraintAxis >= 0 )
    {
        center[this->ConstraintAxis] += v[this->ConstraintAxis];
    }
    else
    {
        center[0] += v[0];
        center[1] += v[1];
        center[2] += v[2];
    }
  
    this->Sphere->SetCenter(center);
    this->UpdateLabelPosition();
}

void vtkSphereWithTextPointWidget::CreateDefaultProperties()
{
    this->Property = vtkProperty::New();
    this->Property->SetColor(0.0,0.0,1.0);

    this->SelectedProperty = vtkProperty::New();
    this->SelectedProperty->SetColor(0.0,1.0,0.0);
    
    this->DisabledProperty = vtkProperty::New();
    this->DisabledProperty->SetColor(0.75,0.5,0.0);
}

void vtkSphereWithTextPointWidget::PlaceWidget(double bds[6])
{
    int i;
    double bounds[6], center[3];

    this->AdjustBounds(bds, bounds, center);
  
    this->Sphere->SetCenter(center);
    this->Sphere->Update();

    for (i=0; i<6; i++)
    {
        this->InitialBounds[i] = bounds[i];
    }
    this->InitialLength = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                               (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                               (bounds[5]-bounds[4])*(bounds[5]-bounds[4]));

}

void vtkSphereWithTextPointWidget::SetLabelScale(double s)
{
    if (s <= 0) 
        s = 1.0;
    this->LabelActor->SetScale( s, s, s );
}

double *vtkSphereWithTextPointWidget::GetLabelScale()
{
    return this->LabelActor->GetScale();
}

void vtkSphereWithTextPointWidget::UpdateLabelPosition()
{
    double pos[3], radius;
    this->Sphere->GetCenter(pos);
    radius = this->Sphere->GetRadius();
    double newPos[3];
    for (int i = 0; i < 3; i++)
        newPos[i] = pos[i];
    if (this->Transform)
        this->Transform->TransformPoint(pos, newPos);
    this->LabelActor->SetPosition(newPos[0]+radius+2, newPos[1]+radius+2, newPos[2]+radius+2);
}

void vtkSphereWithTextPointWidget::SetLabel( const std::string & text )
{ 
    this->Label->SetText(text.c_str());
    this->LabelMapper->SetInput( this->Label->GetOutput() );
    this->LabelActor->SetProperty(this->Property);
    this->UpdateLabelPosition();
}

void vtkSphereWithTextPointWidget::ShowLabel( bool show)
{
    if (show)
    {
        this->LabelActor->VisibilityOn();
    }
    else
    {
        this->LabelActor->VisibilityOff();
    }
}

void vtkSphereWithTextPointWidget::EnableInteraction(int yes)
{
    if (yes)
    {
        this->Interactor->AddObserver(vtkCommand::MouseMoveEvent, this->EventCallbackCommand, this->Priority);
        this->Interactor->AddObserver(vtkCommand::LeftButtonPressEvent, this->EventCallbackCommand, this->Priority);
        this->Interactor->AddObserver(vtkCommand::LeftButtonReleaseEvent, this->EventCallbackCommand, this->Priority);
    }
    else
    {
        this->Interactor->RemoveObserver(this->EventCallbackCommand);
    }
}

void vtkSphereWithTextPointWidget::SetTransform(vtkTransform * trans)
{
    if( this->Transform == trans )
    {
        return;
    }
    if( this->Transform )
    {
        this->Transform->UnRegister( this );
    }
    this->Transform = trans;
    if( this->Transform )
    {
        this->Transform->Register( this );
        this->Actor->SetUserTransform(this->Transform);
    }
}

void vtkSphereWithTextPointWidget::SeNumericLabel(int n)
{
    char num[8];
    sprintf(num, "%i", n);
    this->SetLabel(num);
}
    
void vtkSphereWithTextPointWidget::DeepCopy(vtkSphereWithTextPointWidget *pt)
{
    if (pt == NULL)
    {
        return;
    }
    this->Property->DeepCopy( pt->GetProperty() );
    this->SelectedProperty->DeepCopy(pt->GetSelectedProperty());
    this->DisabledProperty->DeepCopy(pt->GetDisabledProperty());
    this->SetInput(pt->GetInput());
    this->SetInteractor(pt->GetInteractor());
    this->SetCurrentRenderer(pt->GetCurrentRenderer());
    this->Sphere->SetRadius(pt->GetRadius());
    this->Sphere->SetCenter(pt->GetPosition());
    this->SetLabel(pt->GetLabel());
    this->SetPointIndex(pt->GetPointIndex());
    this->SetHighlightedFromOutside(pt->GetHighlightedFromOutside());
    this->SetTransform(pt->GetTransform());
}

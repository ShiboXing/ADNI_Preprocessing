/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPiecewiseFunctionWidget.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPiecewiseFunctionWidget.h"
#include "vtkPiecewiseFunctionActor.h"
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"


vtkCxxRevisionMacro(vtkPiecewiseFunctionWidget, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkPiecewiseFunctionWidget);


vtkPiecewiseFunctionWidget::vtkPiecewiseFunctionWidget()
{
    this->State = vtkPiecewiseFunctionWidget::Outside;
    this->EventCallbackCommand->SetCallback(vtkPiecewiseFunctionWidget::ProcessEvents);
    this->Actor = vtkPiecewiseFunctionActor::New();
    this->Priority = 0.55;
}


vtkPiecewiseFunctionWidget::~vtkPiecewiseFunctionWidget()
{
    this->Actor->Delete();
}


void vtkPiecewiseFunctionWidget::SetEnabled(int enabling)
{
    if ( ! this->Interactor )
    {
        vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
        return;
    }

    //------ Enabling ------------------------------------------
    if ( enabling )
    {
        // already enabled, just return
        if ( this->Enabled )
        {
            return;
        }

        // Find the current renderer if none is set
        if ( ! this->CurrentRenderer )
        {
            int * lastPos = this->Interactor->GetLastEventPosition();
            vtkRenderer * ren = this->Interactor->FindPokedRenderer( lastPos[0], lastPos[1] );
            this->SetCurrentRenderer( ren );
            if (this->CurrentRenderer == NULL)
            {
                return;
            }
        }

        this->Enabled = 1;

        // listen for the following events
        vtkRenderWindowInteractor *i = this->Interactor;
        i->AddObserver(vtkCommand::MouseMoveEvent,
                    this->EventCallbackCommand, this->Priority);
        i->AddObserver(vtkCommand::LeftButtonPressEvent,
                    this->EventCallbackCommand, this->Priority);
        i->AddObserver(vtkCommand::LeftButtonReleaseEvent,
                    this->EventCallbackCommand, this->Priority);

        // Add the xy plot
        this->CurrentRenderer->AddViewProp(this->Actor);
        this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    }
    //------ Disabling ------------------------------------------
    else
    {
        vtkDebugMacro(<<"Disabling line widget");
        if ( ! this->Enabled ) //already disabled, just return
        {
            return;
        }
        this->Enabled = 0;

        // turn off the line
        this->CurrentRenderer->RemoveActor(this->Actor);
        this->InvokeEvent(vtkCommand::DisableEvent,NULL);
        this->SetCurrentRenderer(NULL);
    }

    this->Interactor->Render();
}

void vtkPiecewiseFunctionWidget::ProcessEvents( vtkObject* vtkNotUsed(object),
                                                    unsigned long event,
                                                    void* clientdata,
                                                    void* vtkNotUsed(calldata))
{
    vtkPiecewiseFunctionWidget* self = reinterpret_cast<vtkPiecewiseFunctionWidget *>( clientdata );
  
    //okay, let's do the right thing
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


void vtkPiecewiseFunctionWidget::OnLeftButtonDown()
{
    // We're only here is we are enabled
    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];

    // Okay, make sure that the pick is in the current renderer
    vtkRenderer *ren = this->Interactor->FindPokedRenderer(X,Y);
    if ( ren != this->CurrentRenderer )
    {
        this->State = vtkPiecewiseFunctionWidget::Outside;
        return;
    }
    
    if( !Actor->IsInGraph( X, Y ) )
    {
        this->State = vtkPiecewiseFunctionWidget::Outside;
        return;
    }

    // Okay, we can process this.

    int control = this->Interactor->GetControlKey();
    int shift = this->Interactor->GetShiftKey();
    
    if( control )
    {
        this->Actor->RemovePoint( X, Y, this->CurrentRenderer );
        this->EventCallbackCommand->SetAbortFlag(1);
        this->Interactor->Render();
    }
    else if( shift )
    {
        this->Actor->AddPoint( X, Y );
        this->EventCallbackCommand->SetAbortFlag(1);
        this->Interactor->Render();
    }
    else if( this->Actor->PickPoint( X, Y, this->CurrentRenderer ) )
    {
        this->State = vtkPiecewiseFunctionWidget::MovingPoint;
        this->EventCallbackCommand->SetAbortFlag(1);
        this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    }
}

void vtkPiecewiseFunctionWidget::OnMouseMove()
{
    // compute some info we need for all cases
    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];

    if( this->State == vtkPiecewiseFunctionWidget::MovingPoint )
    {
        this->Actor->MoveCurrentPoint( X, Y );
        this->EventCallbackCommand->SetAbortFlag(1);
        this->Interactor->Render();
    }
}


void vtkPiecewiseFunctionWidget::OnLeftButtonUp()
{
    if (this->State != vtkPiecewiseFunctionWidget::MovingPoint )
    {
        return;
    }

    // stop moving
    this->State = vtkPiecewiseFunctionWidget::Outside;
    this->EventCallbackCommand->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    this->Interactor->Render();
}


void vtkPiecewiseFunctionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
}

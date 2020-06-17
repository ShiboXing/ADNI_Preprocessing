/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkColorTransferFunctionWidget.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkColorTransferFunctionWidget.h"
#include "vtkColorTransferFunctionActor.h"
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCoordinate.h"

vtkCxxRevisionMacro(vtkColorTransferFunctionWidget, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkColorTransferFunctionWidget);
vtkCxxSetObjectMacro(vtkColorTransferFunctionWidget, Actor, vtkColorTransferFunctionActor);

vtkColorTransferFunctionWidget::vtkColorTransferFunctionWidget()
{
  this->Actor = vtkColorTransferFunctionActor::New();
  this->EventCallbackCommand->SetCallback(vtkColorTransferFunctionWidget::ProcessEvents);
  this->State = vtkColorTransferFunctionWidget::Outside;
  this->Priority = 0.55;
}

vtkColorTransferFunctionWidget::~vtkColorTransferFunctionWidget()
{
    if( this->Actor )
        this->Actor->Delete();
}

void vtkColorTransferFunctionWidget::SetEnabled(int enabling)
{
  if ( ! this->Interactor )
    {
    vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
    return;
    }
  
  if ( enabling ) 
    {
    vtkDebugMacro(<<"Enabling color transfer function widget");
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
    i->AddObserver(vtkCommand::MouseMoveEvent,
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::LeftButtonPressEvent,
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::LeftButtonReleaseEvent,
                   this->EventCallbackCommand, this->Priority);


    // Add the scalar bar
    this->CurrentRenderer->AddViewProp(this->Actor);
    this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    }
  else //disabling------------------------------------------
    {
    vtkDebugMacro(<<"Disabling line widget");
    if ( ! this->Enabled ) //already disabled, just return
      {
      return;
      }
    this->Enabled = 0;

    // don't listen for events any more
    this->Interactor->RemoveObserver(this->EventCallbackCommand);

    // turn off the line
    this->CurrentRenderer->RemoveViewProp(this->Actor);
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->SetCurrentRenderer(NULL);
    }

  this->Interactor->Render();
}

void vtkColorTransferFunctionWidget::ProcessEvents(vtkObject* vtkNotUsed(object),
                                       unsigned long event,
                                       void* clientdata,
                                       void* vtkNotUsed(calldata))
{
  vtkColorTransferFunctionWidget* self = reinterpret_cast<vtkColorTransferFunctionWidget *>( clientdata );

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


void vtkColorTransferFunctionWidget::OnLeftButtonDown()
{
  // We're only here is we are enabled
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  if( this->Actor->PickPoint( X, Y, this->CurrentRenderer ) )
  {
    this->State = vtkColorTransferFunctionWidget::Moving;
    this->EventCallbackCommand->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  }

}

void vtkColorTransferFunctionWidget::OnMouseMove()
{
  // compute some info we need for all cases
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  if( this->State == vtkColorTransferFunctionWidget::Moving )
  {
    this->Actor->MoveCurrentPoint( X, Y );
    this->EventCallbackCommand->SetAbortFlag(1);
    this->Interactor->Render();
  }
}

void vtkColorTransferFunctionWidget::OnLeftButtonUp()
{
    if (this->State == vtkColorTransferFunctionWidget::Moving )
    {
        this->State = vtkColorTransferFunctionWidget::Outside;
        this->EventCallbackCommand->SetAbortFlag(1);
        this->EndInteraction();
        this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    }
}


void vtkColorTransferFunctionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Actor: " << this->Actor << "\n";
}

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVolumeTransferFunctionWidget.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVolumeTransferFunctionWidget.h"
#include "vtkColorTransferFunctionActor.h"
#include "vtkPiecewiseFunctionActor.h"
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCoordinate.h"
#include "vtkScalarBarActor.h"


vtkCxxRevisionMacro(vtkVolumeTransferFunctionWidget, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkVolumeTransferFunctionWidget);
vtkCxxSetObjectMacro(vtkVolumeTransferFunctionWidget, ColorActor, vtkColorTransferFunctionActor);
vtkCxxSetObjectMacro(vtkVolumeTransferFunctionWidget, AlphaActor, vtkPiecewiseFunctionActor);


vtkVolumeTransferFunctionWidget::vtkVolumeTransferFunctionWidget()
{
  this->ColorActor = vtkColorTransferFunctionActor::New();
  this->ColorActor->GetPositionCoordinate()->SetValue( 0.1, 0.1 );
  this->ColorActor->GetPosition2Coordinate()->SetValue( 0.8, 0.3 );
  this->ColorActor->GetScalarBarActor()->SetNumberOfLabels( 0 );
  this->AlphaActor = vtkPiecewiseFunctionActor::New();
  this->AlphaActor->GetPositionCoordinate()->SetValue( 0.1, 0.4 );
  this->AlphaActor->GetPosition2Coordinate()->SetValue( 0.8, 0.5 );
  this->EventCallbackCommand->SetCallback(vtkVolumeTransferFunctionWidget::ProcessEvents);
  this->State = vtkVolumeTransferFunctionWidget::Outside;
  this->Priority = 0.55;
}

vtkVolumeTransferFunctionWidget::~vtkVolumeTransferFunctionWidget()
{
    if( this->ColorActor )
        this->ColorActor->Delete();
    if( this->AlphaActor )
        this->AlphaActor->Delete();
}


void vtkVolumeTransferFunctionWidget::SetEnabled(int enabling)
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
    this->CurrentRenderer->AddViewProp(this->AlphaActor);
    this->CurrentRenderer->AddViewProp(this->ColorActor);
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
    this->CurrentRenderer->RemoveViewProp(this->ColorActor);
    this->CurrentRenderer->RemoveViewProp(this->AlphaActor);
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->SetCurrentRenderer(NULL);
    }

  this->Interactor->Render();
}

void vtkVolumeTransferFunctionWidget::ProcessEvents(vtkObject* vtkNotUsed(object),
                                       unsigned long event,
                                       void* clientdata,
                                       void* vtkNotUsed(calldata))
{
  vtkVolumeTransferFunctionWidget* self = reinterpret_cast<vtkVolumeTransferFunctionWidget *>( clientdata );

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


void vtkVolumeTransferFunctionWidget::OnLeftButtonDown()
{
  // We're only here is we are enabled
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  if( this->ColorActor->PickPoint( X, Y, this->CurrentRenderer ) )
  {
    this->State = vtkVolumeTransferFunctionWidget::ColorMoving;
    this->EventCallbackCommand->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  }
  else
  {
    int control = this->Interactor->GetControlKey();
    int shift = this->Interactor->GetShiftKey();

    if( control )
    {
        this->AlphaActor->RemovePoint( X, Y, this->CurrentRenderer );
        this->EventCallbackCommand->SetAbortFlag(1);
        this->Interactor->Render();
    }
    else if( shift )
    {
        this->AlphaActor->AddPoint( X, Y );
        this->EventCallbackCommand->SetAbortFlag(1);
        this->Interactor->Render();
    }
    else if( this->AlphaActor->PickPoint( X, Y, this->CurrentRenderer ) )
    {
        this->State = vtkVolumeTransferFunctionWidget::AlphaMoving;
        this->EventCallbackCommand->SetAbortFlag(1);
        this->StartInteraction();
        this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    }
  }
}


void vtkVolumeTransferFunctionWidget::OnMouseMove()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  if( this->State == vtkVolumeTransferFunctionWidget::ColorMoving )
  {
    this->ColorActor->MoveCurrentPoint( X, Y );
    this->EventCallbackCommand->SetAbortFlag(1);
    this->Interactor->Render();
  }
  else if( this->State == vtkVolumeTransferFunctionWidget::AlphaMoving )
  {
    this->AlphaActor->MoveCurrentPoint( X, Y );
    this->EventCallbackCommand->SetAbortFlag(1);
    this->Interactor->Render();
  }
}

void vtkVolumeTransferFunctionWidget::OnLeftButtonUp()
{
    if (this->State == vtkVolumeTransferFunctionWidget::ColorMoving ||
        this->State == vtkVolumeTransferFunctionWidget::AlphaMoving )
    {
        this->State = vtkVolumeTransferFunctionWidget::Outside;
        this->EventCallbackCommand->SetAbortFlag(1);
        this->EndInteraction();
        this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    }
}


void vtkVolumeTransferFunctionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

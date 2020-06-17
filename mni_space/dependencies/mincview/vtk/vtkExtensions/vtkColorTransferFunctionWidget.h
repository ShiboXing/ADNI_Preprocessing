/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkColorTransferFunctionWidget.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkColorTransferFunctionWidget - 2D widget for manipulating a
//  vtkColorTransferFunction.
// .SECTION Description

// .SECTION See Also
// vtkInteractorObserver


#ifndef __vtkColorTransferFunctionWidget_h
#define __vtkColorTransferFunctionWidget_h

#include "vtkReal.h"
#include "vtkInteractorObserver.h"


class vtkColorTransferFunctionActor;


class VTK_HYBRID_EXPORT vtkColorTransferFunctionWidget : public vtkInteractorObserver
{
public:
  static vtkColorTransferFunctionWidget *New();
  vtkTypeRevisionMacro(vtkColorTransferFunctionWidget,vtkInteractorObserver);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetActor(vtkColorTransferFunctionActor *actor);
  vtkGetObjectMacro(Actor,vtkColorTransferFunctionActor);

  // Description:
  // Methods for turning the interactor observer on and off.
  virtual void SetEnabled(int);

protected:

  vtkColorTransferFunctionWidget();
  ~vtkColorTransferFunctionWidget();

  // the actor that is used
  vtkColorTransferFunctionActor * Actor;

  //handles the events
  static void ProcessEvents(vtkObject* object, 
                            unsigned long event,
                            void* clientdata, 
                            void* calldata);

  // ProcessEvents() dispatches to these methods.
  void OnLeftButtonDown();
  void OnLeftButtonUp();
  void OnMouseMove();

//BTX - manage the state of the widget
  int State;
  enum WidgetState
  {
    Moving=0,
    Outside
  };
//ETX

private:
  vtkColorTransferFunctionWidget(const vtkColorTransferFunctionWidget&);  //Not implemented
  void operator=(const vtkColorTransferFunctionWidget&);  //Not implemented
};

#endif

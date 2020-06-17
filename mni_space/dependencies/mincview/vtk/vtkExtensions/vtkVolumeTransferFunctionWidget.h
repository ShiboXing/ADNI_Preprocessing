/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVolumeTransferFunctionWidget.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVolumeTransferFunctionWidget - 2D widget for manipulating a
//  vtkColorTransferFunction and vtkPiecewiseFunction
// .SECTION Description

// .SECTION See Also
// vtkInteractorObserver


#ifndef __vtkVolumeTransferFunctionWidget_h
#define __vtkVolumeTransferFunctionWidget_h


#include "vtkReal.h"
#include "vtkInteractorObserver.h"


class vtkColorTransferFunctionActor;
class vtkPiecewiseFunctionActor;


class VTK_HYBRID_EXPORT vtkVolumeTransferFunctionWidget : public vtkInteractorObserver
{
public:
  static vtkVolumeTransferFunctionWidget *New();
  vtkTypeRevisionMacro(vtkVolumeTransferFunctionWidget,vtkInteractorObserver);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetColorActor(vtkColorTransferFunctionActor *actor);
  vtkGetObjectMacro(ColorActor,vtkColorTransferFunctionActor);
  
  void SetAlphaActor( vtkPiecewiseFunctionActor * actor );
  vtkGetObjectMacro( AlphaActor,vtkPiecewiseFunctionActor);

  // Description:
  // Methods for turning the interactor observer on and off.
  virtual void SetEnabled(int);

protected:

  vtkVolumeTransferFunctionWidget();
  ~vtkVolumeTransferFunctionWidget();

  // the actor that is used
  vtkColorTransferFunctionActor * ColorActor;
  vtkPiecewiseFunctionActor * AlphaActor;

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
    ColorMoving=0,
    AlphaMoving,
    Outside
  };
//ETX

private:
  vtkVolumeTransferFunctionWidget(const vtkVolumeTransferFunctionWidget&);  //Not implemented
  void operator=(const vtkVolumeTransferFunctionWidget&);  //Not implemented
};

#endif

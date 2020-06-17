/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPiecewiseFunctionWidget.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPiecewiseFunctionWidget - 2D widget for manipulating a color transfer function
// .SECTION Description
// .SECTION See Also
// vtkInteractorObserver vtkPiecewiseFunctionActor


#ifndef __vtkPiecewiseFunctionWidget_h
#define __vtkPiecewiseFunctionWidget_h


#include "vtkReal.h"
#include "vtkInteractorObserver.h"
#include "vtkPiecewiseFunctionActor.h"


class vtkPiecewiseFunctionWidget : public vtkInteractorObserver
{

public:

    static vtkPiecewiseFunctionWidget *New();
    vtkTypeRevisionMacro(vtkPiecewiseFunctionWidget,vtkInteractorObserver);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Methods for turning the interactor observer on and off.
    virtual void SetEnabled(int);
    
    vtkSetObjectMacro(Actor,vtkPiecewiseFunctionActor);
    vtkGetObjectMacro(Actor,vtkPiecewiseFunctionActor);


protected:

    vtkPiecewiseFunctionWidget();
    ~vtkPiecewiseFunctionWidget();

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
        MovingPoint=0,
        Inside,
        Outside
    };
//ETX

    vtkPiecewiseFunctionActor * Actor;

private:

    vtkPiecewiseFunctionWidget(const vtkPiecewiseFunctionWidget&);  //Not implemented
    void operator=(const vtkPiecewiseFunctionWidget&);  //Not implemented

};


#endif

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkColorTransferFunctionActor.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkColorTransferFunctionActor - Create a scalar bar with labels
// .SECTION Description
// .SECTION See Also
// vtkScalarBarActor vtkActor2D vtkTextProperty vtkTextMapper vtkPolyDataMapper2D

#ifndef __vtkColorTransferFunctionActor_h
#define __vtkColorTransferFunctionActor_h

#include "vtkReal.h"
#include "vtkActor2D.h"
#include "vtkPropAssembly.h"

class vtkColorTransferFunction;
class vtkScalarBarActor;
class vtkDiskSource;
class vtkPolyDataMapper2D;
class vtkPropPicker;
class vtkRenderer;


class vtkColorTransferFunctionActor : public vtkActor2D
{
public:
  vtkTypeRevisionMacro(vtkColorTransferFunctionActor,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object with 64 maximum colors; 5 labels; %%-#6.3g label
  // format, no title, and vertical orientation. The initial scalar bar
  // size is (0.05 x 0.8) of the viewport size.
  static vtkColorTransferFunctionActor *New();

  // Description:
  // Draw the scalar bar and annotation text to the screen.
  int RenderOpaqueGeometry(vtkViewport* viewport);
  int RenderTranslucentGeometry(vtkViewport*) { return 0; };
  int RenderOverlay(vtkViewport* viewport);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Set/Get the vtkLookupTable to use. The lookup table specifies the number
  // of colors to use in the table (if not overridden), as well as the scalar
  // range.
  virtual void SetFunction(vtkColorTransferFunction*);
  vtkColorTransferFunction * GetFunction();

  // Description:
  // Get the scalar bar actor inside
  virtual void SetScalarBarActor(vtkScalarBarActor* actor);
  vtkGetObjectMacro(ScalarBarActor,vtkScalarBarActor);
  
  // Got to overide these functions, otherwise, the picker doesn't include the points of the graph
  void InitPathTraversal() { CursorActors->InitPathTraversal(); }
  vtkAssemblyPath * GetNextPath() { return CursorActors->GetNextPath(); }
  int GetNumberOfPaths() { return CursorActors->GetNumberOfPaths(); }

  int PickPoint( int x, int y, vtkRenderer * ren );
  void MoveCurrentPoint( int x, int y );

protected:

  vtkColorTransferFunctionActor();
  ~vtkColorTransferFunctionActor();

  void BuildRepresentation();
  void ComputeCursorScreenPosition( vtkReal x, int & xScreen, int & yScreen, vtkReal range );
  void MoveFunctionPoint( int index, vtkReal x );
  void ConstrainOnLine( int & x, int & y );
  void ScreenToFunction( int x, vtkReal & xFunc );

  vtkReal CursorPointRadius;
  vtkReal CursorLineLength;

  vtkDiskSource * CursorPointSource;
  vtkPolyDataMapper2D * CursorMapper;
  vtkPropAssembly * CursorActors;
  
  int MovingActorIndex;
  vtkActor2D * MovingActor;
  int ActorToIndex( vtkActor2D * actor );
  vtkActor2D * IndexToActor( int index );

  int CursorLinePoint1[2];
  int CursorLinePoint2[2];

  vtkPropPicker * CursorPicker;

private:

  vtkScalarBarActor * ScalarBarActor;

  vtkTimeStamp  BuildTime;
  int LastViewportSize[2];

private:
  vtkColorTransferFunctionActor(const vtkColorTransferFunctionActor&);  // Not implemented.
  void operator=(const vtkColorTransferFunctionActor&);  // Not implemented.
};


#endif


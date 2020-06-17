/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPiecewiseFunctionActor.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPiecewiseFunctionActor - generate an x-y plot from input 
//  piecewise linear function
// .SECTION Description

// .SECTION See Also
// vtkPiecewiseFunctionWidget

#ifndef __vtkPiecewiseFunctionActor_h
#define __vtkPiecewiseFunctionActor_h


#include "vtkReal.h"
#include "vtkActor2D.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPropAssembly.h"


class vtkPoints;
class vtkCellArray;
class vtkPolyData;
class vtkActor2D;
class vtkDiskSource;
class vtkPolyDataMapper2D;
class vtkPropPicker;
class vtkAxisActor2D;
class vtkRenderer;
class vtkProperty2D;


class vtkPiecewiseFunctionActor : public vtkActor2D
{
public:
  
  vtkTypeRevisionMacro(vtkPiecewiseFunctionActor,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object with autorange computation;
  static vtkPiecewiseFunctionActor *New();


  vtkSetObjectMacro(Function,vtkPiecewiseFunction);
  vtkGetObjectMacro(Function,vtkPiecewiseFunction);
  //BTX
  vtkSetVector4Macro(GraphExtent,vtkReal);
  //ETX
  vtkSetMacro(MovingPointIndex,int);
  vtkGetMacro(MovingPointIndex,int);
  
  // Functions used to edit the content of the function
  void MoveCurrentPoint( int x, int y );
  void MoveCurrentPointInGraphSpace( vtkReal x );
  void SetCurrentPoint( int pointIndex );
  int  PickPoint( int x, int y, vtkRenderer * ren );
  
  vtkReal AddPoint( int x, int y );
  vtkReal AddPointInGraphSpace( vtkReal x );
  
  int RemovePoint( int x, int y, vtkRenderer * ren );
  void RemovePointByIndex( int index );

  vtkProperty2D * GetPlotProperty();

//BTX
  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS.
  int RenderOpaqueGeometry(vtkViewport*);
  int RenderOverlay(vtkViewport*);
  int RenderTranslucentGeometry(vtkViewport *) {return 0;}

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

//ETX

  // Got to overide these functions, otherwise, the picker doesn't include the points of the graph
  void InitPathTraversal() { Points->InitPathTraversal(); }
  vtkAssemblyPath * GetNextPath() { return Points->GetNextPath(); }
  int GetNumberOfPaths() { return Points->GetNumberOfPaths(); }
  
  int IsInGraph( int x, int y );
  void GraphToWindow( vtkReal xg, vtkReal yg, int & xw, int & yw );
  void WindowToGraph( int xw, int yw, vtkReal & xg, vtkReal & yg );
  
  
protected:
    vtkPiecewiseFunctionActor();
    ~vtkPiecewiseFunctionActor();

    // Functions used to build representation
    void BuildRepresentation();
    void PushBackPoint( vtkReal x, vtkReal y );
    int  FindPointIndex( vtkActor2D * pointActor );
    void SetWindowPointPosition( int index, vtkReal x, vtkReal y );
    void SetGraphPointPosition( int index, vtkReal x, vtkReal y );
    void SetGraphPointPosition( int index, vtkReal x );
    void PermuteIfNeeded();
    void PermutePoints( int index1, int index2 );

    // Extent of the graph in window pixel coordinate
    vtkSetVector4Macro(WindowExtent,int);
    int WindowExtent[4];
    void ConstrainInGraph( int & x, int & y );

    // Extent of the content of the graph.
    vtkReal GraphExtent[4];
    void ConstrainInGraph( vtkReal & x, vtkReal & y );

    int LastViewportSize[2];

    // Lines that trace the function
    vtkReal                  PointRadius;
    vtkPoints             * LinesVertices;
    vtkCellArray          * LinesCells;
    vtkPolyData           * LinesPolyData;
    vtkPolyDataMapper2D   * LinesMapper;
    vtkActor2D            * LinesActor;

    // Dots that can be selected on the Function and their picker
    vtkDiskSource       * PointSource;
    vtkPolyDataMapper2D * PointMapper;
    vtkPropAssembly     * Points;
    vtkPropPicker       * PointPicker;

    // Axes
    vtkAxisActor2D * XAxis;
    vtkAxisActor2D * YAxis;

    // The funtion represented by the graph
    vtkPiecewiseFunction * Function;

    vtkTimeStamp  BuildTime;

    int PreviousNumPoints;
    int MovingPointIndex;
    vtkActor2D * MovingPointActor;
    vtkActor2D * PointActorFromIndex( int index );
//BTX
private:
  vtkPiecewiseFunctionActor(const vtkPiecewiseFunctionActor&);  // Not implemented.
  void operator=(const vtkPiecewiseFunctionActor&);  // Not implemented.
//ETX
};


#endif


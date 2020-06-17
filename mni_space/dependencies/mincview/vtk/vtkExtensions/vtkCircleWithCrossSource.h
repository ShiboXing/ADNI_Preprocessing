/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkCircleWithCrossSource.h,v $
  Language:  C++
  Date:      $Date: 2005-05-11 19:33:00 $
  Version:   $Revision: 1.5 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCircleWithCrossSource - create a 2D circle with a cross in it
// .SECTION Description
// vtkCircleWithCrossSource is a source object that creates a polyline that 
// forms a circle with a cross in the middle. The circle is centered at
// ( 0, 0 ). The parameters are the radius of the circle and the 
// number of lines used to draw it.

#ifndef __vtkCircleWithCrossSource_h
#define __vtkCircleWithCrossSource_h

#include "vtkReal.h"
#include "vtkPolyDataSource.h"

class vtkPoints;

class vtkCircleWithCrossSource : public vtkPolyDataSource 
{

public:

    static vtkCircleWithCrossSource *New();
    vtkTypeRevisionMacro(vtkCircleWithCrossSource,vtkPolyDataSource);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // The circle will be approximated with Resolusion
    // line segments.
    vtkSetClampMacro(Resolution,int,0,VTK_INT_MAX);
    vtkGetMacro(Resolution,int);

    // Description:
    // The radius of the circle.
    vtkSetClampMacro(Radius,double,0.000001,VTK_DOUBLE_MAX);
    vtkGetMacro(Radius,double);

protected:

    vtkCircleWithCrossSource( int res = 0, double radius = 1.0 );
    ~vtkCircleWithCrossSource() { };

    void Execute();
    void Subdivide( vtkPoints * points, int index1, int index2, int depth );
    
    double Radius;
    int Resolution;

private:

    vtkCircleWithCrossSource(const vtkCircleWithCrossSource&);  // Not implemented.
    void operator=(const vtkCircleWithCrossSource&);            // Not implemented.
};


#endif



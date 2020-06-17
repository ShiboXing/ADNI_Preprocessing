/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPlaneAndAxisActor.h,v $
  Language:  C++
  Date:      $Date: 2004-05-11 21:17:29 $
  Version:   $Revision: 1.3 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPlaneAndAxisActor - Utility Actor that contains a grid plane and axes.
// .SECTION Description
// 
// .SECTION Caveats
// .SECTION See Also


#ifndef __vtkPlaneAndAxisActor_h
#define __vtkPlaneAndAxisActor_h


#include "vtkReal.h"
#include "vtkAssembly.h"


class vtkAxes;
class vtkPolyDataMapper;
class vtkPolyData;               
class vtkPlaneSource;            
class vtkCaptionActor2D;         


class vtkPlaneAndAxisActor : public vtkAssembly
{

public:

  static vtkPlaneAndAxisActor * New();

  vtkTypeRevisionMacro( vtkPlaneAndAxisActor, vtkAssembly );
  void PrintSelf( ostream & os, vtkIndent indent );

  void SetRenderer( vtkRenderer * ren );

  // Will be implemented when time is available. Otherwise use defaults.
  /*vtkSetMacro(NumberOfSubdivisions, int);
  vtkGetMacro(NumberOfSubdivisions, int);
  vtkSetMacro(Size, double);
  vtkGetMacro(Size, double);*/
  
protected:

  vtkPlaneAndAxisActor();
  ~vtkPlaneAndAxisActor();

  vtkAxes           * m_axes;
  vtkPolyDataMapper * m_axesMapper;
  vtkActor          * m_axesActor;
  vtkPolyData       * m_gridPolyData;
  vtkPolyDataMapper * m_gridMapper;
  vtkActor          * m_gridActor;
  vtkPlaneSource    * m_planeSource;
  vtkPolyDataMapper * m_planeMapper;
  vtkActor          * m_planeActor;
  vtkCaptionActor2D * m_captionx;
  vtkCaptionActor2D * m_captiony;
  vtkCaptionActor2D * m_captionz;

  double Size;
  int    NumberOfSubdivisions;


private:

  vtkPlaneAndAxisActor( const vtkPlaneAndAxisActor & );        // Not implemented
  void operator=( const vtkPlaneAndAxisActor & );              // Not implemented

};

#endif

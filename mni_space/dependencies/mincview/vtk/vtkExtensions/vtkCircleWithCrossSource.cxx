/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkCircleWithCrossSource.cxx,v $
  Language:  C++
  Date:      $Date: 2005-05-11 19:33:00 $
  Version:   $Revision: 1.4 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCircleWithCrossSource.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkMath.h"

#include <math.h>

vtkCxxRevisionMacro(vtkCircleWithCrossSource, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkCircleWithCrossSource);

vtkCircleWithCrossSource::vtkCircleWithCrossSource( int res, double radius )
{
    this->Radius = ( radius <= 0.0 ? 0.0 : radius );
    this->Resolution = (res < 0 ? 0 : res);
}

void vtkCircleWithCrossSource::Execute()
{
    vtkDebugMacro(<<"Creating line");

    // Generate points
    int numLines = (int)(pow( 2, 2 + this->Resolution ));
    int numPts = numLines;
    int increment = (int)(pow( 2, this->Resolution ));
    vtkPoints * newPoints = vtkPoints::New();
    newPoints->SetNumberOfPoints(numPts);
    newPoints->SetPoint( 0, 0.0, this->Radius, 0.0 );
    newPoints->SetPoint( increment, this->Radius, 0.0, 0.0 );
    newPoints->SetPoint( 2 * increment, 0.0, -this->Radius, 0.0 );
    newPoints->SetPoint( 3 * increment, -this->Radius, 0.0, 0.0 );
    if( this->Resolution > 0 )
    {
        this->Subdivide( newPoints, 0, increment, 1 );
        this->Subdivide( newPoints, increment, 2 * increment, 1 );
        this->Subdivide( newPoints, 2 * increment, 3 * increment, 1 );
        this->Subdivide( newPoints, 3 * increment, 0, 1 );
    }

    //  Generate circle lines
    vtkCellArray  * newLines;
    newLines = vtkCellArray::New();
    int size = newLines->EstimateSize( 1, numLines + 1 ) + newLines->EstimateSize( 2, 2 );
    newLines->Allocate( size );
    newLines->InsertNextCell(numPts+1);
    int i;
    for ( i = 0; i < numPts; i++ )
    {
        newLines->InsertCellPoint( i );
    }
    newLines->InsertCellPoint( 0 );

    // Generate cross lines
    newLines->InsertNextCell(2);
    newLines->InsertCellPoint( 0 );
    newLines->InsertCellPoint( 2 * increment );
    newLines->InsertNextCell(2);
    newLines->InsertCellPoint( 3 * increment );
    newLines->InsertCellPoint( increment );

    // Update ourselves and release memory
    this->GetOutput()->SetPoints(newPoints);
    newPoints->Delete();

    this->GetOutput()->SetLines(newLines);
    newLines->Delete();
}

void vtkCircleWithCrossSource::Subdivide( vtkPoints * points, int index1, int index2, int depth )
{
    // compute the position vector of the new point
    vtkReal * point1 = points->GetPoint( index1 );
    vtkReal * point2 = points->GetPoint( index2 );
    vtkReal newPoint[3];
    newPoint[0] = point2[0] + point1[0];
    newPoint[1] = point2[1] + point1[1];
    newPoint[2] = point2[2] + point1[2];
    vtkMath::Normalize( newPoint );
    newPoint[0] *= this->Radius;
    newPoint[1] *= this->Radius;
    newPoint[2] *= this->Radius;

    // place the new point in the point array
    int newIndex = index1 + (int)pow( 2, this->Resolution - depth );
    points->SetPoint( newIndex, newPoint );

    // do finer subdivision
    if( depth < this->Resolution )
    {
        this->Subdivide( points, index1, newIndex, depth + 1 );
        this->Subdivide( points, newIndex, index2, depth + 1 );
    }
}

void vtkCircleWithCrossSource::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    os << indent << "Resolution: " << this->Resolution << "\n";

    os << indent << "Radius: (" << this->Radius << "\n";
}

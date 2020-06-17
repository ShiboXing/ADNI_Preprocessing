/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPiecewiseFunctionActor.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// TODO: create a new static class with functions for the manipulation of
// vtkPiecewiseFunction and move the code of functions like permute
// in it so that it can benefit other classes.

#include "vtkPiecewiseFunctionActor.h"
#include "vtkRenderer.h"
#include "vtkPropAssembly.h"
#include "vtkAssemblyNode.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkDiskSource.h"
#include "vtkAssemblyPath.h"
#include "vtkAxisActor2D.h"
#include "vtkObjectFactory.h"
#include "vtkPropPicker.h"
#include "vtkProperty2D.h"


vtkCxxRevisionMacro(vtkPiecewiseFunctionActor, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkPiecewiseFunctionActor);


//----------------------------------------------------------------------------
// Instantiate object
vtkPiecewiseFunctionActor::vtkPiecewiseFunctionActor()
{
    this->Function = vtkPiecewiseFunction::New();
    this->MovingPointIndex = -1;
    this->MovingPointActor = 0;
    this->PointRadius = 5.0;
    this->PreviousNumPoints = 0;

    this->LinesVertices = vtkPoints::New();
    this->LinesCells = vtkCellArray::New();
    this->LinesPolyData = vtkPolyData::New();
    this->LinesPolyData->SetPoints( this->LinesVertices );
    this->LinesPolyData->SetLines( this->LinesCells );
    this->LinesMapper = vtkPolyDataMapper2D::New();
    this->LinesMapper->SetInput( this->LinesPolyData );
    this->LinesActor = vtkActor2D::New();
    this->LinesActor->SetMapper( this->LinesMapper );

    this->PointSource = vtkDiskSource::New();
    this->PointSource->SetInnerRadius( 0.0 );
    this->PointSource->SetOuterRadius( this->PointRadius );
    this->PointSource->SetCircumferentialResolution( 20 );

    this->PointMapper = vtkPolyDataMapper2D::New();
    this->PointMapper->SetInput( this->PointSource->GetOutput() );

    this->Points = vtkPropAssembly::New();

    this->PointPicker = vtkPropPicker::New();

    this->SetGraphExtent( 0, 0, 0, 0 );
    this->SetWindowExtent( 0, 0, 0, 0 );

    this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport();
    this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport();

    this->XAxis = vtkAxisActor2D::New();
    this->XAxis->GetPositionCoordinate()->SetCoordinateSystemToViewport();
    this->XAxis->GetPosition2Coordinate()->SetCoordinateSystemToViewport();
    this->XAxis->AdjustLabelsOff();

    this->YAxis = vtkAxisActor2D::New();
    this->YAxis->GetPositionCoordinate()->SetCoordinateSystemToViewport();
    this->YAxis->GetPosition2Coordinate()->SetCoordinateSystemToViewport();
    this->YAxis->AdjustLabelsOff();
}

//----------------------------------------------------------------------------
vtkPiecewiseFunctionActor::~vtkPiecewiseFunctionActor()
{
    this->LinesVertices->Delete();
    this->LinesCells->Delete();
    this->LinesPolyData->Delete();
    this->LinesMapper->Delete();
    this->LinesActor->Delete();

    this->PointSource->Delete();
    this->PointMapper->Delete();
    this->Points->Delete();

    this->PointPicker->Delete();
}


void vtkPiecewiseFunctionActor::MoveCurrentPoint( int x, int y )
{
    if( this->MovingPointActor )
    {
        int newX = x;
        int newY = y;
        this->ConstrainInGraph( newX, newY );
        vtkReal xg, yg;
        this->WindowToGraph( newX, newY, xg, yg );
        this->SetGraphPointPosition( this->MovingPointIndex, xg, yg );

        // See if we need to permute points
        this->PermuteIfNeeded();

        this->Function->Modified();
    }
}


void vtkPiecewiseFunctionActor::MoveCurrentPointInGraphSpace( vtkReal x )
{
    if( this->MovingPointActor )
    {
        vtkReal xNew, yNew;
        yNew = 0;
        xNew = x;
        this->ConstrainInGraph( xNew, yNew );
        this->SetGraphPointPosition( this->MovingPointIndex, xNew );

        // See if we need to permute points
        this->PermuteIfNeeded();

        this->Function->Modified();
    }
}


int vtkPiecewiseFunctionActor::PickPoint( int x, int y, vtkRenderer * ren )
{
    this->PointPicker->PickProp ( x, y, ren, Points->GetParts() );
    vtkAssemblyPath * path = this->PointPicker->GetPath();
    if ( path != NULL )
    {
        path->InitTraversal();
        vtkAssemblyNode * node1 = path->GetNextNode();
        vtkProp * prop1 = node1->GetViewProp();
        this->MovingPointActor = vtkActor2D::SafeDownCast( prop1 );
        if( this->MovingPointActor )
        {
            this->MovingPointIndex = this->FindPointIndex( this->MovingPointActor );
            return 1;
        }
    }
    return 0;
}


void vtkPiecewiseFunctionActor::PermuteIfNeeded()
{
    vtkReal * points = this->Function->GetDataPointer();
    int index = this->MovingPointIndex;
    int nextIndex = index + 1;
    int prevIndex = index - 1;
    int numPoints = this->Function->GetSize();
    if( prevIndex >= 0 && points[ index * 2 ] < points[ prevIndex * 2 ] )
    {
        PermutePoints( prevIndex, index );
    }
    else if ( nextIndex < numPoints && points[ index * 2 ] > points[ nextIndex * 2 ] )
    {
        PermutePoints( index, nextIndex );
    }
}


void vtkPiecewiseFunctionActor::PermutePoints( int index1, int index2 )
{
    // permute function points
    int numPoints = this->Function->GetSize();
    if( index1 < numPoints && index2 < numPoints )
    {
        vtkReal * points = this->Function->GetDataPointer();

        vtkReal tmp[2];
        tmp[0] = points[ 2 * index2 ];
        tmp[1] = points[ 2 * index2 + 1];
        points[ 2 * index2 ] = points[ 2 * index1 ];
        points[ 2 * index2 + 1 ] = points[ 2 * index1 + 1 ];
        points[ 2 * index1 ] = tmp[0];
        points[ 2 * index1 + 1 ] = tmp[1];
        this->Function->Modified();
        if( this->MovingPointIndex == index1 )
            this->MovingPointIndex = index2;
        else if( this->MovingPointIndex == index2 )
            this->MovingPointIndex = index1;
        this->MovingPointActor = this->PointActorFromIndex( this->MovingPointIndex );
    }
}

// return the x value in graph coord of the inserted point
vtkReal vtkPiecewiseFunctionActor::AddPoint( int x, int y )
{
    int newX = x;
    int newY = y;
    this->ConstrainInGraph( newX, newY );
    vtkReal xg, yg;
    this->WindowToGraph( newX, newY, xg, yg );
    this->Function->AddPoint( xg, yg );
    return xg;
}

// Return the y value of the inserted point
vtkReal vtkPiecewiseFunctionActor::AddPointInGraphSpace( vtkReal x )
{
    vtkReal newX, newY;
    newY = 0;
    newX = x;
    this->ConstrainInGraph( newX, newY );
    newY = this->Function->GetValue( newX );
    this->Function->AddPoint( newX, newY );
    return newY;
}


int vtkPiecewiseFunctionActor::RemovePoint( int x, int y, vtkRenderer * ren )
{
    if( this->PickPoint( x, y, ren ) )
    {
        vtkReal value = this->Function->GetDataPointer()[ this->MovingPointIndex * 2 ];
        this->Function->RemovePoint( value );
        return this->MovingPointIndex;
    }
    return -1;
}


void vtkPiecewiseFunctionActor::RemovePointByIndex( int index )
{
    if( index < this->Function->GetSize() )
    {
        vtkReal value = this->Function->GetDataPointer()[ this->MovingPointIndex * 2 ];
        this->Function->RemovePoint( value );
    }
}


vtkProperty2D * vtkPiecewiseFunctionActor::GetPlotProperty()
{
    return this->LinesActor->GetProperty();
}


//----------------------------------------------------------------------------
// Plot scalar data for each input dataset.
int vtkPiecewiseFunctionActor::RenderOverlay(vtkViewport *viewport)
{
    int renderedSomething=0;

    renderedSomething += this->LinesActor->RenderOverlay(viewport);
    renderedSomething += this->Points->RenderOverlay(viewport);
    renderedSomething += this->XAxis->RenderOverlay(viewport);
    renderedSomething += this->YAxis->RenderOverlay(viewport);

    return renderedSomething;
}

//----------------------------------------------------------------------------
// Plot scalar data for each input dataset.
int vtkPiecewiseFunctionActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
    int renderedSomething=0;

    int * size = viewport->GetSize();
    if ( size[0] != this->LastViewportSize[0] ||
         size[1] != this->LastViewportSize[1] ||
         this->Function->GetMTime() > this->BuildTime ||
         this->GetMTime() > this->BuildTime )
    {
        // compute the new WindowExtent
        vtkDebugMacro(<<"Rebuilding graph");
        this->LastViewportSize[0] = size[0];
        this->LastViewportSize[1] = size[1];

        int * p1 = this->PositionCoordinate->GetComputedViewportValue(viewport);
        int * p2 = this->Position2Coordinate->GetComputedViewportValue(viewport);

        this->WindowExtent[0] = p1[0];
        this->WindowExtent[1] = p2[0];
        this->WindowExtent[2] = p1[1];
        this->WindowExtent[3] = p2[1];
        
        // initialize graph extent if not
        if( this->GraphExtent[0] == 0 &&
            this->GraphExtent[1] == 0 &&
            this->GraphExtent[2] == 0 &&
            this->GraphExtent[3] == 0 )
        {
            this->GraphExtent[0] = this->Function->GetRange()[0];
            this->GraphExtent[1] = this->Function->GetRange()[1];
            this->GraphExtent[2] = 0;
            this->GraphExtent[3] = 1;
        }

        this->XAxis->GetPositionCoordinate()->SetValue( p1[0], p1[1] );
        this->XAxis->GetPosition2Coordinate()->SetValue( p2[0], p1[1] );
        this->XAxis->SetRange( this->GraphExtent[0], this->GraphExtent[1] );
        this->YAxis->GetPosition2Coordinate()->SetValue( p1[0], p1[1] );
        this->YAxis->GetPositionCoordinate()->SetValue( p1[0], p2[1] );
        this->YAxis->SetRange( this->GraphExtent[3], this->GraphExtent[2] );

        BuildRepresentation();

        this->BuildTime.Modified();
    }

    vtkDebugMacro(<<"Rendering Axes");
    renderedSomething += this->XAxis->RenderOpaqueGeometry(viewport);
    renderedSomething += this->YAxis->RenderOpaqueGeometry(viewport);
    renderedSomething += this->LinesActor->RenderOpaqueGeometry(viewport);
    renderedSomething += this->Points->RenderOpaqueGeometry(viewport);

    return renderedSomething;
}


//----------------------------------------------------------------------------
// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkPiecewiseFunctionActor::ReleaseGraphicsResources(vtkWindow *win)
{
    this->XAxis->ReleaseGraphicsResources(win);
    this->YAxis->ReleaseGraphicsResources(win);
    this->LinesActor->ReleaseGraphicsResources(win);
    this->Points->ReleaseGraphicsResources(win);
}



//----------------------------------------------------------------------------
void vtkPiecewiseFunctionActor::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkPiecewiseFunctionActor::BuildRepresentation()
{
    // Cleanup previous representation
    this->LinesVertices->Reset();
    this->LinesCells->Reset();
    this->Points->GetParts()->RemoveAllItems();

    // Build new representation
    int numberOfPoints = Function->GetSize();
    vtkReal * functionPoints = Function->GetDataPointer();

    for( int i = 0; i < numberOfPoints; i++ )
    {
        this->PushBackPoint( functionPoints[ 2 * i ], functionPoints[ 2 * i + 1 ] );
    }
    
    if( numberOfPoints > this->MovingPointIndex && this->MovingPointIndex != -1 )
        this->MovingPointActor = this->PointActorFromIndex( this->MovingPointIndex );
    else
        this->MovingPointActor = 0;
}

//----------------------------------------------------------------------------
void vtkPiecewiseFunctionActor::PushBackPoint( vtkReal x, vtkReal y )
{
    int xWindow, yWindow;
    this->GraphToWindow( x, y, xWindow, yWindow );
    int newPointIndex = LinesVertices->GetNumberOfPoints();
    LinesVertices->InsertNextPoint( xWindow, yWindow, 0 );
    if( newPointIndex >= 1 )
    {
        LinesCells->InsertNextCell( 2 );
        LinesCells->InsertCellPoint( newPointIndex - 1 );
        LinesCells->InsertCellPoint( newPointIndex );
    }

    vtkActor2D * actor = vtkActor2D::New();
    actor->SetMapper( this->PointMapper );
    actor->SetPosition( xWindow, yWindow );
    this->Points->AddPart( actor );
    actor->Delete();
}

//----------------------------------------------------------------------------
int vtkPiecewiseFunctionActor::FindPointIndex( vtkActor2D * pointActor )
{
    vtkPropCollection * points = this->Points->GetParts();
    vtkProp * iterator = NULL;
    int i = 0;
    points->InitTraversal();
    while( (iterator = points->GetNextProp()) )
    {
        if( iterator == pointActor )
            return i;
         ++i;
    }
    return -1;
}

//----------------------------------------------------------------------------
void vtkPiecewiseFunctionActor::SetWindowPointPosition( int index, vtkReal x, vtkReal y )
{
    // Change the lines position
    this->LinesVertices->SetPoint( index, x, y, 0 );

    // Change the points positions
    this->MovingPointActor->SetPosition( x, y );

}

//----------------------------------------------------------------------------
void vtkPiecewiseFunctionActor::SetGraphPointPosition( int index, vtkReal x, vtkReal y )
{
    int numPoints = this->Function->GetSize();
    if( index < numPoints )
    {
        vtkReal * points = this->Function->GetDataPointer();
        points[ 2 * index ] = x;
        points[ 2 * index + 1 ] = y;
        this->Function->Modified();
    }
}


//----------------------------------------------------------------------------
void vtkPiecewiseFunctionActor::SetGraphPointPosition( int index, vtkReal x )
{
    int numPoints = this->Function->GetSize();
    if( index < numPoints )
    {
        vtkReal * points = this->Function->GetDataPointer();
        points[ 2 * index ] = x;
        this->Function->Modified();
    }
}


//----------------------------------------------------------------------------
int vtkPiecewiseFunctionActor::IsInGraph( int x, int y )
{
    if( x >= this->WindowExtent[0] - this->PointRadius &&
        x <= this->WindowExtent[1] + this->PointRadius &&
        y >= this->WindowExtent[2] - this->PointRadius &&
        y <= this->WindowExtent[3] + this->PointRadius )
        return 1;
    return 0;
}

//----------------------------------------------------------------------------
void vtkPiecewiseFunctionActor::ConstrainInGraph( int & x, int & y )
{
    if( x < this->WindowExtent[0] )
        x = this->WindowExtent[0];

    if( x > this->WindowExtent[1] )
        x = this->WindowExtent[1];

    if( y < this->WindowExtent[2] )
        y = this->WindowExtent[2];

    if( y > this->WindowExtent[3] )
        y = this->WindowExtent[3];
}


void vtkPiecewiseFunctionActor::ConstrainInGraph( vtkReal & x, vtkReal & y )
{
    if( x < this->GraphExtent[0] )
        x = this->GraphExtent[0];

    if( x > this->GraphExtent[1] )
        x = this->GraphExtent[1];

    if( y < this->GraphExtent[2] )
        y = this->GraphExtent[2];

    if( y > this->GraphExtent[3] )
        y = this->GraphExtent[3];
}


//----------------------------------------------------------------------------
void vtkPiecewiseFunctionActor::GraphToWindow( vtkReal xg, vtkReal yg, int & xw, int & yw )
{
    int windowHeight = this->WindowExtent[3] - this->WindowExtent[2];
    int windowWidth = this->WindowExtent[1] - this->WindowExtent[0];
    vtkReal graphHeight = this->GraphExtent[3] - this->GraphExtent[2];
    vtkReal graphWidth = this->GraphExtent[1] - this->GraphExtent[0];

    vtkReal xRatio = (xg - this->GraphExtent[0]) / graphWidth;
    vtkReal yRatio = (yg - this->GraphExtent[2]) / graphHeight;
    xw = (int)(xRatio * windowWidth + this->WindowExtent[0]);
    yw = (int)(yRatio * windowHeight + this->WindowExtent[2]);
}

//----------------------------------------------------------------------------
void vtkPiecewiseFunctionActor::WindowToGraph( int xw, int yw, vtkReal & xg, vtkReal & yg )
{
    int windowHeight = this->WindowExtent[3] - this->WindowExtent[2];
    int windowWidth = this->WindowExtent[1] - this->WindowExtent[0];
    vtkReal graphHeight = this->GraphExtent[3] - this->GraphExtent[2];
    vtkReal graphWidth = this->GraphExtent[1] - this->GraphExtent[0];

    vtkReal xRatio = (vtkReal)(xw - this->WindowExtent[0]) / windowWidth;
    vtkReal yRatio = (vtkReal)(yw - this->WindowExtent[2]) / windowHeight;
    xg = xRatio * graphWidth + this->GraphExtent[0];
    yg = yRatio * graphHeight + this->GraphExtent[2];
}


vtkActor2D * vtkPiecewiseFunctionActor::PointActorFromIndex( int index )
{
    return vtkActor2D::SafeDownCast( Points->GetParts()->GetItemAsObject( index ) );
}

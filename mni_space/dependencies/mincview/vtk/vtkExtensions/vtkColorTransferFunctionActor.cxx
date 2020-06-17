/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkColorTransferFunctionActor.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// TODO: create a new static class with functions for the manipulation of
// vtkColorTransferFunction and move the code of functions like permute
// in it so that it can benefit other classes.

#include "vtkColorTransferFunctionActor.h"

#include "vtkObjectFactory.h"
#include "vtkColorTransferFunction.h"
#include "vtkViewport.h"
#include "vtkWindow.h"
#include "vtkScalarBarActor.h"
#include "vtkDiskSource.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPropPicker.h"
#include "vtkLineSource.h"
#include "vtkAppendPolyData.h"
#include "vtkPropCollection.h"
#include "vtkAssemblyPath.h"
#include "vtkAssemblyNode.h"


vtkCxxRevisionMacro(vtkColorTransferFunctionActor, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkColorTransferFunctionActor);

vtkCxxSetObjectMacro(vtkColorTransferFunctionActor, ScalarBarActor, vtkScalarBarActor);

void vtkColorTransferFunctionActor::SetFunction( vtkColorTransferFunction * func )
{
    this->ScalarBarActor->SetLookupTable( func );
}


vtkColorTransferFunction * vtkColorTransferFunctionActor::GetFunction()
{
    return vtkColorTransferFunction::SafeDownCast( this->ScalarBarActor->GetLookupTable() );
}


//----------------------------------------------------------------------------
vtkColorTransferFunctionActor::vtkColorTransferFunctionActor()
{
    this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport();
    this->PositionCoordinate->SetValue( 0.1, 0.1 );
    this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport();
    this->Position2Coordinate->SetValue( 0.8, 0.3 );

    this->ScalarBarActor = vtkScalarBarActor::New();
    this->ScalarBarActor->SetOrientationToHorizontal();
    this->ScalarBarActor->GetPositionCoordinate()->SetReferenceCoordinate(this->PositionCoordinate);
    this->ScalarBarActor->GetPosition2Coordinate()->SetReferenceCoordinate(this->Position2Coordinate);
    this->ScalarBarActor->GetPositionCoordinate()->SetValue( 0, .1 );
    this->ScalarBarActor->GetPosition2Coordinate()->SetValue( 0, 0 );
    this->ScalarBarActor->SetMaximumNumberOfColors( 256 );
    vtkColorTransferFunction * func = vtkColorTransferFunction::New();
    this->ScalarBarActor->SetLookupTable( func );

    this->CursorPointRadius = 5.0;
    this->CursorLineLength = 5.0;

    this->CursorPointSource = vtkDiskSource::New();
    this->CursorPointSource->SetInnerRadius( 0.0 );
    this->CursorPointSource->SetOuterRadius( this->CursorPointRadius );
    this->CursorPointSource->SetCircumferentialResolution( 20 );

    vtkLineSource * cursorLineSource = vtkLineSource::New();
    cursorLineSource->SetPoint1( 0, this->CursorPointRadius, 0 );
    cursorLineSource->SetPoint2( 0, this->CursorPointRadius + this->CursorLineLength, 0 );

    vtkAppendPolyData * cursorPolyData = vtkAppendPolyData::New();
    cursorPolyData->AddInput( this->CursorPointSource->GetOutput() );
    cursorPolyData->AddInput( cursorLineSource->GetOutput() );

    cursorLineSource->Delete();

    this->CursorMapper = vtkPolyDataMapper2D::New();
    this->CursorMapper->SetInput( cursorPolyData->GetOutput() );

    cursorPolyData->Delete();

    this->CursorActors = vtkPropAssembly::New();
    
    this->MovingActorIndex = -1;
    this->MovingActor = 0;

    this->CursorLinePoint1[0] = 0;
    this->CursorLinePoint1[1] = 0;
    this->CursorLinePoint2[0] = 0;
    this->CursorLinePoint2[1] = 0;

    this->LastViewportSize[0] = 0;
    this->LastViewportSize[1] = 0;

    this->CursorPicker = vtkPropPicker::New();
}

//----------------------------------------------------------------------------
void vtkColorTransferFunctionActor::ReleaseGraphicsResources(vtkWindow *win)
{
    if( this->ScalarBarActor )
        this->ScalarBarActor->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------------
vtkColorTransferFunctionActor::~vtkColorTransferFunctionActor()
{
    if( this->ScalarBarActor )
        this->ScalarBarActor->Delete();
}

//----------------------------------------------------------------------------
int vtkColorTransferFunctionActor::RenderOverlay(vtkViewport *viewport)
{
    int renderedSomething = 0;
    if( this->ScalarBarActor )
        renderedSomething += this->ScalarBarActor->RenderOverlay(viewport);
        
    this->CursorActors->RenderOverlay(viewport);

    renderedSomething = (renderedSomething > 0)?(1):(0);

    return renderedSomething;
}

//----------------------------------------------------------------------------
int vtkColorTransferFunctionActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int renderedSomething = 0;

  renderedSomething += this->ScalarBarActor->RenderOpaqueGeometry(viewport);
  
  int * scalarBarPoint1 = this->ScalarBarActor->GetPositionCoordinate()->GetComputedViewportValue( viewport );
  int * scalarBarPoint2 = this->ScalarBarActor->GetPosition2Coordinate()->GetComputedViewportValue( viewport );
  this->CursorLinePoint1[0] = scalarBarPoint1[0];
  this->CursorLinePoint1[1] = scalarBarPoint1[1] - (int)this->CursorPointRadius - (int)this->CursorLineLength;
  this->CursorLinePoint2[0] = scalarBarPoint2[0];
  this->CursorLinePoint2[1] = this->CursorLinePoint1[1];

  int * size = viewport->GetSize();
  if ( size[0] != this->LastViewportSize[0] ||
       size[1] != this->LastViewportSize[1] ||
       this->GetMTime() > this->BuildTime )
  {
    vtkDebugMacro(<<"Rebuilding graph");
    this->LastViewportSize[0] = size[0];
    this->LastViewportSize[1] = size[1];
    BuildRepresentation();
    this->BuildTime.Modified();
  }

  renderedSomething = (renderedSomething > 0)?(1):(0);

  return renderedSomething;
}


int vtkColorTransferFunctionActor::PickPoint( int x, int y, vtkRenderer * ren )
{
    this->CursorPicker->PickProp ( x, y, ren, CursorActors->GetParts() );
    vtkAssemblyPath * path = this->CursorPicker->GetPath();
    if ( path != NULL )
    {
        path->InitTraversal();
        vtkAssemblyNode * node1 = path->GetNextNode();
        vtkProp * prop1 = node1->GetViewProp();
        this->MovingActor = vtkActor2D::SafeDownCast( prop1 );
        if( this->MovingActor )
        {
            this->MovingActorIndex = this->ActorToIndex( this->MovingActor );
            return 1;
        }
    }
    return 0;
}


void vtkColorTransferFunctionActor::MoveCurrentPoint( int x, int y )
{
    if( this->MovingActor )
    {
        int newX = x;
        int newY = y;
        this->ConstrainOnLine( newX, newY );
        this->MovingActor->SetPosition( newX, newY );
        vtkReal xFunc;
        this->ScreenToFunction( newX, xFunc );
        this->MoveFunctionPoint( this->MovingActorIndex, xFunc );
    }
}


void vtkColorTransferFunctionActor::MoveFunctionPoint( int index, vtkReal x )
{
    vtkColorTransferFunction * func = this->GetFunction();
    if( func )
    {
        int numPoints = func->GetSize();
        if( index < numPoints )
        {
            vtkReal * points = func->GetDataPointer();
            points[ 4 * index ] = x;
            func->Modified();
        }
    }
}


void vtkColorTransferFunctionActor::ConstrainOnLine( int & x, int & y )
{
    y = this->CursorLinePoint1[1];
    if( x < this->CursorLinePoint1[0] )
        x = this->CursorLinePoint1[0];
    else if( x > this->CursorLinePoint2[0] - 1 )
        x = this->CursorLinePoint2[0] - 1;
}


//----------------------------------------------------------------------------
void vtkColorTransferFunctionActor::BuildRepresentation()
{
    this->CursorActors->GetParts()->RemoveAllItems();

    vtkColorTransferFunction * func = this->GetFunction();
    if( !func )
    {
        vtkWarningMacro(<<"No color transfer function has been specified for the scalar bar actor");
        return;
    }

    int numberOfPoints = func->GetSize();
    vtkReal * funcData = func->GetDataPointer();
    int xScreen, yScreen;
    vtkReal range = func->GetRange()[1] - func->GetRange()[0];
    for( int i = 0; i < numberOfPoints; i++ )
    {
        this->ComputeCursorScreenPosition( funcData[ 4 * i ], xScreen, yScreen, range );
        vtkActor2D * newActor = vtkActor2D::New();
        newActor->SetPosition( xScreen, yScreen );
        newActor->SetMapper( this->CursorMapper );
        this->CursorActors->AddPart( newActor );
        newActor->Delete();
    }

    if( this->MovingActorIndex > 0 && this->MovingActorIndex < numberOfPoints )
    {
        this->MovingActor = this->IndexToActor( this->MovingActorIndex );
    }
}

//----------------------------------------------------------------------------
void vtkColorTransferFunctionActor::ComputeCursorScreenPosition( vtkReal x, int & xScreen, int & yScreen, vtkReal range )
{
    vtkReal rangeScreen = (vtkReal)( this->CursorLinePoint2[0] - this->CursorLinePoint1[0] - 1 );
    xScreen = (int) ( x * rangeScreen / range ) + this->CursorLinePoint1[0];
    yScreen = this->CursorLinePoint1[1];
}


void vtkColorTransferFunctionActor::ScreenToFunction( int x, vtkReal & xFunc )
{
    vtkColorTransferFunction * func = this->GetFunction();
    if( func )
    {
        vtkReal screenRange = (vtkReal)( this->CursorLinePoint2[0] - this->CursorLinePoint1[0] );
        vtkReal funcRange = func->GetRange()[1] - func->GetRange()[0];
        xFunc = (vtkReal)( x - this->CursorLinePoint1[0] ) * funcRange / screenRange + func->GetRange()[0];
    }
}


int vtkColorTransferFunctionActor::ActorToIndex( vtkActor2D * actor )
{
    vtkPropCollection * points = this->CursorActors->GetParts();
    vtkProp * iterator = NULL;
    int i = 0;
    points->InitTraversal();
    while( (iterator = points->GetNextProp()) )
    {
        if( iterator == actor )
            return i;
         ++i;
    }
    return -1;
}


vtkActor2D * vtkColorTransferFunctionActor::IndexToActor( int index )
{
    return vtkActor2D::SafeDownCast( CursorActors->GetParts()->GetItemAsObject( index ) );
}


//----------------------------------------------------------------------------
void vtkColorTransferFunctionActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  this->ScalarBarActor->PrintSelf(os,indent);
}

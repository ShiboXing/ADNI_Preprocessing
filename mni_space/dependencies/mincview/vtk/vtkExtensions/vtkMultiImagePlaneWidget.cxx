/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMultiImagePlaneWidget.cxx,v $
  Language:  C++
  Date:      $Date: 2010-04-08 14:46:31 $
  Version:   $Revision: 1.21 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMultiImagePlaneWidget.h"

#include "vtkActor.h"
#include "vtkAssembly.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkImageData.h"
#include "vtkImageMapToColors.h"
#include "vtkImageReslice.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPlaneSource.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTexture.h"
#include "vtkTextureMapToPlane.h"
#include "vtkTransform.h"
#include "vtkObjectCallback.h"
#include "vtkIgnsEvents.h"

vtkCxxRevisionMacro(vtkMultiImagePlaneWidget, "$Revision: 1.21 $");
vtkStandardNewMacro(vtkMultiImagePlaneWidget);

vtkCxxSetObjectMacro(vtkMultiImagePlaneWidget, PlaneProperty, vtkProperty);
vtkCxxSetObjectMacro(vtkMultiImagePlaneWidget, SelectedPlaneProperty, vtkProperty);
vtkCxxSetObjectMacro(vtkMultiImagePlaneWidget, CursorProperty, vtkProperty);
vtkCxxSetObjectMacro(vtkMultiImagePlaneWidget, MarginProperty, vtkProperty);
vtkCxxSetObjectMacro(vtkMultiImagePlaneWidget, TexturePlaneProperty, vtkProperty);
vtkCxxSetObjectMacro(vtkMultiImagePlaneWidget, ColorMap, vtkImageMapToColors);

vtkMultiImagePlaneWidget::vtkMultiImagePlaneWidget() : vtkMulti3DWidget()
{
    this->State = vtkMultiImagePlaneWidget::Start;

    // Specify the events to be observed
    this->EventsObserved.push_back( vtkCommand::LeftButtonPressEvent );
    this->EventsObserved.push_back( vtkCommand::LeftButtonReleaseEvent );
    this->EventsObserved.push_back( vtkCommand::MiddleButtonPressEvent );
    this->EventsObserved.push_back( vtkCommand::MiddleButtonReleaseEvent );
    this->EventsObserved.push_back( vtkCommand::RightButtonPressEvent );
    this->EventsObserved.push_back( vtkCommand::RightButtonReleaseEvent );
    this->EventsObserved.push_back( vtkCommand::MouseMoveEvent );

    this->PlaneOrientation         = 0;
    this->RestrictPlaneToVolume    = 1;
    this->OriginalWindow           = 1.0;
    this->OriginalLevel            = 0.5;
    this->CurrentWindow            = 1.0;
    this->CurrentLevel             = 0.5;
    this->TextureInterpolate       = 1;
    this->ResliceInterpolate       = VTK_LINEAR_RESLICE;
    this->UserControlledLookupTable= 0;
    this->DisplayText              = 0;
    this->CurrentCursorPosition[0] = 0;
    this->CurrentCursorPosition[1] = 0;
    this->CurrentCursorPosition[2] = 0;
    this->CurrentWorldPosition[0] = 0.0;
    this->CurrentWorldPosition[1] = 0.0;
    this->CurrentWorldPosition[2] = 0.0;
    this->CurrentImageValue        = VTK_FLOAT_MAX;
    this->MarginSelectMode         = 8;

    // Represent the plane's outline geometry
    //
    this->PlaneSource = vtkPlaneSource::New();
    this->PlaneSource->SetXResolution(1);
    this->PlaneSource->SetYResolution(1);
    this->PlaneOutlinePolyData = vtkPolyData::New();
    this->GeneratePlaneOutline();

    // Represent the resliced image plane
    this->ColorMap           = vtkImageMapToColors::New();
    this->Reslice            = vtkImageReslice::New();
    this->ResliceAxes        = vtkMatrix4x4::New();
    this->Texture            = vtkTexture::New();
    this->TexturePlaneCoords = vtkTextureMapToPlane::New();
    this->Transform          = vtkTransform::New();
    this->ImageData          = 0;
    this->LookupTable        = 0;
    this->GenerateTexturePlane();

    // Represent the cross hair cursor
    this->CursorPolyData = vtkPolyData::New();
    this->GenerateCursor();

    // Represent the oblique positioning margins
    this->MarginPolyData = vtkPolyData::New();
    this->GenerateMargins();

    // Define some default point coordinates
    vtkReal bounds[6];
    bounds[0] = -0.5;
    bounds[1] =  0.5;
    bounds[2] = -0.5;
    bounds[3] =  0.5;
    bounds[4] = -0.5;
    bounds[5] =  0.5;

    // Initial creation of the widget, serves to initialize it
    this->PlaceWidget(bounds);

    // Set up the initial properties
    this->PlaneProperty         = 0;
    this->SelectedPlaneProperty = 0;
    this->TexturePlaneProperty  = 0;
    this->CursorProperty        = 0;
    this->MarginProperty        = 0;
    this->TextProperties        = 0;
    this->CreateDefaultProperties();

    // Set up actions
    this->LeftButtonAction = vtkMultiImagePlaneWidget::CURSOR_ACTION;
    this->MiddleButtonAction = vtkMultiImagePlaneWidget::SLICE_MOTION_ACTION;
    this->RightButtonAction = vtkMultiImagePlaneWidget::WINDOW_LEVEL_ACTION;

    // Set up modifiers
    this->LeftButtonAutoModifier = vtkMultiImagePlaneWidget::NO_MODIFIER;
    this->MiddleButtonAutoModifier = vtkMultiImagePlaneWidget::NO_MODIFIER;
    this->RightButtonAutoModifier = vtkMultiImagePlaneWidget::NO_MODIFIER;

    this->LastButtonPressed = vtkMultiImagePlaneWidget::NO_BUTTON;

    this->TextureVisibility = 1;
}

vtkMultiImagePlaneWidget::~vtkMultiImagePlaneWidget()
{
    this->ClearActors();

    this->PlaneOutlinePolyData->Delete();
    this->PlaneSource->Delete();

    if ( this->PlaneProperty )
    {
        this->PlaneProperty->Delete();
    }

    if ( this->SelectedPlaneProperty )
    {
        this->SelectedPlaneProperty->Delete();
    }

    if ( this->CursorProperty )
    {
        this->CursorProperty->Delete();
    }

    if ( this->MarginProperty )
    {
        this->MarginProperty->Delete();
    }

    this->ResliceAxes->Delete();
    this->Transform->Delete();
    this->Reslice->Delete();

    if ( this->LookupTable )
    {
        this->LookupTable->UnRegister(this);
    }

    this->TexturePlaneCoords->Delete();
    this->ColorMap->Delete();
    this->Texture->Delete();

    if ( this->TexturePlaneProperty )
    {
        this->TexturePlaneProperty->Delete();
    }

    if ( this->ImageData )
    {
        this->ImageData = 0;
    }

    this->CursorPolyData->Delete();

    this->MarginPolyData->Delete();
}


void vtkMultiImagePlaneWidget::SetTextureVisibility( int vis )
{
    if( this->TextureVisibility == vis )
    {
        return;
    }

    this->TextureVisibility = vis;

    if ( this->Enabled )
    {
        if( this->TextureVisibility )
        {
            RendererVec::iterator itRen = this->Renderers.begin();
            TextActorVec::iterator itActor = this->TextActors.begin();
            while( itRen != this->Renderers.end() )
            {
                (*itRen)->AddViewProp( (*itActor) );
                ++itRen;
                ++itActor;
            }
        }
        else
        {
            RendererVec::iterator itRen = this->Renderers.begin();
            TextActorVec::iterator itActor = this->TextActors.begin();
            while( itRen != this->Renderers.end() )
            {
                (*itRen)->RemoveViewProp( (*itActor) );
                ++itRen;
                ++itActor;
            }
        }
    }

    this->Modified();
}


template< class T >
void ClearVec( std::vector< T > & vec )
{
    typename std::vector< T >::iterator it = vec.begin();
    for( ; it != vec.end(); ++it )
    {
        if( *it )
            (*it)->Delete();
    }
    vec.clear();
}


void vtkMultiImagePlaneWidget::ClearActors()
{
    ClearVec( this->PlaneOutlineMappers );
    ClearVec( this->PlaneOutlineActors );
    ClearVec( this->TexturePlaneMappers );
    ClearVec( this->TexturePlaneActors );
    ClearVec( this->CursorMappers );
    ClearVec( this->CursorActors );
    ClearVec( this->MarginMappers );
    ClearVec( this->MarginActors );
    ClearVec( this->TextActors );
    ClearVec( this->PlanePickers );
}


void vtkMultiImagePlaneWidget::InternalAddRenderer( vtkRenderer * ren, vtkAssembly * assembly )
{
    // plane outline
    vtkPolyDataMapper * mapper = vtkPolyDataMapper::New();
    vtkActor * actor = vtkActor::New();
    mapper->SetInput( this->PlaneOutlinePolyData );
    mapper->SetResolveCoincidentTopologyToPolygonOffset();
    actor->SetMapper( mapper );
    actor->PickableOff();
    this->PlaneOutlineMappers.push_back( mapper );
    this->PlaneOutlineActors.push_back( actor );

    // texture plane
    mapper = vtkPolyDataMapper::New();
    vtkActor * texturePlaneActor = vtkActor::New();
    mapper->SetInput( vtkPolyData::SafeDownCast(this->TexturePlaneCoords->GetOutput() ) );
    texturePlaneActor->SetMapper( mapper );
    texturePlaneActor->SetTexture( this->Texture );
    texturePlaneActor->PickableOn();
    this->TexturePlaneMappers.push_back( mapper );
    this->TexturePlaneActors.push_back( texturePlaneActor );

    // Cursor
    mapper = vtkPolyDataMapper::New();
    actor = vtkActor::New();
    mapper->SetInput( this->CursorPolyData );
    mapper->SetResolveCoincidentTopologyToPolygonOffset();
    actor->SetMapper( mapper );
    actor->PickableOff();
    actor->VisibilityOff();
    this->CursorMappers.push_back( mapper );
    this->CursorActors.push_back( actor );

    // Margin
    mapper = vtkPolyDataMapper::New();
    actor = vtkActor::New();
    mapper->SetInput( this->MarginPolyData );
    mapper->SetResolveCoincidentTopologyToPolygonOffset();
    actor->SetMapper( mapper );
    actor->PickableOff();
    actor->VisibilityOff();
    this->MarginMappers.push_back( mapper );
    this->MarginActors.push_back( actor );

    // Text
    sprintf( this->TextBuff, "NA" );
    vtkTextActor * text = vtkTextActor::New();
    text->SetInput(this->TextBuff);
#if ((VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION < 2))
    text->ScaledTextOff();
#else    
    text->SetTextScaleModeToNone();
#endif    
    text->SetTextProperty( this->TextProperties );
    vtkCoordinate* coord = text->GetPositionCoordinate();
    coord->SetCoordinateSystemToNormalizedViewport();
    coord->SetValue(.01, .01);
    text->VisibilityOff();
    this->TextActors.push_back( text );

    // Picker
    vtkCellPicker * picker = vtkCellPicker::New();
    picker->SetTolerance(0.005); //need some fluff
    
    picker->PickFromListOn();
    this->PlanePickers.push_back( picker );
}


template< typename T >
void RemoveVecElement( int index, T & vec )
{
    if( index < vec.size() )
    {
        typename T::iterator it = vec.begin() + index;
        if( (*it) )
            (*it)->Delete();
        vec.erase( it );
    }
}

void vtkMultiImagePlaneWidget::InternalRemoveRenderer( int index )
{
    this->DisableForOneRenderer( index );
    RemoveVecElement( index, this->PlaneOutlineMappers );
    RemoveVecElement( index, this->PlaneOutlineActors );
    RemoveVecElement( index, this->TexturePlaneMappers );
    RemoveVecElement( index, this->TexturePlaneActors );
    RemoveVecElement( index, this->CursorMappers );
    RemoveVecElement( index, this->CursorActors );
    RemoveVecElement( index, this->MarginMappers );
    RemoveVecElement( index, this->MarginActors );
    RemoveVecElement( index, this->TextActors );
    RemoveVecElement( index, this->PlanePickers );
}

void vtkMultiImagePlaneWidget::InternalEnable()
{
    int index = 0;
    for( RendererVec::iterator it = this->Renderers.begin(); it != this->Renderers.end(); ++it, ++index )
    {
        // plane outline
        vtkActor * actor = this->PlaneOutlineActors[ index ];
        actor->SetProperty( this->PlaneProperty );
        if( this->Assemblies[ index ] )
            this->Assemblies[ index ]->AddPart( actor );
        else
            (*it)->AddViewProp( actor );

        // texture plane
        actor = this->TexturePlaneActors[ index ];
        actor->PickableOn();
        actor->SetProperty(this->TexturePlaneProperty);
        if (this->TextureVisibility)
        {
            if( this->Assemblies[ index ] )
                this->Assemblies[ index ]->AddPart( actor );
            else
                (*it)->AddViewProp( actor );
        }

        // Cursor
        actor = this->CursorActors[ index ];
        actor->SetProperty( this->CursorProperty );
        if( this->Assemblies[ index ] )
            this->Assemblies[ index ]->AddPart( actor );
        else
            (*it)->AddViewProp( actor );

        // Margin
        actor = this->MarginActors[ index ];
        actor->SetProperty( this->MarginProperty );
        if( this->Assemblies[ index ] )
            this->Assemblies[ index ]->AddPart( actor );
        else
            (*it)->AddViewProp( actor );

        // Text
        vtkTextActor * text = this->TextActors[ index ];
        text->VisibilityOff();
        (*it)->AddViewProp( text );

        // Picker
        if( this->PlanePickers[ index ] )
        {
            vtkAssembly * topAssembly = this->GetUppermostParent( (*it), this->TexturePlaneActors[ index ] );
            if( topAssembly )
            {
                this->PlanePickers[ index ]->AddPickList( topAssembly );
            }
            else
            {
                this->PlanePickers[ index ]->AddPickList( this->TexturePlaneActors[ index ] );
            }
            this->TexturePlaneActors[ index ]->PickableOn();
        }
    }
}


void vtkMultiImagePlaneWidget::InternalDisable()
{
    RendererVec::iterator it = this->Renderers.begin();
    for( int i = 0; i < this->Renderers.size(); ++i, ++it )
    {
        this->DisableForOneRenderer( i );
    }
}

void vtkMultiImagePlaneWidget::DisableForOneRenderer( int rendererIndex )
{
    // turn off the plane
    if( this->Assemblies[ rendererIndex ] )
    {
        this->Assemblies[ rendererIndex ]->RemovePart( this->PlaneOutlineActors[ rendererIndex ] );

        //turn off the texture plane
        this->Assemblies[ rendererIndex ]->RemovePart( this->TexturePlaneActors[ rendererIndex ] );

        //turn off the cursor
        this->Assemblies[ rendererIndex ]->RemovePart( this->CursorActors[ rendererIndex ] );

        //turn off the margins
        this->Assemblies[ rendererIndex ]->RemovePart( this->MarginActors[ rendererIndex ] );
    }
    else
    {
        this->Renderers[ rendererIndex ]->RemoveViewProp( this->PlaneOutlineActors[ rendererIndex ] );

        //turn off the texture plane
        this->Renderers[ rendererIndex ]->RemoveViewProp( this->TexturePlaneActors[ rendererIndex ] );

        //turn off the cursor
        this->Renderers[ rendererIndex ]->RemoveViewProp( this->CursorActors[ rendererIndex ] );

        //turn off the margins
        this->Renderers[ rendererIndex ]->RemoveViewProp( this->MarginActors[ rendererIndex ] );
    }

    //turn off the image data annotation
    this->Renderers[ rendererIndex ]->RemoveViewProp( this->TextActors[ rendererIndex ] );

    if ( this->PlanePickers[ rendererIndex ] )
    {
        this->TexturePlaneActors[ rendererIndex ]->PickableOff();
    }
}

void vtkMultiImagePlaneWidget::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    if ( this->PlaneProperty )
    {
        os << indent << "Plane Property:\n";
        this->PlaneProperty->PrintSelf(os,indent.GetNextIndent());
    }
    else
    {
        os << indent << "Plane Property: (none)\n";
    }

    if ( this->SelectedPlaneProperty )
    {
        os << indent << "Selected Plane Property:\n";
        this->SelectedPlaneProperty->PrintSelf(os,indent.GetNextIndent());
    }
    else
    {
        os << indent << "Selected Plane Property: (none)\n";
    }

    if ( this->LookupTable )
    {
        os << indent << "LookupTable:\n";
        this->LookupTable->PrintSelf(os,indent.GetNextIndent());
    }
    else
    {
        os << indent << "LookupTable: (none)\n";
    }

    if ( this->CursorProperty )
    {
        os << indent << "Cursor Property:\n";
        this->CursorProperty->PrintSelf(os,indent.GetNextIndent());
    }
    else
    {
        os << indent << "Cursor Property: (none)\n";
    }

    if ( this->MarginProperty )
    {
        os << indent << "Margin Property:\n";
        this->MarginProperty->PrintSelf(os,indent.GetNextIndent());
    }
    else
    {
        os << indent << "Margin Property: (none)\n";
    }

    if ( this->TexturePlaneProperty )
    {
        os << indent << "TexturePlane Property:\n";
        this->TexturePlaneProperty->PrintSelf(os,indent.GetNextIndent());
    }
    else
    {
        os << indent << "TexturePlane Property: (none)\n";
    }

    if ( this->ColorMap )
    {
        os << indent << "ColorMap:\n";
        this->ColorMap->PrintSelf(os,indent.GetNextIndent());
    }
    else
    {
        os << indent << "ColorMap: (none)\n";
    }

    vtkReal *o = this->PlaneSource->GetOrigin();
    vtkReal *pt1 = this->PlaneSource->GetPoint1();
    vtkReal *pt2 = this->PlaneSource->GetPoint2();

    os << indent << "Origin: (" << o[0] << ", "
    << o[1] << ", "
    << o[2] << ")\n";
    os << indent << "Point 1: (" << pt1[0] << ", "
    << pt1[1] << ", "
    << pt1[2] << ")\n";
    os << indent << "Point 2: (" << pt2[0] << ", "
    << pt2[1] << ", "
    << pt2[2] << ")\n";

    os << indent << "Plane Orientation: " << this->PlaneOrientation << "\n";
    os << indent << "Reslice Interpolate: " << this->ResliceInterpolate << "\n";
    os << indent << "Texture Interpolate: "
    << (this->TextureInterpolate ? "On\n" : "Off\n") ;
    os << indent << "Texture Visibility: "
    << (this->TextureVisibility ? "On\n" : "Off\n") ;
    os << indent << "Restrict Plane To Volume: "
    << (this->RestrictPlaneToVolume ? "On\n" : "Off\n") ;
    os << indent << "Display Text: "
    << (this->DisplayText ? "On\n" : "Off\n") ;
    os << indent << "Interaction: "
    << (this->Interaction ? "On\n" : "Off\n") ;
    os << indent << "User Controlled Lookup Table: "
    << (this->UserControlledLookupTable ? "On\n" : "Off\n") ;
    os << indent << "LeftButtonAction: " << this->LeftButtonAction << endl;
    os << indent << "MiddleButtonAction: " << this->MiddleButtonAction << endl;
    os << indent << "RightButtonAction: " << this->RightButtonAction << endl;
    os << indent << "LeftButtonAutoModifier: " <<
    this->LeftButtonAutoModifier << endl;
    os << indent << "MiddleButtonAutoModifier: " <<
    this->MiddleButtonAutoModifier << endl;
    os << indent << "RightButtonAutoModifier: " <<
    this->RightButtonAutoModifier << endl;
}

void vtkMultiImagePlaneWidget::BuildRepresentation()
{
    vtkReal *o = this->PlaneSource->GetOrigin();
    vtkReal *pt1 = this->PlaneSource->GetPoint1();
    vtkReal *pt2 = this->PlaneSource->GetPoint2();

    vtkReal x[3];
    x[0] = o[0] + (pt1[0]-o[0]) + (pt2[0]-o[0]);
    x[1] = o[1] + (pt1[1]-o[1]) + (pt2[1]-o[1]);
    x[2] = o[2] + (pt1[2]-o[2]) + (pt2[2]-o[2]);

    vtkPoints* points = this->PlaneOutlinePolyData->GetPoints();
    points->SetPoint(0,o);
    points->SetPoint(1,pt1);
    points->SetPoint(2,x);
    points->SetPoint(3,pt2);
    this->PlaneOutlinePolyData->Modified();

    this->PlaneSource->GetNormal(this->Normal);
    vtkMath::Normalize(this->Normal);
}


void vtkMultiImagePlaneWidget::HighlightPlane( int highlight )
{
    if ( highlight )
    {
        for( int i = 0; i < this->PlaneOutlineActors.size(); ++i )
        {
            this->PlaneOutlineActors[ i ]->SetProperty( this->SelectedPlaneProperty );
        }
        this->PlanePickers[ this->CurrentRendererIndex ]->GetPickPosition( this->LastPickPosition );
    }
    else
    {
        for( int i = 0; i < this->PlaneOutlineActors.size(); ++i )
        {
            this->PlaneOutlineActors[ i ]->SetProperty( this->PlaneProperty );
        }
    }
}


void vtkMultiImagePlaneWidget::OnLeftButtonDown()
{
    switch (this->LeftButtonAction)
    {
    case vtkMultiImagePlaneWidget::CURSOR_ACTION:
        this->StartCursor();
        break;
    case vtkMultiImagePlaneWidget::SLICE_MOTION_ACTION:
        this->StartSliceMotion();
        break;
    case vtkMultiImagePlaneWidget::WINDOW_LEVEL_ACTION:
        this->StartWindowLevel();
        break;
    }
}

void vtkMultiImagePlaneWidget::OnLeftButtonUp()
{
    switch (this->LeftButtonAction)
    {
    case vtkMultiImagePlaneWidget::CURSOR_ACTION:
        this->StopCursor();
        break;
    case vtkMultiImagePlaneWidget::SLICE_MOTION_ACTION:
        this->StopSliceMotion();
        break;
    case vtkMultiImagePlaneWidget::WINDOW_LEVEL_ACTION:
        this->StopWindowLevel();
        break;
    }
}

void vtkMultiImagePlaneWidget::OnMiddleButtonDown()
{
    switch (this->MiddleButtonAction)
    {
    case vtkMultiImagePlaneWidget::CURSOR_ACTION:
        this->StartCursor();
        break;
    case vtkMultiImagePlaneWidget::SLICE_MOTION_ACTION:
        this->StartSliceMotion();
        break;
    case vtkMultiImagePlaneWidget::WINDOW_LEVEL_ACTION:
        this->StartWindowLevel();
        break;
    }
}

void vtkMultiImagePlaneWidget::OnMiddleButtonUp()
{
    switch (this->MiddleButtonAction)
    {
    case vtkMultiImagePlaneWidget::CURSOR_ACTION:
        this->StopCursor();
        break;
    case vtkMultiImagePlaneWidget::SLICE_MOTION_ACTION:
        this->StopSliceMotion();
        break;
    case vtkMultiImagePlaneWidget::WINDOW_LEVEL_ACTION:
        this->StopWindowLevel();
        break;
    }
}

void vtkMultiImagePlaneWidget::OnRightButtonDown()
{
    switch (this->RightButtonAction)
    {
    case vtkMultiImagePlaneWidget::CURSOR_ACTION:
        this->StartCursor();
        break;
    case vtkMultiImagePlaneWidget::SLICE_MOTION_ACTION:
        this->StartSliceMotion();
        break;
    case vtkMultiImagePlaneWidget::WINDOW_LEVEL_ACTION:
        this->StartWindowLevel();
        break;
    }
}

void vtkMultiImagePlaneWidget::OnRightButtonUp()
{
    switch (this->RightButtonAction)
    {
    case vtkMultiImagePlaneWidget::CURSOR_ACTION:
        this->StopCursor();
        break;
    case vtkMultiImagePlaneWidget::SLICE_MOTION_ACTION:
        this->StopSliceMotion();
        break;
    case vtkMultiImagePlaneWidget::WINDOW_LEVEL_ACTION:
        this->StopWindowLevel();
        break;
    }
}

void vtkMultiImagePlaneWidget::StartCursor()
{
    // Get the position of the pick
    vtkRenderWindowInteractor * interactor = this->Interactors[ this->CurrentInteractorIndex ];
    int X = interactor->GetEventPosition()[0];
    int Y = interactor->GetEventPosition()[1];

    // Make sure we have a valid renderer to pick from
    if( this->CurrentRendererIndex == -1 )
    {
        this->State = vtkMultiImagePlaneWidget::Outside;
        return;
    }
    vtkRenderer * currentRenderer = this->Renderers[ this->CurrentRendererIndex ];

    // Okay, we can process this. If anything is picked, then we
    // can start pushing the plane.
    vtkAssemblyPath * path;
    this->PlanePickers[ this->CurrentRendererIndex ]->Pick(X,Y,0.0,currentRenderer);
    path = this->PlanePickers[ this->CurrentRendererIndex ]->GetPath();

    int found = 0;
    int i;
    if ( path != 0 )
    {
        // Deal with the possibility that we may be using a shared picker
        path->InitTraversal();
        vtkAssemblyNode * node;
        for ( i = 0; i < path->GetNumberOfItems() && !found ; i++ )
        {
            node = path->GetNextNode();
            if ( node->GetViewProp() == vtkProp::SafeDownCast( this->TexturePlaneActors[ this->CurrentRendererIndex ] ) )
            {
                found = 1;
            }
        }
    }

    if( ! found || path == 0 )
    {
        this->State = vtkMultiImagePlaneWidget::Outside;
        this->HighlightPlane(0);
        this->ActivateCursor(0);
        this->ActivateText(0);
        return;
    }
    else
    {
        this->State = vtkMultiImagePlaneWidget::Cursoring;
        this->HighlightPlane(1);
        this->ActivateCursor(1);
        this->ActivateText(1);
        this->UpdateCursor(X,Y);
        this->ManageTextDisplay();
    }

    this->Callback->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent,0);
    if (this->State != vtkMultiImagePlaneWidget::Outside)
        this->InvokeEvent(vtkCommand::UserEvent+IGNS_PLANE_SELECTED,0);

    this->RenderAll();
}

void vtkMultiImagePlaneWidget::StopCursor()
{
    if ( this->State == vtkMultiImagePlaneWidget::Outside || this->State == vtkMultiImagePlaneWidget::Start )
    {
        return;
    }

    this->State = vtkMultiImagePlaneWidget::Start;
    this->HighlightPlane(0);
    this->ActivateCursor(0);
    this->ActivateText(0);

    this->Callback->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent,0);
    this->InvokeEvent(vtkCommand::UserEvent+IGNS_MOVE_2D_POINT,0);
    this->RenderAll();
}

void vtkMultiImagePlaneWidget::StartSliceMotion()
{
    vtkRenderWindowInteractor * interactor = this->Interactors[ this->CurrentInteractorIndex ];
    int X = interactor->GetEventPosition()[0];
    int Y = interactor->GetEventPosition()[1];

    // Okay, make sure that the pick is in the current renderer
    if( this->CurrentRendererIndex == -1 )
    {
        this->State = vtkMultiImagePlaneWidget::Outside;
        return;
    }
    vtkRenderer * currentRenderer = this->Renderers[ this->CurrentRendererIndex ];

    // Okay, we can process this. If anything is picked, then we
    // can start pushing or check for adjusted states.
    vtkAssemblyPath * path;
    this->PlanePickers[ this->CurrentRendererIndex ]->Pick(X,Y,0.0,currentRenderer);
    path = this->PlanePickers[ this->CurrentRendererIndex ]->GetPath();

    int found = 0;
    int i;
    if ( path != 0 )
    {
        // Deal with the possibility that we may be using a shared picker
        path->InitTraversal();
        vtkAssemblyNode *node;
        for(i = 0; i< path->GetNumberOfItems() && !found ;i++)
        {
            node = path->GetNextNode();
            if(node->GetViewProp() == vtkProp::SafeDownCast(this->TexturePlaneActors[ this->CurrentRendererIndex ]) )
            {
                found = 1;
            }
        }
    }

    if ( !found || path == 0 )
    {
        this->State = vtkMultiImagePlaneWidget::Outside;
        this->HighlightPlane(0);
        this->ActivateMargins(0);
        return;
    }
    else
    {
        this->State = vtkMultiImagePlaneWidget::Pushing;
        this->HighlightPlane(1);
        this->ActivateMargins(1);
        this->AdjustState();
        this->UpdateMargins();
    }

    this->Callback->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent,0);
    if (this->State != vtkMultiImagePlaneWidget::Outside)
        this->InvokeEvent(vtkCommand::UserEvent+IGNS_PLANE_SELECTED,0);
    this->RenderAll();
}

void vtkMultiImagePlaneWidget::StopSliceMotion()
{
    if ( this->State == vtkMultiImagePlaneWidget::Outside || this->State == vtkMultiImagePlaneWidget::Start )
    {
        return;
    }

    this->State = vtkMultiImagePlaneWidget::Start;
    this->HighlightPlane(0);
    this->ActivateMargins(0);

    this->Callback->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent,0);
    this->RenderAll();
}

void vtkMultiImagePlaneWidget::StartWindowLevel()
{
    vtkRenderWindowInteractor * interactor = this->Interactors[ this->CurrentInteractorIndex ];
    int X = interactor->GetEventPosition()[0];
    int Y = interactor->GetEventPosition()[1];

    // Okay, make sure that the pick is in the current renderer
    if( this->CurrentRendererIndex != -1 )
    {
        this->State = vtkMultiImagePlaneWidget::Outside;
        return;
    }
    vtkRenderer * currentRenderer = this->Renderers[ this->CurrentRendererIndex ];

    // Okay, we can process this. If anything is picked, then we
    // can start window-levelling.
    vtkAssemblyPath *path;
    this->PlanePickers[ this->CurrentRendererIndex ]->Pick(X,Y,0.0,currentRenderer);
    path = this->PlanePickers[ this->CurrentRendererIndex ]->GetPath();

    int found = 0;
    int i;
    if ( path != 0 )
    {
        // Deal with the possibility that we may be using a shared picker
        path->InitTraversal();
        vtkAssemblyNode *node;
        for ( i = 0; i < path->GetNumberOfItems() && !found ; i++ )
        {
            node = path->GetNextNode();
            if ( node->GetViewProp() == vtkProp::SafeDownCast(this->TexturePlaneActors[this->CurrentRendererIndex]) )
            {
                found = 1;
            }
        }
    }

    if( ! found || path == 0 )
    {
        this->State = vtkMultiImagePlaneWidget::Outside;
        this->HighlightPlane(0);
        this->ActivateText(0);
        return;
    }
    else
    {
        this->State = vtkMultiImagePlaneWidget::WindowLevelling;
        this->HighlightPlane(1);
        this->ActivateText(1);
        this->WindowLevel(X,Y);
        this->ManageTextDisplay();
    }

    this->Callback->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent,0);
    this->RenderAll();
}

void vtkMultiImagePlaneWidget::StopWindowLevel()
{
    if ( this->State == vtkMultiImagePlaneWidget::Outside || this->State == vtkMultiImagePlaneWidget::Start )
    {
        return;
    }

    this->State = vtkMultiImagePlaneWidget::Start;
    this->HighlightPlane(0);
    this->ActivateText(0);

    this->Callback->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent,0);
    this->RenderAll();
}

void vtkMultiImagePlaneWidget::OnMouseMove()
{
    // See whether we're active
    //
    if ( this->State == vtkMultiImagePlaneWidget::Outside || this->State == vtkMultiImagePlaneWidget::Start )
    {
        return;
    }

    vtkRenderWindowInteractor * interactor = this->Interactors[ this->CurrentInteractorIndex ];
    int X = interactor->GetEventPosition()[0];
    int Y = interactor->GetEventPosition()[1];

    // Do different things depending on state
    // Calculations everybody does
    //
    vtkReal focalPoint[4], pickPoint[4], prevPickPoint[4];
    vtkReal z, vpn[3];

    vtkRenderer * currentRenderer = this->Renderers[ this->CurrentRendererIndex ];
    if (this->CurrentRendererIndex < 0)
    {
        this->State = vtkMultiImagePlaneWidget::Outside;
        return;
    }
    vtkCamera * camera = currentRenderer->GetActiveCamera();
    if ( ! camera )
    {
        return;
    }

    // Compute the two points defining the motion vector
    //
    this->ComputeWorldToDisplay( this->CurrentRendererIndex, this->LastPickPosition[0], this->LastPickPosition[1], this->LastPickPosition[2], focalPoint );
    z = focalPoint[2];

    this->ComputeDisplayToWorld( this->CurrentRendererIndex, vtkReal(interactor->GetLastEventPosition()[0]), vtkReal(interactor->GetLastEventPosition()[1]), z, prevPickPoint);

    this->ComputeDisplayToWorld( this->CurrentRendererIndex, vtkReal(X), vtkReal(Y), z, pickPoint);

    if ( this->State == vtkMultiImagePlaneWidget::WindowLevelling )
    {
        this->WindowLevel(X,Y);
        this->ManageTextDisplay();
    }
    else if ( this->State == vtkMultiImagePlaneWidget::Pushing )
    {
        this->Push(prevPickPoint, pickPoint);
        this->UpdateNormal();
        this->UpdateOrigin();
        this->UpdateMargins();
    }
    else if ( this->State == vtkMultiImagePlaneWidget::Spinning )
    {
        this->Spin(prevPickPoint, pickPoint);
        this->UpdateNormal();
        this->UpdateOrigin();
        this->UpdateMargins();
    }
    else if ( this->State == vtkMultiImagePlaneWidget::Rotating )
    {
        camera->GetViewPlaneNormal(vpn);
        this->Rotate(prevPickPoint, pickPoint, vpn);
        this->UpdateNormal();
        this->UpdateOrigin();
        this->UpdateMargins();
    }
    else if ( this->State == vtkMultiImagePlaneWidget::Scaling )
    {
        this->Scale(prevPickPoint, pickPoint, X, Y);
        this->UpdateNormal();
        this->UpdateOrigin();
        this->UpdateMargins();
    }
    else if ( this->State == vtkMultiImagePlaneWidget::Moving )
    {
        this->Translate(prevPickPoint, pickPoint);
        this->UpdateNormal();
        this->UpdateOrigin();
        this->UpdateMargins();
    }
    else if ( this->State == vtkMultiImagePlaneWidget::Cursoring )
    {
        this->UpdateCursor(X,Y);
        this->ManageTextDisplay();
    }

    // Interact, if desired
    //
    this->Callback->SetAbortFlag(1);
    this->InvokeEvent(vtkCommand::InteractionEvent,0);

    this->RenderAll();
}

void vtkMultiImagePlaneWidget::WindowLevel(int X, int Y)
{
    vtkReal range[2];
    this->LookupTable->GetTableRange(range);
    vtkReal window = range[1] - range[0];
    vtkReal level = 0.5*(range[0] + range[1]);

    vtkReal owin = this->OriginalWindow;

    vtkRenderWindowInteractor * interactor = this->Interactors[ this->CurrentInteractorIndex ];

    level = level + (X - interactor->GetLastEventPosition()[0])*owin/500.0;
    window = window + (interactor->GetLastEventPosition()[1] - Y) * owin/250.0;

    if ( window == 0.0 )
    {
        window = 0.001;
    }

    vtkReal rmin = level - window*0.5;
    vtkReal rmax = level + window*0.5;

    if( rmin < rmax )
    {
        this->CurrentWindow = window;
        this->CurrentLevel = level;
        if (!this->UserControlledLookupTable)
        {
            this->LookupTable->SetTableRange(rmin,rmax);
        }
    }
}

void vtkMultiImagePlaneWidget::SetWindowLevel(vtkReal window, vtkReal level)
{
    vtkReal rmin = level - window*0.5;
    vtkReal rmax = level + window*0.5;
    this->CurrentWindow = window;
    this->OriginalWindow  = window;
    this->CurrentLevel = level;
    this->OriginalLevel  = level;
    this->LookupTable->SetTableRange(rmin,rmax);

    if(this->Enabled)
    {
        this->RenderAll();
    }
}

void vtkMultiImagePlaneWidget::GetWindowLevel(vtkReal wl[2])
{
    vtkReal range[2];
    this->LookupTable->GetTableRange(range);
    wl[0] = range[1] - range[0];
    wl[1] = 0.5*(range[0]+range[1]);
}

int vtkMultiImagePlaneWidget::GetCursorData(vtkReal xyzv[4])
{
    if ( this->State != vtkMultiImagePlaneWidget::Cursoring  || this->CurrentImageValue == VTK_FLOAT_MAX )
    {
        return 0;
    }

    xyzv[0] = this->CurrentCursorPosition[0];
    xyzv[1] = this->CurrentCursorPosition[1];
    xyzv[2] = this->CurrentCursorPosition[2];
#ifdef VTK_REAL_IS_FLOAT
    xyzv[3] = this->ImageData->GetScalarComponentAsFloat( this->CurrentCursorPosition[0], this->CurrentCursorPosition[1], this->CurrentCursorPosition[2], 0 );
#else    
    xyzv[3] = this->ImageData->GetScalarComponentAsDouble( this->CurrentCursorPosition[0], this->CurrentCursorPosition[1], this->CurrentCursorPosition[2], 0 );
#endif    

    return 1;
}

void vtkMultiImagePlaneWidget::ManageTextDisplay()
{
    if ( !this->DisplayText )
    {
        return;
    }

    if ( this->State == vtkMultiImagePlaneWidget::WindowLevelling )
    {
        sprintf(this->TextBuff,"Window, Level: ( %g, %g )", this->CurrentWindow, this->CurrentLevel );
    }
    else if ( this->State == vtkMultiImagePlaneWidget::Cursoring )
    {
        if( this->CurrentImageValue == VTK_FLOAT_MAX )
        {
            sprintf(this->TextBuff,"Off Image");
        }
        else
        {
#ifdef VTK_REAL_IS_FLOAT
            vtkReal val = this->ImageData->GetScalarComponentAsFloat( this->CurrentCursorPosition[0], this->CurrentCursorPosition[1], this->CurrentCursorPosition[2], 0 );
#else
            vtkReal val = this->ImageData->GetScalarComponentAsDouble( this->CurrentCursorPosition[0], this->CurrentCursorPosition[1], this->CurrentCursorPosition[2], 0 );            
#endif
            sprintf(this->TextBuff,"( %3d, %3d, %3d ): %g",
                    this->CurrentCursorPosition[0],
                    this->CurrentCursorPosition[1],
                    this->CurrentCursorPosition[2],val);
        }
    }

    TextActorVec::iterator it = this->TextActors.begin();
    while ( it != this->TextActors.end() )
    {
        (*it)->SetInput(this->TextBuff);
        (*it)->Modified();
        ++it;
    }
}

void vtkMultiImagePlaneWidget::Push(vtkReal *p1, vtkReal *p2)
{
    // Get the motion vector
    //
    vtkReal v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];

    this->PlaneSource->Push( vtkMath::Dot(v,this->Normal) );
    this->PlaneSource->Update();
    this->BuildRepresentation();
}

void vtkMultiImagePlaneWidget::CreateDefaultProperties()
{
    if ( ! this->PlaneProperty )
    {
        this->PlaneProperty = vtkProperty::New();
        this->PlaneProperty->SetAmbient(1);
        this->PlaneProperty->SetColor(1,1,1);
        this->PlaneProperty->SetRepresentationToWireframe();
        this->PlaneProperty->SetInterpolationToFlat();
    }

    if ( ! this->SelectedPlaneProperty )
    {
        this->SelectedPlaneProperty = vtkProperty::New();
        this->SelectedPlaneProperty->SetAmbient(1);
        this->SelectedPlaneProperty->SetColor(0,1,0);
        this->SelectedPlaneProperty->SetRepresentationToWireframe();
        this->SelectedPlaneProperty->SetInterpolationToFlat();
    }

    if ( ! this->CursorProperty )
    {
        this->CursorProperty = vtkProperty::New();
        this->CursorProperty->SetAmbient(1);
        this->CursorProperty->SetColor(1,0,0);
        this->CursorProperty->SetRepresentationToWireframe();
        this->CursorProperty->SetInterpolationToFlat();
    }

    if ( ! this->MarginProperty )
    {
        this->MarginProperty = vtkProperty::New();
        this->MarginProperty->SetAmbient(1);
        this->MarginProperty->SetColor(0,0,1);
        this->MarginProperty->SetRepresentationToWireframe();
        this->MarginProperty->SetInterpolationToFlat();
    }

    if ( ! this->TexturePlaneProperty )
    {
        this->TexturePlaneProperty = vtkProperty::New();
        this->TexturePlaneProperty->SetAmbient(1);
        this->TexturePlaneProperty->SetInterpolationToFlat();
    }

    if( ! this->TextProperties )
    {
        this->TextProperties = vtkTextProperty::New();
        this->TextProperties->SetColor(1,1,1);
        this->TextProperties->SetFontFamilyToArial();
        this->TextProperties->SetFontSize(18);
        this->TextProperties->BoldOff();
        this->TextProperties->ItalicOff();
        this->TextProperties->ShadowOff();
        this->TextProperties->SetJustificationToLeft();
        this->TextProperties->SetVerticalJustificationToBottom();
    }
}

void vtkMultiImagePlaneWidget::PlaceWidget(vtkReal bds[6])
{
    vtkReal bounds[6], center[3];

    this->AdjustBounds(bds, bounds, center);

    if ( this->PlaneOrientation == 1 )
    {
        this->PlaneSource->SetOrigin(bounds[0],center[1],bounds[4]);
        this->PlaneSource->SetPoint1(bounds[1],center[1],bounds[4]);
        this->PlaneSource->SetPoint2(bounds[0],center[1],bounds[5]);
    }
    else if ( this->PlaneOrientation == 2 )
    {
        this->PlaneSource->SetOrigin(bounds[0],bounds[2],center[2]);
        this->PlaneSource->SetPoint1(bounds[1],bounds[2],center[2]);
        this->PlaneSource->SetPoint2(bounds[0],bounds[3],center[2]);
    }
    else //default or x-normal
    {
        this->PlaneSource->SetOrigin(center[0],bounds[2],bounds[4]);
        this->PlaneSource->SetPoint1(center[0],bounds[3],bounds[4]);
        this->PlaneSource->SetPoint2(center[0],bounds[2],bounds[5]);
    }
    this->PlaneSource->Update();
    this->BuildRepresentation();
    this->UpdateNormal();
    this->UpdateOrigin();
}

void vtkMultiImagePlaneWidget::SetPlaneOrientation(int i)
{
    // Generate a XY plane if i = 2, z-normal
    // or a YZ plane if i = 0, x-normal
    // or a ZX plane if i = 1, y-normal
    //
    this->PlaneOrientation = i;
    this->Modified();

    // This method must be called _after_ SetInput
    //
    this->ImageData = vtkImageData::SafeDownCast(this->Reslice->GetInput());
    if ( !this->ImageData )
    {
        vtkErrorMacro(<<"SetInput() before setting plane orientation.");
        return;
    }

    this->ImageData->UpdateInformation();
    int extent[6];
    this->ImageData->GetWholeExtent(extent);
    vtkReal origin[3];
    this->ImageData->GetOrigin(origin);
    vtkReal spacing[3];
    this->ImageData->GetSpacing(spacing);

    // Prevent obscuring voxels by offsetting the plane geometry
    //
    vtkReal xbounds[] = {origin[0] + spacing[0] * (extent[0] - 0.5),
                        origin[0] + spacing[0] * (extent[1] + 0.5)};
    vtkReal ybounds[] = {origin[1] + spacing[1] * (extent[2] - 0.5),
                        origin[1] + spacing[1] * (extent[3] + 0.5)};
    vtkReal zbounds[] = {origin[2] + spacing[2] * (extent[4] - 0.5),
                        origin[2] + spacing[2] * (extent[5] + 0.5)};

    if ( spacing[0] < 0.0f )
    {
        vtkReal t = xbounds[0];
        xbounds[0] = xbounds[1];
        xbounds[1] = t;
    }
    if ( spacing[1] < 0.0f )
    {
        vtkReal t = ybounds[0];
        ybounds[0] = ybounds[1];
        ybounds[1] = t;
    }
    if ( spacing[2] < 0.0f )
    {
        vtkReal t = zbounds[0];
        zbounds[0] = zbounds[1];
        zbounds[1] = t;
    }

    if ( i == 2 ) //XY, z-normal
    {
        this->PlaneSource->SetOrigin(xbounds[0],ybounds[0],zbounds[0]);
        this->PlaneSource->SetPoint1(xbounds[1],ybounds[0],zbounds[0]);
        this->PlaneSource->SetPoint2(xbounds[0],ybounds[1],zbounds[0]);
    }
    else if ( i == 0 ) //YZ, x-normal
    {
        this->PlaneSource->SetOrigin(xbounds[0],ybounds[0],zbounds[0]);
        this->PlaneSource->SetPoint1(xbounds[0],ybounds[1],zbounds[0]);
        this->PlaneSource->SetPoint2(xbounds[0],ybounds[0],zbounds[1]);
    }
    else  //ZX, y-normal
    {
        this->PlaneSource->SetOrigin(xbounds[0],ybounds[0],zbounds[0]);
        this->PlaneSource->SetPoint1(xbounds[0],ybounds[0],zbounds[1]);
        this->PlaneSource->SetPoint2(xbounds[1],ybounds[0],zbounds[0]);
    }

    this->PlaneSource->Update();
    this->BuildRepresentation();
    this->UpdateNormal();
    this->UpdateOrigin();
}

void vtkMultiImagePlaneWidget::SetInput(vtkDataSet* input)
{
    this->Superclass::SetInput(input);
    this->ImageData = vtkImageData::SafeDownCast(this->GetInput());

    if( !this->ImageData )
    {
        // If NULL is passed, remove any reference that Reslice had
        // on the old ImageData
        //
        this->Reslice->SetInput(NULL);
        return;
    }

    vtkReal range[2];
    this->ImageData->GetScalarRange(range);

    if ( !this->UserControlledLookupTable )
    {
        this->LookupTable->SetTableRange(range[0],range[1]);
        this->LookupTable->Build();
    }

    this->OriginalWindow = range[1] - range[0];
    this->OriginalLevel = 0.5*(range[0] + range[1]);

    this->Reslice->SetInput(this->ImageData);
    this->SetResliceInterpolate(this->ResliceInterpolate);

    this->ColorMap->SetInput(this->Reslice->GetOutput());

    this->Texture->SetInput(this->ColorMap->GetOutput());
    this->Texture->SetInterpolate(this->TextureInterpolate);

    this->SetPlaneOrientation(this->PlaneOrientation);
}

void vtkMultiImagePlaneWidget::UpdateOrigin()
{
    int i;

    if ( this->RestrictPlaneToVolume )
    {
        if ( !this->Reslice )
        {
            return;
        }
        this->ImageData = vtkImageData::SafeDownCast(this->Reslice->GetInput());
        if ( !this->ImageData )
        {
            return;
        }
        this->ImageData->UpdateInformation();
        vtkReal origin[3];
        this->ImageData->GetOrigin(origin);
        vtkReal spacing[3];
        this->ImageData->GetSpacing(spacing);
        int extent[6];
        this->ImageData->GetWholeExtent(extent);
        vtkReal bounds[] = {origin[0] + spacing[0]*extent[0],
                           origin[0] + spacing[0]*extent[1],
                           origin[1] + spacing[1]*extent[2],
                           origin[1] + spacing[1]*extent[3],
                           origin[2] + spacing[2]*extent[4],
                           origin[2] + spacing[2]*extent[5]};

        for ( i = 0; i <= 4; i += 2 ) // reverse bounds if necessary
        {
            if ( bounds[i] > bounds[i+1] )
            {
                vtkReal t = bounds[i+1];
                bounds[i+1] = bounds[i];
                bounds[i] = t;
            }
        }

        vtkReal abs_normal[3];
        this->PlaneSource->GetNormal(abs_normal);
        vtkReal planeCenter[3];
        this->PlaneSource->GetCenter(planeCenter);
        vtkReal nmax = 0.0f;
        int k = 0;
        for ( i = 0; i < 3; i++ )
        {
            abs_normal[i] = fabs(abs_normal[i]);
            if ( abs_normal[i]>nmax )
            {
                nmax = abs_normal[i];
                k = i;
            }
        }
        // Force the plane to lie within the true image bounds along its normal
        //
        if ( planeCenter[k] > bounds[2*k+1] )
        {
            planeCenter[k] = bounds[2*k+1];
            this->PlaneSource->SetCenter(planeCenter);
            this->PlaneSource->Update();
            this->BuildRepresentation();
        }
        else if ( planeCenter[k] < bounds[2*k] )
        {
            planeCenter[k] = bounds[2*k];
            this->PlaneSource->SetCenter(planeCenter);
            this->PlaneSource->Update();
            this->BuildRepresentation();
        }
    }

    this->ResliceAxes->DeepCopy(this->Reslice->GetResliceAxes());
    this->ResliceAxes->SetElement(0,3,0);
    this->ResliceAxes->SetElement(1,3,0);
    this->ResliceAxes->SetElement(2,3,0);

    // Transpose is an exact way to invert a pure rotation matrix
    //
    this->ResliceAxes->Transpose();

    vtkReal planeOrigin[4];
    this->PlaneSource->GetOrigin(planeOrigin);
    planeOrigin[3] = 1.0;
    vtkReal originXYZW[4];
    this->ResliceAxes->MultiplyPoint(planeOrigin,originXYZW);

    this->ResliceAxes->Transpose();
    vtkReal neworiginXYZW[4];
    vtkReal point[] =  {0.0,0.0,originXYZW[2],1.0};
    this->ResliceAxes->MultiplyPoint(point,neworiginXYZW);

    this->ResliceAxes->SetElement(0,3,neworiginXYZW[0]);
    this->ResliceAxes->SetElement(1,3,neworiginXYZW[1]);
    this->ResliceAxes->SetElement(2,3,neworiginXYZW[2]);

    this->Reslice->SetResliceAxes(this->ResliceAxes);

    vtkReal spacingXYZ[3];
    this->Reslice->GetOutputSpacing(spacingXYZ);
    this->Reslice->SetOutputOrigin(0.5*spacingXYZ[0] + originXYZW[0],
                                   0.5*spacingXYZ[1] + originXYZW[1],
                                   0.0);
}

void vtkMultiImagePlaneWidget::UpdateNormal()
{
    vtkReal planeAxis1[3];
    vtkReal planeAxis2[3];

    this->GetVector1(planeAxis1);
    this->GetVector2(planeAxis2);

    // The x,y dimensions of the plane
    //
    vtkReal planeSizeX = vtkMath::Normalize(planeAxis1);
    vtkReal planeSizeY = vtkMath::Normalize(planeAxis2);

    this->PlaneSource->GetNormal(this->Normal);

    // Generate the slicing matrix
    //
    int i;
    this->ResliceAxes->Identity();
    for ( i = 0; i < 3; i++ )
    {
        this->ResliceAxes->SetElement(i,0,planeAxis1[i]);
        this->ResliceAxes->SetElement(i,1,planeAxis2[i]);
        this->ResliceAxes->SetElement(i,2,this->Normal[i]);
    }

    // Transpose is an exact way to invert a pure rotation matrix
    //
    this->ResliceAxes->Transpose();

    vtkReal planeOrigin[4];
    this->PlaneSource->GetOrigin(planeOrigin);
    planeOrigin[3] = 1.0;
    vtkReal originXYZW[4];
    this->ResliceAxes->MultiplyPoint(planeOrigin,originXYZW);

    this->ResliceAxes->Transpose();
    vtkReal neworiginXYZW[4];
    vtkReal point[] =  {0.0,0.0,originXYZW[2],1.0};
    this->ResliceAxes->MultiplyPoint(point,neworiginXYZW);

    this->ResliceAxes->SetElement(0,3,neworiginXYZW[0]);
    this->ResliceAxes->SetElement(1,3,neworiginXYZW[1]);
    this->ResliceAxes->SetElement(2,3,neworiginXYZW[2]);

    this->Reslice->SetResliceAxes(this->ResliceAxes);

    this->ImageData = vtkImageData::SafeDownCast(this->Reslice->GetInput());
    if (!this->ImageData)
    {
        return;
    }

    // Calculate appropriate pixel spacing for the reslicing
    //
    this->ImageData->UpdateInformation();
    vtkReal spacing[3];
    this->ImageData->GetSpacing(spacing);

    vtkReal spacingX = fabs(planeAxis1[0]*spacing[0])+\
                      fabs(planeAxis1[1]*spacing[1])+\
                      fabs(planeAxis1[2]*spacing[2]);

    vtkReal spacingY = fabs(planeAxis2[0]*spacing[0])+\
                      fabs(planeAxis2[1]*spacing[1])+\
                      fabs(planeAxis2[2]*spacing[2]);


    // Pad extent up to a power of two for efficient texture mapping

    // make sure we're working with valid values
    vtkReal realExtentX;
    if (spacingX == 0)
        realExtentX = 0;
    else
        realExtentX = planeSizeX / spacingX;

    int extentX;
    // Sanity check the input data:
    // * if realExtentX is too large, extentX will wrap
    // * if spacingX is 0, things will blow up.
    // * if realExtentX is naturally 0 or < 0, the padding will yield an
    //   extentX of 1, which is also not desirable if the input data is invalid.
    if (realExtentX > (VTK_INT_MAX >> 1) || realExtentX < 1)
    {
        vtkErrorMacro(<<"Invalid X extent.  Perhaps the input data is empty?");
        extentX = 0;
    }
    else
    {
        extentX = 1;
        while (extentX < realExtentX)
        {
            extentX = extentX << 1;
        }
    }

    // make sure extentY doesn't wrap during padding
    vtkReal realExtentY;
    if (spacingY == 0)
        realExtentY = 0;
    else
        realExtentY = planeSizeY / spacingY;

    int extentY;
    if (realExtentY > (VTK_INT_MAX >> 1) || realExtentY < 1)
    {
        vtkErrorMacro(<<"Invalid Y extent.  Perhaps the input data is empty?");
        extentY = 0;
    }
    else
    {
        extentY = 1;
        while (extentY < realExtentY)
        {
            extentY = extentY << 1;
        }
    }

    this->Reslice->SetOutputSpacing(spacingX,spacingY,1);
    this->Reslice->SetOutputOrigin(0.5*spacingX + originXYZW[0],
                                   0.5*spacingY + originXYZW[1],
                                   0.0);

    if (extentX > 0 && extentY > 0) // Do not set negative extents
        this->Reslice->SetOutputExtent(0,extentX-1,0,extentY-1,0,0);
    else
        this->Reslice->SetOutputExtent(0,1,0,1,0,0);

    // Find expansion factor to account for increasing the extent
    // to a power of two
    //
    vtkReal expand1 = extentX*spacingX;
    vtkReal expand2 = extentY*spacingY;

    // Set the texture coordinates to map the image to the plane
    //
    this->TexturePlaneCoords->SetOrigin(planeOrigin[0],
                                        planeOrigin[1],planeOrigin[2]);
    this->TexturePlaneCoords->SetPoint1(planeOrigin[0] + planeAxis1[0]*expand1,
                                        planeOrigin[1] + planeAxis1[1]*expand1,
                                        planeOrigin[2] + planeAxis1[2]*expand1);
    this->TexturePlaneCoords->SetPoint2(planeOrigin[0] + planeAxis2[0]*expand2,
                                        planeOrigin[1] + planeAxis2[1]*expand2,
                                        planeOrigin[2] + planeAxis2[2]*expand2);
}

vtkImageData* vtkMultiImagePlaneWidget::GetResliceOutput()
{
    if ( ! this->Reslice )
    {
        return 0;
    }
    return this->Reslice->GetOutput();
}

void vtkMultiImagePlaneWidget::SetResliceInterpolate(int i)
{
    if ( this->ResliceInterpolate == i )
    {
        return;
    }
    this->ResliceInterpolate = i;
    this->Modified();

    if ( !this->Reslice )
    {
        return;
    }

    if ( i == VTK_NEAREST_RESLICE )
    {
        this->Reslice->SetInterpolationModeToNearestNeighbor();
    }
    else if ( i == VTK_LINEAR_RESLICE)
    {
        this->Reslice->SetInterpolationModeToLinear();
    }
    else
    {
        this->Reslice->SetInterpolationModeToCubic();
    }
    this->Texture->SetInterpolate(this->TextureInterpolate);
}

void vtkMultiImagePlaneWidget::SetPicker( int index, vtkCellPicker * picker )
{
    if( index > this->PlanePickers.size() )
    {
        vtkErrorMacro( << "index > number of renderers" << endl );
    }

    if( this->PlanePickers[ index ] != picker )
    {
        // to avoid destructor recursion
        vtkCellPicker * temp = this->PlanePickers[index];
        this->PlanePickers[ index ] = picker;
        if (temp != 0)
        {
            temp->UnRegister(this);
        }
        if (this->PlanePickers[ index ] != 0)
        {
            picker->Register( this );
            picker->SetTolerance( 0.005 ); //need some fluff
            if( this->Assemblies[ index ] != 0 )
            {
                picker->AddPickList( this->Assemblies[ index ] );
            }
            else
            {
                picker->AddPickList( this->TexturePlaneActors[ index ] );
            }
            picker->PickFromListOn();
        }
    }
}

vtkLookupTable * vtkMultiImagePlaneWidget::CreateDefaultLookupTable()
{
    vtkLookupTable* lut = vtkLookupTable::New();
    lut->Register(this);
    lut->Delete();
    lut->SetNumberOfColors( 256);
    lut->SetHueRange( 0, 0);
    lut->SetSaturationRange( 0, 0);
    lut->SetValueRange( 0 ,1 );
    lut->SetAlphaRange( 1, 1 );
    lut->Build();
    return lut;
}

void vtkMultiImagePlaneWidget::SetLookupTable(vtkLookupTable* table)
{
    if (this->LookupTable != table)
    {
        // to avoid destructor recursion
        vtkLookupTable *temp = this->LookupTable;
        this->LookupTable = table;
        if (temp != 0)
        {
            temp->UnRegister(this);
        }
        if (this->LookupTable != 0)
        {
            this->LookupTable->Register(this);
        }
        else  //create a default lut
        {
            this->LookupTable = this->CreateDefaultLookupTable();
        }
    }

    this->ColorMap->SetLookupTable(this->LookupTable);
    this->Texture->SetLookupTable(this->LookupTable);

    if( this->ImageData && !this->UserControlledLookupTable)
    {
        vtkReal range[2];
        this->ImageData->GetScalarRange(range);

        this->LookupTable->SetTableRange(range[0],range[1]);
        this->LookupTable->Build();

        this->OriginalWindow = range[1] - range[0];
        this->OriginalLevel = 0.5*(range[0] + range[1]);
    }
}

void vtkMultiImagePlaneWidget::SetSlicePosition(vtkReal position)
{
    vtkReal amount = 0.0f;
    vtkReal planeOrigin[3];
    this->PlaneSource->GetOrigin(planeOrigin);

    if ( this->PlaneOrientation == 2 ) // z axis
    {
        amount = position - planeOrigin[2];
    }
    else if ( this->PlaneOrientation == 0 ) // x axis
    {
        amount = position - planeOrigin[0];
    }
    else if ( this->PlaneOrientation == 1 )  //y axis
    {
        amount = position - planeOrigin[1];
    }
    else
    {
        vtkGenericWarningMacro("only works for ortho planes: set plane orientation first");
        return;
    }

    this->PlaneSource->Push(amount);
    this->PlaneSource->Update();
    this->BuildRepresentation();
    this->UpdateOrigin();
}

vtkReal vtkMultiImagePlaneWidget::GetSlicePosition()
{
    vtkReal planeOrigin[3];
    this->PlaneSource->GetOrigin(planeOrigin);

    if ( this->PlaneOrientation == 2 )
    {
        return planeOrigin[2];
    }
    else if ( this->PlaneOrientation == 1 )
    {
        return planeOrigin[1];
    }
    else if ( this->PlaneOrientation == 0 )
    {
        return planeOrigin[0];
    }
    else
    {
        vtkGenericWarningMacro("only works for ortho planes: set plane orientation first");
    }

    return 0.0f;
}

void vtkMultiImagePlaneWidget::SetSliceIndex(int index)
{
    if ( !this->Reslice )
    {
        return;
    }
    this->ImageData = vtkImageData::SafeDownCast(this->Reslice->GetInput());
    if ( !this->ImageData )
    {
        return;
    }
    this->ImageData->UpdateInformation();
    vtkReal origin[3];
    this->ImageData->GetOrigin(origin);
    vtkReal spacing[3];
    this->ImageData->GetSpacing(spacing);
    vtkReal planeOrigin[3];
    this->PlaneSource->GetOrigin(planeOrigin);
    vtkReal pt1[3];
    this->PlaneSource->GetPoint1(pt1);
    vtkReal pt2[3];
    this->PlaneSource->GetPoint2(pt2);

    if ( this->PlaneOrientation == 2 )
    {
        planeOrigin[2] = origin[2] + index*spacing[2];
        pt1[2] = planeOrigin[2];
        pt2[2] = planeOrigin[2];
    }
    else if ( this->PlaneOrientation == 1 )
    {
        planeOrigin[1] = origin[1] + index*spacing[1];
        pt1[1] = planeOrigin[1];
        pt2[1] = planeOrigin[1];
    }
    else if ( this->PlaneOrientation == 0 )
    {
        planeOrigin[0] = origin[0] + index*spacing[0];
        pt1[0] = planeOrigin[0];
        pt2[0] = planeOrigin[0];
    }
    else
    {
        vtkGenericWarningMacro("only works for ortho planes: set plane orientation first");
        return;
    }

    this->PlaneSource->SetOrigin(planeOrigin);
    this->PlaneSource->SetPoint1(pt1);
    this->PlaneSource->SetPoint2(pt2);
    this->PlaneSource->Update();
    this->BuildRepresentation();
    this->UpdateOrigin();
}

int vtkMultiImagePlaneWidget::GetSliceIndex()
{
    if ( ! this->Reslice )
    {
        return 0;
    }
    this->ImageData = vtkImageData::SafeDownCast(this->Reslice->GetInput());
    if ( ! this->ImageData )
    {
        return 0;
    }
    this->ImageData->UpdateInformation();
    vtkReal origin[3];
    this->ImageData->GetOrigin(origin);
    vtkReal spacing[3];
    this->ImageData->GetSpacing(spacing);
    vtkReal planeOrigin[3];
    this->PlaneSource->GetOrigin(planeOrigin);

    if ( this->PlaneOrientation == 2 )
    {
        return vtkMath::Round((planeOrigin[2]-origin[2])/spacing[2]);
    }
    else if ( this->PlaneOrientation == 1 )
    {
        return vtkMath::Round((planeOrigin[1]-origin[1])/spacing[1]);
    }
    else if ( this->PlaneOrientation == 0 )
    {
        return vtkMath::Round((planeOrigin[0]-origin[0])/spacing[0]);
    }
    else
    {
        vtkGenericWarningMacro("only works for ortho planes: set plane orientation first");
    }

    return 0;
}

void vtkMultiImagePlaneWidget::ActivateCursor( int i )
{
    if( i == 0 )
    {
        ActorVec::iterator it = this->CursorActors.begin();
        while( it != this->CursorActors.end() )
        {
            (*it)->VisibilityOff();
            ++it;
        }
    }
    else
    {
        ActorVec::iterator it = this->CursorActors.begin();
        while( it != this->CursorActors.end() )
        {
            (*it)->VisibilityOn();
            ++it;
        }
    }
}

void vtkMultiImagePlaneWidget::ActivateMargins(int i)
{
    if( i == 0 )
    {
        ActorVec::iterator it = this->MarginActors.begin();
        while( it != this->MarginActors.end() )
        {
            (*it)->VisibilityOff();
            ++it;
        }
    }
    else
    {
        ActorVec::iterator it = this->MarginActors.begin();
        while( it != this->MarginActors.end() )
        {
            (*it)->VisibilityOn();
            ++it;
        }
    }
}

void vtkMultiImagePlaneWidget::ActivateText(int i)
{
    if( i == 0 )
    {
        TextActorVec::iterator it = this->TextActors.begin();
        while( it != this->TextActors.end() )
        {
            (*it)->VisibilityOff();
            ++it;
        }
    }
    else
    {
        TextActorVec::iterator it = this->TextActors.begin();
        while( it != this->TextActors.end() )
        {
            (*it)->VisibilityOn();
            ++it;
        }
    }
}

void vtkMultiImagePlaneWidget::UpdateCursor( int X, int Y )
{
    if( this->CurrentRendererIndex == -1 )
    {
        return;
    }

    vtkRenderer * renderer = this->Renderers[ this->CurrentRendererIndex ];
    vtkAssemblyPath *path;
    this->PlanePickers[this->CurrentRendererIndex]->Pick(X,Y,0.0,renderer);
    path = this->PlanePickers[ this->CurrentRendererIndex ]->GetPath();
    this->CurrentImageValue = VTK_DOUBLE_MAX;

    int found = 0;
    int i;
    if ( path != 0 )
    {
        // Deal with the possibility that we may be using a shared picker
        //
        path->InitTraversal();
        vtkAssemblyNode *node;
        for ( i = 0; i< path->GetNumberOfItems() && !found ; i++ )
        {
            node = path->GetNextNode();
            if ( node->GetViewProp() == vtkProp::SafeDownCast( this->TexturePlaneActors[ this->CurrentRendererIndex ]) )
            {
                found = 1;
            }
        }
    }

    if( !found || path == 0 )
    {
        ActorVec::iterator it = this->CursorActors.begin();
        while( it != this->CursorActors.end() )
        {
            (*it)->VisibilityOff();
            ++it;
        }
        return;
    }
    else
    {
        ActorVec::iterator it = this->CursorActors.begin();
        while( it != this->CursorActors.end() )
        {
            (*it)->VisibilityOn();
            ++it;
        }
    }

    vtkReal q[3];
    this->PlanePickers[ this->CurrentRendererIndex ]->GetPickPosition(q);
    memcpy(this->CurrentWorldPosition,q,3*sizeof(vtkReal));
    
    // Transform the pick position in the image space
    vtkReal qTransformed[3] = { 0, 0, 0 };
    this->TransformToImageSpace( q, qTransformed );

    // vtkImageData will find the nearest implicit point to q
    //
    vtkIdType ptId = this->ImageData->FindPoint(qTransformed);

    if ( ptId == -1 )
    {
        ActorVec::iterator it = this->CursorActors.begin();
        while( it != this->CursorActors.end() )
        {
            (*it)->VisibilityOff();
            ++it;
        }
        return;
    }

    vtkReal closestPt[3];
    this->ImageData->GetPoint(ptId,closestPt);

    vtkReal origin[3];
    this->ImageData->GetOrigin(origin);
    vtkReal spacing[3];
    this->ImageData->GetSpacing(spacing);
    int extent[6];
    this->ImageData->GetExtent(extent);

    vtkReal o[3];
    this->PlaneSource->GetOrigin(o);

    int iq[3];
    int iqtemp;
    vtkReal qro[3];
    for (i = 0; i < 3; i++)
    {
        // compute world to image coords
        iqtemp = vtkMath::Round((closestPt[i]-origin[i])/spacing[i]);

        // we have a valid pick already, just enforce bounds check
        iq[i] =
            (iqtemp < extent[2*i])?extent[2*i]:((iqtemp > extent[2*i+1])?extent[2*i+1]:iqtemp);

        // compute image to world coords
        q[i] = iq[i]*spacing[i] + origin[i];

        // q relative to the plane origin
        qro[i]= q[i] - o[i];
    }

    memcpy(this->CurrentCursorPosition,iq,3*sizeof(int));
    this->CurrentImageValue = 0.0;

    vtkReal p1o[3];
    vtkReal p2o[3];

    this->GetVector1(p1o);
    this->GetVector2(p2o);

    vtkReal Lp1 = vtkMath::Dot(qro,p1o)/vtkMath::Dot(p1o,p1o);
    vtkReal Lp2 = vtkMath::Dot(qro,p2o)/vtkMath::Dot(p2o,p2o);

    vtkReal p1[3];
    this->PlaneSource->GetPoint1(p1);
    vtkReal p2[3];
    this->PlaneSource->GetPoint2(p2);

    vtkReal a[3];
    vtkReal b[3];
    vtkReal c[3];
    vtkReal d[3];

    for (i = 0; i < 3; i++)
    {
        a[i] = o[i]  + Lp2*p2o[i];   // left
        b[i] = p1[i] + Lp2*p2o[i];   // right
        c[i] = o[i]  + Lp1*p1o[i];   // bottom
        d[i] = p2[i] + Lp1*p1o[i];   // top
    }

    vtkPoints* cursorPts = this->CursorPolyData->GetPoints();

    cursorPts->SetPoint(0,a);
    cursorPts->SetPoint(1,b);
    cursorPts->SetPoint(2,c);
    cursorPts->SetPoint(3,d);

    this->CursorPolyData->Modified();
}

void vtkMultiImagePlaneWidget::SetOrigin(vtkReal x, vtkReal y, vtkReal z)
{
    this->PlaneSource->SetOrigin(x,y,z);
}

void vtkMultiImagePlaneWidget::SetOrigin(vtkReal xyz[3])
{
    this->PlaneSource->SetOrigin(xyz);
}

vtkReal* vtkMultiImagePlaneWidget::GetOrigin()
{
    return this->PlaneSource->GetOrigin();
}

void vtkMultiImagePlaneWidget::GetOrigin(vtkReal xyz[3])
{
    this->PlaneSource->GetOrigin(xyz);
}

void vtkMultiImagePlaneWidget::SetPoint1(vtkReal x, vtkReal y, vtkReal z)
{
    this->PlaneSource->SetPoint1(x,y,z);
}

void vtkMultiImagePlaneWidget::SetPoint1(vtkReal xyz[3])
{
    this->PlaneSource->SetPoint1(xyz);
}

vtkReal* vtkMultiImagePlaneWidget::GetPoint1()
{
    return this->PlaneSource->GetPoint1();
}

void vtkMultiImagePlaneWidget::GetPoint1(vtkReal xyz[3])
{
    this->PlaneSource->GetPoint1(xyz);
}

void vtkMultiImagePlaneWidget::SetPoint2(vtkReal x, vtkReal y, vtkReal z)
{
    this->PlaneSource->SetPoint2(x,y,z);
}

void vtkMultiImagePlaneWidget::SetPoint2(vtkReal xyz[3])
{
    this->PlaneSource->SetPoint2(xyz);
}

vtkReal* vtkMultiImagePlaneWidget::GetPoint2()
{
    return this->PlaneSource->GetPoint2();
}

void vtkMultiImagePlaneWidget::GetPoint2(vtkReal xyz[3])
{
    this->PlaneSource->GetPoint2(xyz);
}

vtkReal* vtkMultiImagePlaneWidget::GetCenter()
{
    return this->PlaneSource->GetCenter();
}

void vtkMultiImagePlaneWidget::GetCenter(vtkReal xyz[3])
{
    this->PlaneSource->GetCenter(xyz);
}

vtkReal* vtkMultiImagePlaneWidget::GetNormal()
{
    return this->PlaneSource->GetNormal();
}

void vtkMultiImagePlaneWidget::GetNormal(vtkReal xyz[3])
{
    this->PlaneSource->GetNormal(xyz);
}

void vtkMultiImagePlaneWidget::GetPolyData(vtkPolyData *pd)
{
    pd->ShallowCopy(this->PlaneSource->GetOutput());
}

void vtkMultiImagePlaneWidget::UpdatePlacement(void)
{
    this->PlaneSource->Update();
    this->BuildRepresentation();
    this->UpdateNormal();
    this->UpdateOrigin();
    this->UpdateMargins();
}

void vtkMultiImagePlaneWidget::SetTextProperty(vtkTextProperty* tprop)
{
    if( tprop && tprop != this->TextProperties )
    {
        this->TextProperties = tprop;
        if( this->GetEnabled() )
        {
            TextActorVec::iterator it = this->TextActors.begin();
            while( it != this->TextActors.end() )
            {
                (*it)->SetTextProperty( tprop );
                ++it;
            }
        }
    }
}

vtkTextProperty * vtkMultiImagePlaneWidget::GetTextProperty()
{
    return this->TextProperties;
}

vtkTexture* vtkMultiImagePlaneWidget::GetTexture()
{
    return this->Texture;
}

void vtkMultiImagePlaneWidget::GetVector1(vtkReal v1[3])
{
    vtkReal* p1 = this->PlaneSource->GetPoint1();
    vtkReal* o =  this->PlaneSource->GetOrigin();
    v1[0] = p1[0] - o[0];
    v1[1] = p1[1] - o[1];
    v1[2] = p1[2] - o[2];
}

void vtkMultiImagePlaneWidget::GetVector2(vtkReal v2[3])
{
    vtkReal* p2 = this->PlaneSource->GetPoint2();
    vtkReal* o =  this->PlaneSource->GetOrigin();
    v2[0] = p2[0] - o[0];
    v2[1] = p2[1] - o[1];
    v2[2] = p2[2] - o[2];
}

void vtkMultiImagePlaneWidget::AdjustState()
{
    int *auto_modifier = NULL;
    switch (this->LastButtonPressed)
    {
    case vtkMultiImagePlaneWidget::LEFT_BUTTON:
        auto_modifier = &this->LeftButtonAutoModifier;
        break;
    case vtkMultiImagePlaneWidget::MIDDLE_BUTTON:
        auto_modifier = &this->MiddleButtonAutoModifier;
        break;
    case vtkMultiImagePlaneWidget::RIGHT_BUTTON:
        auto_modifier = &this->RightButtonAutoModifier;
        break;
    }

    if (this->Interactors[ this->CurrentInteractorIndex ]->GetShiftKey() ||
            (auto_modifier &&
             (*auto_modifier & vtkMultiImagePlaneWidget::SHIFT_MODIFIER)))
    {
        this->State = vtkMultiImagePlaneWidget::Scaling;
        return;
    }

    vtkReal v1[3];
    this->GetVector1(v1);
    vtkReal v2[3];
    this->GetVector2(v2);
    vtkReal planeSize1 = vtkMath::Normalize(v1);
    vtkReal planeSize2 = vtkMath::Normalize(v2);
    
    // Transform the last pick position in the space
    // of the widgets.
    vtkReal lastPickPositionTrans[ 3 ] = { 0, 0, 0 };
    this->TransformToImageSpace( this->LastPickPosition, lastPickPositionTrans );

    vtkReal * planeOrigin = this->PlaneSource->GetOrigin();
    
    vtkReal ppo[3] = {lastPickPositionTrans[0] - planeOrigin[0],
                     lastPickPositionTrans[1] - planeOrigin[1],
                     lastPickPositionTrans[2] - planeOrigin[2] };

    vtkReal x2D = vtkMath::Dot(ppo,v1);
    vtkReal y2D = vtkMath::Dot(ppo,v2);

    // Divide plane into three zones for different user interactions:
    // four corners -- spin around the plane's normal at its center
    // four edges   -- rotate around one of the plane's axes at its center
    // center area  -- push
    //
    vtkReal marginX = planeSize1 * 0.05;
    vtkReal marginY = planeSize2 * 0.05;

    vtkReal x0 = marginX;
    vtkReal y0 = marginY;
    vtkReal x1 = planeSize1 - marginX;
    vtkReal y1 = planeSize2 - marginY;

    if ( x2D < x0  )       // left margin
    {
        if (y2D < y0)        // bottom left corner
        {
            this->MarginSelectMode =  0;
        }
        else if (y2D > y1)   // top left corner
        {
            this->MarginSelectMode =  3;
        }
        else                 // left edge
        {
            this->MarginSelectMode =  4;
        }
    }
    else if ( x2D > x1 )   // right margin
    {
        if (y2D < y0)        // bottom right corner
        {
            this->MarginSelectMode =  1;
        }
        else if (y2D > y1)   // top right corner
        {
            this->MarginSelectMode =  2;
        }
        else                 // right edge
        {
            this->MarginSelectMode =  5;
        }
    }
    else                   // middle
    {
        if (y2D < y0)        // bottom edge
        {
            this->MarginSelectMode =  6;
        }
        else if (y2D > y1)   // top edge
        {
            this->MarginSelectMode =  7;
        }
        else                 // central area
        {
            this->MarginSelectMode =  8;
        }
    }

    if (this->Interactors[ this->CurrentInteractorIndex ]->GetControlKey() ||
            (auto_modifier &&
             (*auto_modifier & vtkMultiImagePlaneWidget::CONTROL_MODIFIER)))
    {
        this->State = vtkMultiImagePlaneWidget::Moving;
    }
    else
    {
        if (this->MarginSelectMode >= 0 && this->MarginSelectMode < 4)
        {
            this->State = vtkMultiImagePlaneWidget::Spinning;
            return;
        }
        else if (this->MarginSelectMode == 8)
        {
            this->State = vtkMultiImagePlaneWidget::Pushing;
            return;
        }
        else
        {
            this->State = vtkMultiImagePlaneWidget::Rotating;
        }
    }

    vtkReal *raPtr = 0;
    vtkReal *rvPtr = 0;
    vtkReal rvfac = 1.0;
    vtkReal rafac = 1.0;

    switch ( this->MarginSelectMode )
    {
        // left bottom corner
    case 0:
        raPtr = v2;
        rvPtr = v1;
        rvfac = -1.0;
        rafac = -1.0;
        break;
        // right bottom corner
    case 1:
        raPtr = v2;
        rvPtr = v1;
        rafac = -1.0;
        break;
        // right top corner
    case 2:
        raPtr = v2;
        rvPtr = v1;
        break;
        // left top corner
    case 3:
        raPtr = v2;
        rvPtr = v1;
        rvfac = -1.0;
        break;
    case 4:
        raPtr = v2;
        rvPtr = v1;
        rvfac = -1.0;
        break; // left
    case 5:
        raPtr = v2;
        rvPtr = v1;
        break; // right
    case 6:
        raPtr = v1;
        rvPtr = v2;
        rvfac = -1.0;
        break; // bottom
    case 7:
        raPtr = v1;
        rvPtr = v2;
        break; // top
    default:
        raPtr = v1;
        rvPtr = v2;
        break;
    }

    for (int i = 0; i < 3; i++)
    {
        this->RotateAxis[i] = *raPtr++ * rafac;
        this->RadiusVector[i] = *rvPtr++ * rvfac;
    }
}

void vtkMultiImagePlaneWidget::Spin(vtkReal *p1, vtkReal *p2)
{
    // Disable cursor snap
    //
    this->PlaneOrientation = 3;

    // Get the motion vector, in world coords
    //
    vtkReal v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];

    // Plane center and normal before transform
    //
    vtkReal* wc = this->PlaneSource->GetCenter();
    vtkReal* wn = this->Normal;

    // Radius vector from center to cursor position
    //
    vtkReal rv[3] = {p2[0]-wc[0], p2[1]-wc[1], p2[2]-wc[2]};

    // Distance between center and cursor location
    //
    vtkReal rs = vtkMath::Normalize(rv);

    // Spin direction
    //
    vtkReal wn_cross_rv[3];
    vtkMath::Cross(wn,rv,wn_cross_rv);

    // Spin angle
    //
#if ((VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION < 4))
    vtkReal dw = vtkMath::RadiansToDegrees() * vtkMath::Dot(v,wn_cross_rv) / rs;
#else    
    vtkReal dw = vtkMath::DegreesFromRadians(vtkMath::Dot(v,wn_cross_rv) / rs);
#endif
    this->Transform->Identity();
    this->Transform->Translate(wc[0],wc[1],wc[2]);
    this->Transform->RotateWXYZ(dw,wn);
    this->Transform->Translate(-wc[0],-wc[1],-wc[2]);

    vtkReal newpt[3];
    this->Transform->TransformPoint(this->PlaneSource->GetPoint1(),newpt);
    this->PlaneSource->SetPoint1(newpt);
    this->Transform->TransformPoint(this->PlaneSource->GetPoint2(),newpt);
    this->PlaneSource->SetPoint2(newpt);
    this->Transform->TransformPoint(this->PlaneSource->GetOrigin(),newpt);
    this->PlaneSource->SetOrigin(newpt);

    this->PlaneSource->Update();
    this->BuildRepresentation();
}

void vtkMultiImagePlaneWidget::Rotate(vtkReal *p1, vtkReal *p2, vtkReal *vpn)
{
    // Disable cursor snap
    //
    this->PlaneOrientation = 3;

    // Get the motion vector, in world coords
    //
    vtkReal v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];

    // Plane center and normal
    //
    vtkReal* wc = this->PlaneSource->GetCenter();

    // Radius of the rotating circle of the picked point
    //
    vtkReal radius = fabs( this->RadiusVector[0]*(p2[0]-wc[0]) +
                          this->RadiusVector[1]*(p2[1]-wc[1]) +
                          this->RadiusVector[2]*(p2[2]-wc[2]) );

    // Rotate direction ra_cross_rv
    //
    vtkReal rd[3];
    vtkMath::Cross(this->RotateAxis,this->RadiusVector,rd);

    // Direction cosin between rotating direction and view normal
    //
    vtkReal rd_dot_vpn = rd[0]*vpn[0] + rd[1]*vpn[1] + rd[2]*vpn[2];

    // 'push' plane edge when mouse moves away from plane center
    // 'pull' plane edge when mouse moves toward plane center
    //
#if ((VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION < 4))
    vtkReal dw = vtkMath::RadiansToDegrees() * (vtkMath::Dot(this->RadiusVector,v))/radius * (-rd_dot_vpn);
#else
    vtkReal dw = vtkMath::DegreesFromRadians((vtkMath::Dot(this->RadiusVector,v))/radius * (-rd_dot_vpn));
#endif
    this->Transform->Identity();
    this->Transform->Translate(wc[0],wc[1],wc[2]);
    this->Transform->RotateWXYZ(dw,this->RotateAxis);
    this->Transform->Translate(-wc[0],-wc[1],-wc[2]);

    vtkReal newpt[3];
    this->Transform->TransformPoint(this->PlaneSource->GetPoint1(),newpt);
    this->PlaneSource->SetPoint1(newpt);
    this->Transform->TransformPoint(this->PlaneSource->GetPoint2(),newpt);
    this->PlaneSource->SetPoint2(newpt);
    this->Transform->TransformPoint(this->PlaneSource->GetOrigin(),newpt);
    this->PlaneSource->SetOrigin(newpt);

    this->PlaneSource->Update();
    this->BuildRepresentation();
}

void vtkMultiImagePlaneWidget::GeneratePlaneOutline()
{
    vtkPoints* points   = vtkPoints::New(VTK_DOUBLE);
    points->SetNumberOfPoints(4);
    int i;
    for (i = 0; i < 4; i++)
    {
        points->SetPoint(i,0.0,0.0,0.0);
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->Allocate(cells->EstimateSize(4,2));
    vtkIdType pts[2];
    pts[0] = 3;
    pts[1] = 2;       // top edge
    cells->InsertNextCell(2,pts);
    pts[0] = 0;
    pts[1] = 1;       // bottom edge
    cells->InsertNextCell(2,pts);
    pts[0] = 0;
    pts[1] = 3;       // left edge
    cells->InsertNextCell(2,pts);
    pts[0] = 1;
    pts[1] = 2;       // right edge
    cells->InsertNextCell(2,pts);

    this->PlaneOutlinePolyData->SetPoints(points);
    points->Delete();
    this->PlaneOutlinePolyData->SetLines(cells);
    cells->Delete();
}

void vtkMultiImagePlaneWidget::GenerateTexturePlane()
{
    this->SetResliceInterpolate(this->ResliceInterpolate);

    this->LookupTable = this->CreateDefaultLookupTable();

    this->ColorMap->SetLookupTable(this->LookupTable);
    this->ColorMap->SetOutputFormatToRGBA();
    this->ColorMap->PassAlphaToOutputOn();

    this->TexturePlaneCoords->SetInput(this->PlaneSource->GetOutput());
    this->TexturePlaneCoords->AutomaticPlaneGenerationOff();

    this->Texture->SetQualityTo32Bit();
    this->Texture->MapColorScalarsThroughLookupTableOff();
    this->Texture->SetInterpolate(this->TextureInterpolate);
    this->Texture->RepeatOff();
    this->Texture->SetLookupTable(this->LookupTable);
}

void vtkMultiImagePlaneWidget::GenerateMargins()
{
    // Construct initial points
    vtkPoints* points = vtkPoints::New(VTK_DOUBLE);
    points->SetNumberOfPoints(8);
    int i;
    for (i = 0; i < 8; i++)
    {
        points->SetPoint(i,0.0,0.0,0.0);
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->Allocate(cells->EstimateSize(4,2));
    vtkIdType pts[2];
    pts[0] = 0;
    pts[1] = 1;       // top margin
    cells->InsertNextCell(2,pts);
    pts[0] = 2;
    pts[1] = 3;       // bottom margin
    cells->InsertNextCell(2,pts);
    pts[0] = 4;
    pts[1] = 5;       // left margin
    cells->InsertNextCell(2,pts);
    pts[0] = 6;
    pts[1] = 7;       // right margin
    cells->InsertNextCell(2,pts);

    this->MarginPolyData->SetPoints(points);
    points->Delete();
    this->MarginPolyData->SetLines(cells);
    cells->Delete();
}

void vtkMultiImagePlaneWidget::GenerateCursor()
{
    // Construct initial points
    //
    vtkPoints* points = vtkPoints::New(VTK_DOUBLE);
    points->SetNumberOfPoints(4);
    int i;
    for (i = 0; i < 4; i++)
    {
        points->SetPoint(i,0.0,0.0,0.0);
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->Allocate(cells->EstimateSize(2,2));
    vtkIdType pts[2];
    pts[0] = 0;
    pts[1] = 1;       // horizontal segment
    cells->InsertNextCell(2,pts);
    pts[0] = 2;
    pts[1] = 3;       // vertical segment
    cells->InsertNextCell(2,pts);

    this->CursorPolyData->SetPoints(points);
    points->Delete();
    this->CursorPolyData->SetLines(cells);
    cells->Delete();
}


void vtkMultiImagePlaneWidget::UpdateMargins()
{
    vtkReal v1[3];
    this->GetVector1(v1);
    vtkReal v2[3];
    this->GetVector2(v2);
    vtkReal o[3];
    this->PlaneSource->GetOrigin(o);
    vtkReal p1[3];
    this->PlaneSource->GetPoint1(p1);
    vtkReal p2[3];
    this->PlaneSource->GetPoint2(p2);

    vtkReal a[3];
    vtkReal b[3];
    vtkReal c[3];
    vtkReal d[3];

    vtkReal s = 0.05;
    vtkReal t = 0.05;

    int i;
    for ( i = 0; i < 3; i++)
    {
        a[i] = o[i] + v2[i]*(1-t);
        b[i] = p1[i] + v2[i]*(1-t);
        c[i] = o[i] + v2[i]*t;
        d[i] = p1[i] + v2[i]*t;
    }

    vtkPoints* marginPts = this->MarginPolyData->GetPoints();

    marginPts->SetPoint(0,a);
    marginPts->SetPoint(1,b);
    marginPts->SetPoint(2,c);
    marginPts->SetPoint(3,d);

    for ( i = 0; i < 3; i++)
    {
        a[i] = o[i] + v1[i]*s;
        b[i] = p2[i] + v1[i]*s;
        c[i] = o[i] + v1[i]*(1-s);
        d[i] = p2[i] + v1[i]*(1-s);
    }

    marginPts->SetPoint(4,a);
    marginPts->SetPoint(5,b);
    marginPts->SetPoint(6,c);
    marginPts->SetPoint(7,d);

    this->MarginPolyData->Modified();
}

void vtkMultiImagePlaneWidget::Translate(vtkReal *p1, vtkReal *p2)
{
    // Get the motion vector
    //
    vtkReal v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];

    vtkReal *o = this->PlaneSource->GetOrigin();
    vtkReal *pt1 = this->PlaneSource->GetPoint1();
    vtkReal *pt2 = this->PlaneSource->GetPoint2();
    vtkReal origin[3], point1[3], point2[3];

    vtkReal vdrv = this->RadiusVector[0]*v[0] + \
                  this->RadiusVector[1]*v[1] + \
                  this->RadiusVector[2]*v[2];
    vtkReal vdra = this->RotateAxis[0]*v[0] + \
                  this->RotateAxis[1]*v[1] + \
                  this->RotateAxis[2]*v[2];

    int i;
    if ( this->MarginSelectMode == 8 )       // everybody comes along
    {
        for (i=0; i<3; i++)
        {
            origin[i] = o[i] + v[i];
            point1[i] = pt1[i] + v[i];
            point2[i] = pt2[i] + v[i];
        }
        this->PlaneSource->SetOrigin(origin);
        this->PlaneSource->SetPoint1(point1);
        this->PlaneSource->SetPoint2(point2);
    }
    else if ( this->MarginSelectMode == 4 ) // left edge
    {
        for (i=0; i<3; i++)
        {
            origin[i] = o[i]   + vdrv*this->RadiusVector[i];
            point2[i] = pt2[i] + vdrv*this->RadiusVector[i];
        }
        this->PlaneSource->SetOrigin(origin);
        this->PlaneSource->SetPoint2(point2);
    }
    else if ( this->MarginSelectMode == 5 ) // right edge
    {
        for (i=0; i<3; i++)
        {
            point1[i] = pt1[i] + vdrv*this->RadiusVector[i];
        }
        this->PlaneSource->SetPoint1(point1);
    }
    else if ( this->MarginSelectMode == 6 ) // bottom edge
    {
        for (i=0; i<3; i++)
        {
            origin[i] = o[i]   + vdrv*this->RadiusVector[i];
            point1[i] = pt1[i] + vdrv*this->RadiusVector[i];
        }
        this->PlaneSource->SetOrigin(origin);
        this->PlaneSource->SetPoint1(point1);
    }
    else if ( this->MarginSelectMode == 7 ) // top edge
    {
        for (i=0; i<3; i++)
        {
            point2[i] = pt2[i] + vdrv*this->RadiusVector[i];
        }
        this->PlaneSource->SetPoint2(point2);
    }
    else if ( this->MarginSelectMode == 3 ) // top left corner
    {
        for (i=0; i<3; i++)
        {
            origin[i] = o[i]   + vdrv*this->RadiusVector[i];
            point2[i] = pt2[i] + vdrv*this->RadiusVector[i] +
                        vdra*this->RotateAxis[i];
        }
        this->PlaneSource->SetOrigin(origin);
        this->PlaneSource->SetPoint2(point2);
    }
    else if ( this->MarginSelectMode == 0 ) // bottom left corner
    {
        for (i=0; i<3; i++)
        {
            origin[i] = o[i]   + vdrv*this->RadiusVector[i] +
                        vdra*this->RotateAxis[i];
            point1[i] = pt1[i] + vdra*this->RotateAxis[i];
            point2[i] = pt2[i] + vdrv*this->RadiusVector[i];
        }
        this->PlaneSource->SetOrigin(origin);
        this->PlaneSource->SetPoint1(point1);
        this->PlaneSource->SetPoint2(point2);
    }
    else if ( this->MarginSelectMode == 2 ) // top right corner
    {
        for (i=0; i<3; i++)
        {
            point1[i] = pt1[i] + vdrv*this->RadiusVector[i];
            point2[i] = pt2[i] + vdra*this->RotateAxis[i];
        }
        this->PlaneSource->SetPoint1(point1);
        this->PlaneSource->SetPoint2(point2);
    }
    else                                   // bottom right corner
    {
        for (i=0; i<3; i++)
        {
            origin[i] = o[i]   + vdra*this->RotateAxis[i];
            point1[i] = pt1[i] + vdrv*this->RadiusVector[i] +
                        vdra*this->RotateAxis[i];
        }
        this->PlaneSource->SetPoint1(point1);
        this->PlaneSource->SetOrigin(origin);
    }

    this->PlaneSource->Update();
    this->BuildRepresentation();
}

void vtkMultiImagePlaneWidget::Scale(vtkReal *p1, vtkReal *p2, int vtkNotUsed(X), int Y)
{
    // Get the motion vector
    //
    vtkReal v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];

    vtkReal *o = this->PlaneSource->GetOrigin();
    vtkReal *pt1 = this->PlaneSource->GetPoint1();
    vtkReal *pt2 = this->PlaneSource->GetPoint2();

    vtkReal center[3];
    center[0] = o[0] + (pt1[0]-o[0])/2.0 + (pt2[0]-o[0])/2.0;
    center[1] = o[1] + (pt1[1]-o[1])/2.0 + (pt2[1]-o[1])/2.0;
    center[2] = o[2] + (pt1[2]-o[2])/2.0 + (pt2[2]-o[2])/2.0;

    // Compute the scale factor
    //
    vtkReal sf = vtkMath::Norm(v) /
                sqrt(vtkMath::Distance2BetweenPoints(pt1,pt2));
    if ( Y > this->Interactors[ this->CurrentInteractorIndex ]->GetLastEventPosition()[1] )
    {
        sf = 1.0 + sf;
    }
    else
    {
        sf = 1.0 - sf;
    }

    // Move the corner points
    //
    vtkReal origin[3], point1[3], point2[3];

    for (int i=0; i<3; i++)
    {
        origin[i] = sf * (o[i] - center[i]) + center[i];
        point1[i] = sf * (pt1[i] - center[i]) + center[i];
        point2[i] = sf * (pt2[i] - center[i]) + center[i];
    }

    this->PlaneSource->SetOrigin(origin);
    this->PlaneSource->SetPoint1(point1);
    this->PlaneSource->SetPoint2(point2);
    this->PlaneSource->Update();
    this->BuildRepresentation();
}


void vtkMultiImagePlaneWidget::TransformToImageSpace( vtkReal * in, vtkReal * out )
{
    vtkAssemblyPath * path = this->PlanePickers[ this->CurrentRendererIndex ]->GetPath();
    vtkMatrix4x4 * pathMatrix = path->GetLastNode()->GetMatrix();
    vtkMatrix4x4 * invertMatrix = vtkMatrix4x4::New();
    vtkMatrix4x4::Invert( pathMatrix, invertMatrix );
    vtkReal untransformed[ 4 ] = { 0, 0, 0, 1 };
    untransformed[ 0 ] = in[0];
    untransformed[ 1 ] = in[1];
    untransformed[ 2 ] = in[2];
    vtkReal transformed[ 4 ] = { 0, 0, 0, 1 };
    invertMatrix->MultiplyPoint( untransformed, transformed );
    out[ 0 ] = transformed[ 0 ];
    out[ 1 ] = transformed[ 1 ];
    out[ 2 ] = transformed[ 2 ];
}

vtkActor * vtkMultiImagePlaneWidget::GetTexturePlaneActor(int index)
{
    if (index < this->TexturePlaneActors.size())
        return this->TexturePlaneActors[index];
    return 0;
}

void vtkMultiImagePlaneWidget::GetCurrentWorldPosition( vtkReal* pos)
{
    if (pos)
        memcpy(pos, this->CurrentWorldPosition,3*sizeof(vtkReal)); 
}

void vtkMultiImagePlaneWidget::ExternalUpdateCursor(int xy[2], vtkRenderer *ren)
{
    RendererVec::iterator itRen = this->Renderers.begin();
    int index = 0;
    this->CurrentRendererIndex = -1;
    while( itRen != this->Renderers.end() )
    {
        if( (*itRen) == ren )
        {
            this->CurrentRendererIndex = index;
            break;
        }
        ++itRen;
    }
    UpdateCursor(xy[0], xy[1]);
}

void vtkMultiImagePlaneWidget::ExternalActivateCursor(int active)
{
    this->ActivateCursor(active);
}


#include "view.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyle.h"
#include "vtkInteractorStyleTerrain.h"
#include "vtkInteractorStyleImage.h"
#include "vtkCellPicker.h"
#include "scenemanager.h"
#include "sceneobject.h"


vtkCxxSetObjectMacro(View,Renderer,vtkRenderer);
vtkCxxSetObjectMacro(View,InteractorStyle,vtkInteractorStyle);
vtkCxxSetObjectMacro(View,Picker,vtkCellPicker);
vtkCxxSetObjectMacro(View,Manager,SceneManager);


const char DefaultViewNames[4][20] = { "Sagittal\0", "Coronal\0", "Transverse\0", "3D view\0" };


View::View() 
{
    this->Name = "";
    this->SetName( DefaultViewNames[THREED_VIEW_TYPE] );
    this->Interactor = 0;
    this->Renderer = vtkRenderer::New();
    this->Renderer->GetActiveCamera()->ParallelProjectionOn();  // by default, we want parallele projection.
    this->InteractorStyle = vtkInteractorStyleTerrain::New();
    this->Picker = vtkCellPicker::New();
    this->Type = THREED_VIEW_TYPE;
    this->Renderer->GetActiveCamera()->SetPosition( 1, 0, 0 );
    this->Renderer->GetActiveCamera()->SetFocalPoint( 0, 0, 0 );
    this->Renderer->GetActiveCamera()->SetViewUp( 0, 0, 1 );
    this->Manager = 0;
}


View::~View()
{
    if( this->Name )
        delete [] this->Name;
    if( this->Renderer )
        this->Renderer->Delete();
    if( this->Picker )
        this->Picker->Delete();
    if( this->Manager )
        this->Manager->Delete();
}


void View::SetType( int type )
{
    if( this->Type == type )
    {
        return;
    }
    
    this->Type = type;
    switch( type )
    {
        case SAGITTAL_VIEW_TYPE:
            this->Renderer->GetActiveCamera()->SetPosition( -1, 0, 0 ); // nose to the left
            this->Renderer->GetActiveCamera()->SetFocalPoint( 0, 0, 0 );
            this->Renderer->GetActiveCamera()->SetViewUp( 0, 0, 1 ); 
            if (this->InteractorStyle)
            {
                this->InteractorStyle->Delete();
                this->InteractorStyle = vtkInteractorStyleImage::New();
            }
            this->SetName( DefaultViewNames[SAGITTAL_VIEW_TYPE] );
            break;
        case CORONAL_VIEW_TYPE:
            this->Renderer->GetActiveCamera()->SetPosition( 0, 1, 0 ); // left on right
            this->Renderer->GetActiveCamera()->SetFocalPoint( 0, 0, 0 );
            this->Renderer->GetActiveCamera()->SetViewUp( 0, 0, 1 ); 
            if (this->InteractorStyle)
            {
                this->InteractorStyle->Delete();
                this->InteractorStyle = vtkInteractorStyleImage::New();
            }
            this->SetName( DefaultViewNames[CORONAL_VIEW_TYPE] );
            break;
        case TRANSVERSE_VIEW_TYPE: // axial
            this->Renderer->GetActiveCamera()->SetPosition( 0, 0, -1 ); //left on right
            this->Renderer->GetActiveCamera()->SetFocalPoint( 0, 0, 0 );
            this->Renderer->GetActiveCamera()->SetViewUp( 0, 1, 0 ); //nose up
            if (this->InteractorStyle)
            {
                this->InteractorStyle->Delete();
                this->InteractorStyle = vtkInteractorStyleImage::New();
            }
            this->SetName( DefaultViewNames[TRANSVERSE_VIEW_TYPE] );
            break;
        case THREED_VIEW_TYPE:
            this->Renderer->GetActiveCamera()->SetPosition( 1, 0, 0 );
            this->Renderer->GetActiveCamera()->SetFocalPoint( 0, 0, 0 );
            this->Renderer->GetActiveCamera()->SetViewUp( 0, 0, 1 );
            break;
    }
}


void View::SetInteractor( vtkRenderWindowInteractor * interactor )
{
    if( interactor == this->Interactor )
    {
        return;
    }
    
    if( this->Interactor )
    {
        this->ReleaseAllObjects();
        this->Interactor->UnRegister( this );
    }
    
    interactor->Register( this );
    this->Interactor = interactor;
    this->SetupAllObjects();
    this->Interactor->SetInteractorStyle( this->InteractorStyle );
    this->InteractorStyle->Delete();
    this->Interactor->GetRenderWindow()->AddRenderer( this->Renderer );
}

   
void View::ReleaseView()
{
    this->ReleaseAllObjects();
    this->Renderer->SetRenderWindow( 0 );
    this->Interactor->SetRenderWindow( 0 );
}


void View::SetBackgroundColor( vtkReal * color )
{
    this->Renderer->SetBackground( color );
    this->NotifyNeedRender();
}


void View::ResetCamera()
{
    if( this->Renderer )
    {
        this->Renderer->ResetCamera();
    }    
}

void View::ZoomCamera(double factor)
{
    if( this->Renderer )
    {
        this->Renderer->GetActiveCamera()->Zoom(factor);
    }    
}

void View::NotifyNeedRender()
{
    emit Modified();
}


void View::SetupAllObjects()
{
    if( !this->Interactor )
    {
        vtkErrorMacro(<< "Can't setup objects before setting its interactor." );
        return;
    }
    
    int numberOfObjects = this->Manager->GetNumberOfObjects();
    for( int i = 0; i < numberOfObjects; i++ )
    {
        this->Manager->GetObject( i )->Setup( this, 0 );
    }
}


void View::ReleaseAllObjects()
{
    int numberOfObjects = this->Manager->GetNumberOfObjects();
    for( int i = 0; i < numberOfObjects; i++ )
    {
        this->Manager->GetObject( i )->Release( this);
    }
}

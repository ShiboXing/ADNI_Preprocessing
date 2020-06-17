#include "vtkQtImageViewer.h"
#include "vtkQtRenderWindow.h"
#include "vtkQtRenderWindowInteractor.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkInteractorStyleImage.h"
#include "vtkImageData.h"
#include "vtkReal.h"

vtkQtImageViewer::vtkQtImageViewer( QWidget * parent, const char * name ) : vtkQtRenderWindow( parent, name )
{
    m_actor = vtkImageActor::New();
    m_actor->InterpolateOff();
    
    m_renderer = vtkRenderer::New();
    m_renderer->AddViewProp( m_actor );
    
    AddRenderer( m_renderer );

    vtkRenderWindowInteractor * interactor = MakeRenderWindowInteractor();
    
    vtkInteractorStyleImage * interactorStyle = vtkInteractorStyleImage::New();
    interactor->SetInteractorStyle( interactorStyle );
    interactorStyle->Delete();
}


vtkQtImageViewer::~vtkQtImageViewer()
{
    m_actor->Delete();
    m_renderer->Delete();
}


void vtkQtImageViewer::SetInput( vtkImageData * input )
{
    m_actor->SetInput( input );
}


void vtkQtImageViewer::RenderFirst()
{
    m_actor->GetInput()->UpdateInformation();
    int * extent = m_actor->GetInput()->GetWholeExtent();
    int diffx = extent[1] - extent[0] + 1;
    vtkReal scalex = (vtkReal)diffx / 2.0; 
    int diffy = extent[3] - extent[2] + 1;
    vtkReal scaley = (vtkReal)diffy / 2.0;

    vtkCamera * cam = m_renderer->GetActiveCamera();
    cam->ParallelProjectionOn();
    cam->SetParallelScale( scaley );
    double * prevPos = cam->GetPosition();
    double * prevFocal = cam->GetFocalPoint();
    cam->SetPosition( scalex, scaley, prevPos[2] );
    cam->SetFocalPoint( scalex, scaley, prevFocal[2] );

    Render();
}


vtkQtImageViewer * vtkQtImageViewer::New()
{
    return new vtkQtImageViewer;
}


void vtkQtImageViewer::PrintSelf(ostream& os, vtkIndent indent)
{
}


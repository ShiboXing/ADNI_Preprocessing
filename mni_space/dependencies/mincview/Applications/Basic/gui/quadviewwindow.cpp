#include "quadviewwindow.h"

#include <qvariant.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorObserver.h"
#include "vtkInteractorStyle.h"
#include "scenemanager.h"
#include "view.h"
#include "zoomfactordialog.h"
#include "vtkRenderWindow.h"
#include "vtkQtRenderWindow.h"
#include "vtkQtAsyncCallback.h"
#include "vtkMulti3DWidget.h"
#include "vtkActor2D.h"
#include "vtkTextMapper.h"
#include "vtkCornerAnnotation.h"

const QString QuadViewWindow::ViewNames[4] = { "Transverse","ThreeD","Coronal","Sagittal" };

QuadViewWindow::QuadViewWindow( QWidget* parent, const char* name, WFlags fl ) : QWidget( parent, name, fl )
{
    if ( !name )
    {
        setName( "QuadViewWindow" );
    }

    m_generalLayout = new QVBoxLayout( this, 11, 6, "GeneralLayout");

    // Add a button box at the top of the window. These buttons control view layout and manipulation tools    
    m_buttonBox = new QHBoxLayout( 0, 0, 6, "ButtonBox" );
    m_generalLayout->addItem( m_buttonBox );
    
    // Reset camera button
    m_resetCamerasButton = new QPushButton( this, "ResetCameraButton" );
    m_resetCamerasButton->setMinimumSize( QSize( 120, 0 ) );
    m_resetCamerasButton->setText( "Reset Cameras" );
    m_buttonBox->addWidget( m_resetCamerasButton );
    connect( m_resetCamerasButton, SIGNAL(clicked()), this, SLOT( ResetCameraButtonClicked() ) );
    
    // Reset camera button
    m_zoomCamerasButton = new QPushButton( this, "ZoomCameraButton" );
    m_zoomCamerasButton->setMinimumSize( QSize( 120, 0 ) );
    m_zoomCamerasButton->setText( "Zoom" );
    m_buttonBox->addWidget( m_zoomCamerasButton );
    connect( m_zoomCamerasButton, SIGNAL(clicked()), this, SLOT( ZoomCameraButtonClicked() ) );
    
    // Expand/Reduce current view button
    m_expandViewButton = new QPushButton( this, "ExpandViewButton" );
    m_expandViewButton->setMinimumSize( QSize( 120, 0 ) );
    m_expandViewButton->setText( "Expand View" );
    m_buttonBox->addWidget( m_expandViewButton );
    connect( m_expandViewButton, SIGNAL(clicked()), this, SLOT( ExpandViewButtonClicked() ) );
    
    // Spacer at the end of the button box so that the buttons don't scale
    m_buttonBoxSpacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    m_buttonBox->addItem( m_buttonBoxSpacer );

    // Add the 3 splitters that separate the 4 vtk windows
    m_verticalSplitter = new QSplitter( this, "VerticalSplitter" );
    m_verticalSplitter->setOrientation( QSplitter::Vertical );
    m_generalLayout->addWidget( m_verticalSplitter );

    m_upperHorizontalSplitter = new QSplitter( m_verticalSplitter, "UpperHorizontalSplitter" );
    m_upperHorizontalSplitter->setOrientation( QSplitter::Horizontal );
    
    m_lowerHorizontalSplitter = new QSplitter( m_verticalSplitter, "LowerHorizontalSplitter" );
    m_lowerHorizontalSplitter->setOrientation( QSplitter::Horizontal );

    // Create the 4 basic vtk windows
    m_vtkWindows[0] = new vtkQtRenderWindow( m_upperHorizontalSplitter, "UpperLeftView" );
    m_vtkWindows[1] = new vtkQtRenderWindow( m_upperHorizontalSplitter, "UpperRightView" );
    m_vtkWindows[2] = new vtkQtRenderWindow( m_lowerHorizontalSplitter, "LowerLeftView" );
    m_vtkWindows[3] = new vtkQtRenderWindow( m_lowerHorizontalSplitter, "LowerRightView" );
    resize( QSize(1000, 800).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    this->ViewWindowButtonPressedCallback = vtkObjectCallback< QuadViewWindow >::New();
    this->ViewWindowButtonPressedCallback->SetCallback( this, &QuadViewWindow::ViewWindowButtonPressed );
    
    for( int i = 0; i < 4; i++ )
    {
        this->ViewNeedRender[i] = 0;
        m_interactors[i] = m_vtkWindows[i]->MakeRenderWindowInteractor();
        m_interactors[i]->SetRenderWindow( m_vtkWindows[i] );
        m_interactors[i]->AddObserver( vtkCommand::LeftButtonPressEvent, this->ViewWindowButtonPressedCallback, 1 );
        m_interactors[i]->AddObserver( vtkCommand::MiddleButtonPressEvent, this->ViewWindowButtonPressedCallback, 1 );
        m_interactors[i]->AddObserver( vtkCommand::RightButtonPressEvent, this->ViewWindowButtonPressedCallback, 1 );
        m_interactors[i]->Initialize();
    }
    
    this->currentViewExpanded = 0;
    this->Mode = Immediate;
}


QuadViewWindow::~QuadViewWindow()
{
    if( m_sceneManager )
    {
        // @TODO: do some release to avoid some vtk objects to have non-zero reference-counting
        // when the application is closed.
        m_sceneManager->ReleaseAllViews();
    }
    
    // vtk windows need implicit deleting to decrement reference count.
    for( int i = 0; i < 4; i++ )
    {
        m_vtkWindows[i]->Delete();
        m_interactors[i]->Delete();
    }
}


void QuadViewWindow::SetSceneManager( SceneManager * man )
{
    if( man )
    {
        View * view = man->GetView( ViewNames[0] );
        if( !view )
        {
            view = man->CreateView( TRANSVERSE_VIEW_TYPE );
            view->SetName( ViewNames[0] );
        }
        view->SetInteractor( m_vtkWindows[0]->GetInteractor() );
        connect( view, SIGNAL( Modified() ), this, SLOT( Win0NeedsRender() ) );
        
        view = man->GetView( ViewNames[1] );
        if( !view )
        {
            view = man->CreateView( THREED_VIEW_TYPE );
            view->SetName( ViewNames[1] );
        }
        view->SetInteractor( m_vtkWindows[1]->GetInteractor() );
        connect( view, SIGNAL( Modified() ), this, SLOT( Win1NeedsRender() ) );
        
        view = man->GetView( ViewNames[2] );
        if( !view )
        {
            view = man->CreateView( CORONAL_VIEW_TYPE );
            view->SetName( ViewNames[2] );
        }
        view->SetInteractor( m_vtkWindows[2]->GetInteractor() );
        connect( view, SIGNAL( Modified() ), this, SLOT( Win2NeedsRender() ) );
        
        view = man->GetView( ViewNames[3] );
        if( !view )
        {
            view = man->CreateView( SAGITTAL_VIEW_TYPE );
            view->SetName( ViewNames[3] );
        }
        view->SetInteractor( m_vtkWindows[3]->GetInteractor() );
        connect( view, SIGNAL( Modified() ), this, SLOT( Win3NeedsRender() ) );
        
        connect( man, SIGNAL( ObjectAdded() ), this, SLOT( RenderAll() ) );
        connect( man, SIGNAL( ObjectRemoved() ), this, SLOT( RenderAll() ) );
        
        m_sceneManager = man;
        m_sceneManager->SetCurrentView( 1 );
        
        m_sceneManager->PreDisplaySetup();
        this->PlaceCornerText();
    }
}


void QuadViewWindow::ViewWindowButtonPressed( vtkObject * caller, unsigned long eventId, void * callData )
{
    vtkRenderWindowInteractor * interactor = (vtkRenderWindowInteractor*)caller;
    for( int i = 0; i < 4; i++ )
    {
        if( interactor == this->m_vtkWindows[ i ]->GetInteractor() )
        {
            if( m_sceneManager->GetCurrentView() != i )
            {
                m_sceneManager->SetCurrentView( i );
                this->ViewWindowButtonPressedCallback->AbortFlagOn();
                interactor->InvokeEvent(eventId);
            }
        }
    }
}


void QuadViewWindow::Win0NeedsRender() { this->WinNeedsRender( 0 ); }
void QuadViewWindow::Win1NeedsRender() { this->WinNeedsRender( 1 ); }
void QuadViewWindow::Win2NeedsRender() { this->WinNeedsRender( 2 ); }
void QuadViewWindow::Win3NeedsRender() { this->WinNeedsRender( 3 ); }

void QuadViewWindow::WinNeedsRender( int winIndex )
{
    if( this->Mode == Immediate )
    {
        m_vtkWindows[ winIndex ]->Render();
    }
    else
    {
        this->ViewNeedRender[ winIndex ] = 1;
    }
}

void QuadViewWindow::ResetCameraButtonClicked()
{
    m_sceneManager->ResetAllCameras();
    this->RenderAll();
}

void QuadViewWindow::ZoomCameraButtonClicked()
{
    ZoomFactorDialog *dlg = new ZoomFactorDialog(this, "Zoom Factor", TRUE, Qt::WDestructiveClose);
    dlg->SetSceneManager(m_sceneManager);
    if (dlg->exec() == QDialog::Accepted)
        this->RenderAll();
}

void QuadViewWindow::RenderAll()
{
    for( int i = 0; i < 4; i++ )
    {
        m_vtkWindows[i]->Render();
    }
}

void QuadViewWindow::RenderSignal()
{
    if( this->Mode == OnSignal )
    {
        for( int i = 0; i < 4; i++ )
        {
            if( this->ViewNeedRender[i] )
            {
                m_vtkWindows[i]->Render();
                this->ViewNeedRender[i] = 0;
            }
        }
    }
}

void QuadViewWindow::ExpandViewButtonClicked()
{
    if( this->currentViewExpanded )
    {
        for( int i = 0; i < 4; i++ )
        {
            m_vtkWindows[i]->show();
        }
        this->currentViewExpanded = 0;
        m_expandViewButton->setText( "Expand view" );
    }
    else
    {
        int currentView = m_sceneManager->GetCurrentView();
        for( int i = 0; i < 4; i++ )
        {
            if( i != currentView )
            {
                m_vtkWindows[i]->hide();
            }
        }
        m_expandViewButton->setText( "Dock view" );
        this->currentViewExpanded = 1;
    }
}

void QuadViewWindow::PlaceCornerText()
{
    vtkRenderer *renderer = m_sceneManager->GetViewRenderer(CORONAL_VIEW_TYPE);
    vtkCornerAnnotation *ca = vtkCornerAnnotation::New();
    ca->SetText(0, "R");
    ca->SetText(1, "L");
    ca->SetText(2, "R");
    ca->SetText(3, "L");
    ca->SetMaximumFontSize(20);
    vtkTextMapper *textMapper = vtkTextMapper::New();
    ca->SetMapper(textMapper);
    renderer->AddActor2D( ca );
    textMapper->Delete();
    ca->ReleaseGraphicsResources(m_vtkWindows[2]);
    ca->Delete();
    renderer = m_sceneManager->GetViewRenderer(TRANSVERSE_VIEW_TYPE);
    ca = vtkCornerAnnotation::New();
    ca->SetText(0, "R");
    ca->SetText(1, "L");
    ca->SetText(2, "R");
    ca->SetText(3, "L");
    ca->SetMaximumFontSize(20);
    textMapper = vtkTextMapper::New();
    ca->SetMapper(textMapper);
    renderer->AddActor2D( ca );
    textMapper->Delete();
    ca->ReleaseGraphicsResources(m_vtkWindows[0]);
    ca->Delete();
    renderer = m_sceneManager->GetViewRenderer(SAGITTAL_VIEW_TYPE);
    ca = vtkCornerAnnotation::New();
    ca->SetText(0, "A");
    ca->SetText(1, "P");
    ca->SetText(2, "A");
    ca->SetText(3, "P");
    ca->SetMaximumFontSize(20);
    textMapper = vtkTextMapper::New();
    ca->SetMapper(textMapper);
    renderer->AddActor2D( ca );
    textMapper->Delete();
    ca->ReleaseGraphicsResources(m_vtkWindows[3]);
    ca->Delete();
    renderer = m_sceneManager->GetViewRenderer(THREED_VIEW_TYPE);
    ca = vtkCornerAnnotation::New();
    ca->SetText(0, "I");
    ca->SetText(1, "I");
    ca->SetText(2, "S");
    ca->SetText(3, "S");
    ca->SetMaximumFontSize(20);
    textMapper = vtkTextMapper::New();
    ca->SetMapper(textMapper);
    renderer->AddActor2D( ca );
    textMapper->Delete();
    ca->ReleaseGraphicsResources(m_vtkWindows[1]);
    ca->Delete();
}


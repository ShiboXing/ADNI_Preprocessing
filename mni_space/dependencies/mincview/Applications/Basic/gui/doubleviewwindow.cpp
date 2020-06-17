/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: doubleviewwindow.cpp,v $
  Language:  C++
  Date:      $Date: 2010-03-31 14:27:27 $
  Version:   $Revision: 1.13 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "doubleviewwindow.h"

#include <qstring.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qframe.h>
#include "vtkQtRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkPolyData.h"
#include "vtkGrid.h"
#include "vtkOutlineFilter.h"
#include "vtkActor.h"
#include "vtkImageActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkTransform.h"
#include "vtkImageData.h"
#include "vtkImageReslice.h"
#include "vtkImageBlend.h"
#include "vtkLookupTable.h"
#include "vtkImageShiftScale.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkCamera.h"
#include "vtkSmartPointer.h"

#include "vtkQtRenderWindowInteractor.h"

const int defaultWinX = 640;
const int defaultWinY = 480;
const vtkReal defaultScale = 255.0/65535.0;
const int maxGridStep = 30;
const int minGridStep = 1;
const int defaultGridStep = 10;
const int defaultOpacity = 50;

DoubleViewWindow::DoubleViewWindow( QWidget* parent, const char* name, WFlags fl ) : 
DoubleViewWindowBase( parent, name, fl )
{
    if ( !name )
    {
        setName( "MultipleFilesViewWindow" );
    }

    // Create the 2 view windows
    m_usImageWindow = new vtkQtRenderWindow( leftFrame, "USImageWindow" );
    m_usImageWindow->setMinimumSize( QSize( defaultWinX, defaultWinY ) );
    m_usImageWindow->setMaximumSize( QSize( defaultWinX, defaultWinY ) );
    m_usImageWindow->setGeometry( QRect( 2, 2, defaultWinX, defaultWinY ) );
    m_usImageWindow->SetSize( defaultWinX, defaultWinY);
    
    m_mriImageWindow = new vtkQtRenderWindow( rightFrame, "MRIImageWindow" );
    m_mriImageWindow->setMinimumSize( QSize( defaultWinX, defaultWinY ) );
    m_mriImageWindow->setMaximumSize( QSize( defaultWinX, defaultWinY ) );
    m_mriImageWindow->setGeometry( QRect( 2, 2, defaultWinX, defaultWinY ) );
    m_mriImageWindow->SetSize( defaultWinX, defaultWinY);
    
    resize( QSize(2* defaultWinX, defaultWinY).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    
    this->InitializeVariables();
}

DoubleViewWindow::~DoubleViewWindow()
{
    m_usImageWindow->Delete();
    m_mriImageWindow->Delete();
    if (m_grid)
        m_grid->Delete();
    if(m_usRenderer)
    {
        m_usRenderer->Delete();
    }
    if(m_mriRenderer)
    {
        m_mriRenderer->Delete();
    }
    if (m_usGridActor)
    {
        m_usGridActor->Delete();
    }
    if (m_usActor)
    {
        m_usActor->Delete();
    }
    if (m_mriActor)
    {
        m_mriActor->Delete();
    }
    if (m_mriGridActor)
    {
        m_mriGridActor->Delete();
    }
    if (m_usOutlineActor)
    {
        m_usOutlineActor->Delete();
    }
    if (m_usOutlineFilter)
    {
        m_usOutlineFilter->Delete();
    }
    if (m_usOutlineMapper)
    {
        m_usOutlineMapper->Delete();
    }
    if (m_mriOutlineActor)
    {
        m_mriOutlineActor->Delete();
    }
    if (m_mriOutlineFilter)
    {
        m_mriOutlineFilter->Delete();
    }
    if (m_mriOutlineMapper)
    {
        m_mriOutlineMapper->Delete();
    }
    if( m_imageData )
    {
        m_imageData->UnRegister( 0 );
    }
    if (m_reslice)
    {
        m_reslice->Delete();
    } 
    if (m_displayImage)
    {
        m_displayImage->Delete();
    }
    if (m_blendedImage)
    {
        m_blendedImage->Delete();
    }
    if (m_usLut)
    {
        m_usLut->Delete();
    }
    if( m_transform )
    {
        m_transform->UnRegister( 0 );
    }
    if (m_displayWindowLevelColorsImage)
    {
//        cout << m_displayWindowLevelColorsImage->GetReferenceCount();
        
//        m_displayWindowLevelColorsImage->Delete(); // crash!!!!
    }
}

void DoubleViewWindow::GridCheckBoxToggled()
{
    if (m_gridOn)
    {
        decreaseGridStepPushButton->setEnabled( FALSE );
        increaseGridStepPushButton->setEnabled( FALSE );
        gridStepSlider->setEnabled( FALSE );
        gridStepLineEdit->setEnabled( FALSE );
        gridStepLineEdit->setText( " " );
        m_usRenderer->RemoveViewProp(m_usGridActor);
        m_mriRenderer->RemoveViewProp(m_mriGridActor);
    }
    else
    {
        decreaseGridStepPushButton->setEnabled( TRUE );
        increaseGridStepPushButton->setEnabled( TRUE );
        gridStepSlider->setEnabled( TRUE );
        gridStepSlider->setValue( m_grid->GetGridStep() );
        gridStepLineEdit->setEnabled( TRUE );
        gridStepLineEdit->setText( QString::number(m_grid->GetGridStep()) );
        m_usRenderer->AddViewProp(m_usGridActor);
        m_mriRenderer->AddViewProp(m_mriGridActor);
    }
    this->UpdateViews();
    m_gridOn = !m_gridOn;
}

void DoubleViewWindow::BlendCheckBoxToggled()
{
    m_blending = !m_blending;
    if (!m_blending)
    {
        decreaseOpacityPushButton->setEnabled( FALSE );
        increaseOpacityPushButton->setEnabled( FALSE );
        opacitySlider->setEnabled( FALSE );
        opacityStepLineEdit->setEnabled( FALSE );
        opacityStepLineEdit->setText( " " );
    }
    else
    {
        decreaseOpacityPushButton->setEnabled( TRUE );
        increaseOpacityPushButton->setEnabled( TRUE );
        opacitySlider->setEnabled( TRUE );
        opacityStepLineEdit->setEnabled( TRUE );
        opacityStepLineEdit->setText( QString::number( m_opacity ) );
    }
}

void DoubleViewWindow::RestoreViewsButtonClicked()
{
    this->SetDefaultView(m_usActor, m_usRenderer);
    this->SetDefaultView(m_mriActor, m_mriRenderer);
    this->RestoreGridAndBlending();
    this->UpdateViews();
}

void DoubleViewWindow::RestoreGridAndBlending()
{
    if (m_grid)
    {
        m_grid->RestoreDefaultGrid();
        this->InitializeGrid();
        gridStepSlider->setValue( m_grid->GetGridStep() );
    }
    m_opacity = defaultOpacity;
    if (opacitySlider->value() > 0)
    {
        opacitySlider->setValue( m_opacity );
    }
}

void DoubleViewWindow::CloseButtonClicked()
{
    QWidget *parent = this->parentWidget(true);
    parent->close(TRUE);
}

void DoubleViewWindow::OpacitySliderValueChanged( int newOpacity )
{
    if (m_blending)
    {
        opacityStepLineEdit->setText( QString::number( newOpacity ) );
        m_blendedImage->SetOpacity(0, 1.0 - newOpacity/100.0);
        m_blendedImage->SetOpacity(1, newOpacity/100.0);
        m_mriImageWindow->Render(); //TODO check condition
        m_opacity = newOpacity;
    }
}

void DoubleViewWindow::DecreaseOpacityButtonClicked()
{
    if (m_blending)
    {
        int sliderPosition, newOpacity;
        sliderPosition = opacitySlider->value();
        newOpacity = sliderPosition-opacitySlider->lineStep();
        if (newOpacity < 0)
            newOpacity = 0;
        opacitySlider->setValue(newOpacity);
    }
}

void DoubleViewWindow::IncreaseOpacityButtonClicked()
{
    if (m_blending)
    {
        int sliderPosition, newOpacity;
        sliderPosition = opacitySlider->value();
        newOpacity = sliderPosition+opacitySlider->lineStep();
        if (newOpacity > 100)
            newOpacity = 100;
        opacitySlider->setValue(newOpacity);
    }
}

void DoubleViewWindow::OpacityLineEditTextChanged()
{
    if (m_blending)
    {
        QString t = opacityStepLineEdit->text();
        int newOpacity = t.toInt();
        if (newOpacity < 0)
            newOpacity = 0;
        if (newOpacity > 100)
            newOpacity = 100;
        opacityStepLineEdit->blockSignals(TRUE);
        opacitySlider->setValue( newOpacity );
        opacityStepLineEdit->blockSignals(FALSE);
    }
}
    
void DoubleViewWindow::GridStepSliderValueChanged( int newStep )
{
    if (m_gridOn)
    {
        gridStepLineEdit->setText( QString::number( newStep ) );
        m_grid->SetGridStep(newStep); 
        m_grid->MakeGrid();
        this->UpdateViews(); //TODO check condition
    }
}

void DoubleViewWindow::DecreaseGridStepButtonClicked()
{
    if (m_gridOn)
    {
        int sliderPosition, newStep;
        sliderPosition = gridStepSlider->value();
        newStep = sliderPosition-gridStepSlider->lineStep();
        if (newStep < minGridStep)
            newStep = minGridStep;
        gridStepSlider->setValue(newStep);
    }
}

void DoubleViewWindow::IncreaseGridStepButtonClicked()
{
    if (m_gridOn)
    {
        int sliderPosition, newStep;
        sliderPosition = gridStepSlider->value();
        newStep = sliderPosition+gridStepSlider->lineStep();
        if (newStep > maxGridStep)
            newStep = maxGridStep;
        gridStepSlider->setValue(newStep);
    }
}

void DoubleViewWindow::GridStepEditChanged()
{
    if (m_gridOn)
    {
        QString t = gridStepLineEdit->text();
        int newStep = t.toInt();
        if (newStep < minGridStep)
            newStep = minGridStep;
        if (newStep > maxGridStep)
            newStep = maxGridStep;
        gridStepSlider->setValue( newStep );
    }
}
    
void DoubleViewWindow::UpdateViews()
{
    m_usImageWindow->Render();
    m_mriImageWindow->Render();
}

void DoubleViewWindow::EnableBlendingButton()
{
    blendCheckBox->setEnabled( TRUE );
    m_blending = FALSE;
}

void DoubleViewWindow::DisableBlending()
{
    if (m_blending)
    {
        m_blending = FALSE;
        if (blendCheckBox->isChecked())
            blendCheckBox->setChecked( FALSE );
        decreaseOpacityPushButton->setEnabled( FALSE );
        increaseOpacityPushButton->setEnabled( FALSE );
        opacitySlider->setEnabled( FALSE );
        opacityStepLineEdit->setEnabled( FALSE );
        opacityStepLineEdit->setText( " " );
        if (m_blendedImage)
        {
            m_blendedImage->Delete();
            m_blendedImage = 0;
            m_mriActor->SetInput(m_displayWindowLevelColorsImage->GetOutput());
            m_mriImageWindow->Render();
        }
    }
    blendCheckBox->setEnabled( FALSE );
}

void DoubleViewWindow::UnBlend()
{
}

void DoubleViewWindow::InitializeGrid()
{
    m_grid->SetGridSize(defaultWinX, defaultWinY);
    m_grid->SetGridStep(defaultGridStep);
    vtkReal tmpScale[3] = {0.01, 0.01, 0.01};
    if (m_transform)
    {
        vtkSmartPointer< vtkTransform > tmp = vtkSmartPointer< vtkTransform >::New();
        tmp->Identity();
        tmp->SetInput( m_transform->GetLinearInverse() );
        tmp->GetScale(tmpScale);
    }
    m_grid->SetGridScale(tmpScale);
}

void DoubleViewWindow::SetGridActors()
{
    m_usGridActor = vtkActor::New();
    m_usGridActor->GetProperty()->SetRepresentationToWireframe();
    m_usGridActor->GetProperty()->SetLineWidth(1.0);
    m_usGridActor->GetProperty()->SetColor(0.9, 0.8, 0.33);
    m_usGridActor->GetProperty()->SetInterpolationToFlat();
    m_usGridActor->GetProperty()->BackfaceCullingOff();
    vtkPolyDataMapper * usGridMapper = vtkPolyDataMapper::New();
    usGridMapper->SetInput( m_grid->GetGridPolyData() );
    m_usGridActor->SetMapper( usGridMapper );
    usGridMapper->Delete();
    
    m_mriGridActor = vtkActor::New();
    m_mriGridActor->GetProperty()->SetRepresentationToWireframe();
    m_mriGridActor->GetProperty()->SetLineWidth(1.0);
    m_mriGridActor->GetProperty()->SetColor(0.9, 0.8, 0.33);
    m_mriGridActor->GetProperty()->SetInterpolationToFlat();
    m_mriGridActor->GetProperty()->BackfaceCullingOff();
    vtkPolyDataMapper * mriGridMapper = vtkPolyDataMapper::New();
    mriGridMapper->SetInput( m_grid->GetGridPolyData() );
    m_mriGridActor->SetMapper( mriGridMapper );
    mriGridMapper->Delete();
}

void DoubleViewWindow::InitializeVariables()
{
    m_usRenderer= vtkRenderer::New();
    m_usImageWindow->AddRenderer( m_usRenderer );
    m_usInteractor = m_usImageWindow->MakeRenderWindowInteractor();
    m_usInteractor->SetRenderWindow(m_usImageWindow);
    m_mriRenderer= vtkRenderer::New();
    m_mriImageWindow->AddRenderer( m_mriRenderer );
    m_mriInteractor = m_mriImageWindow->MakeRenderWindowInteractor();
    m_mriInteractor->SetRenderWindow(m_mriImageWindow);

    this->InitializeActors();

    m_reslice = vtkImageReslice::New();
    m_displayImage = 0;
    m_displayWindowLevelColorsImage = 0;
    m_blendedImage = 0;
    
    m_transform = 0;
    
    m_blending = FALSE;
    m_gridOn = FALSE;
    m_usGridActor = 0;
    m_mriGridActor = 0;
    m_opacity = defaultOpacity;
    m_imageData = 0;
    m_win_level[0] = 255;
    m_win_level[1] = 127.5;

    m_grid = vtkGrid::New();
    this->InitializeGrid();
    this->SetGridActors();
    gridStepSlider->setMinValue( minGridStep );
    gridStepSlider->setMaxValue( maxGridStep );
    gridStepSlider->setFocusPolicy( QWidget::ClickFocus );

    m_usLut = vtkLookupTable::New();
    m_usLut->SetTableRange(0,255);
    m_usLut->SetHueRange( 0.3, 0.3 );
    m_usLut->SetSaturationRange( 1, 1 );
    m_usLut->SetValueRange( 0.25, 1 );
    m_usLut->Build();
}

void DoubleViewWindow::InitializeActors()
{
}

void DoubleViewWindow::SetImageData(vtkImageData *data)
{
    if( m_imageData == data )
    {
        return;
    }
    
    if( m_imageData )
    {
        m_imageData->UnRegister( 0 );
    }
    
    m_imageData = data;
    
    if( m_imageData )
    {
        m_imageData->Register( 0 );
        
        vtkReal range[2];
        m_imageData->GetScalarRange( range );
        m_win_level[0] = range[1] - range[0];
        m_win_level[1] = 0.5 * (range[1] - range[0]);
    }
}

void DoubleViewWindow::SetTransform(vtkTransform *t)
{
    if( m_transform == t )
    {
        return;
    }
    
    if( m_transform )
    {
        m_transform->UnRegister( 0 );
    }
    
    m_transform = t;
    
    if( m_transform )
    {
        m_transform->Register( 0 );
    }
}

void DoubleViewWindow::SetDefaultView(vtkImageActor *actor, vtkRenderer *renderer)
{
    vtkImageData *input = actor->GetInput();
    if (input)
    {
        input->UpdateInformation();
        int * extent = input->GetWholeExtent();
        int diffx = extent[1] - extent[0] + 1;
        vtkReal scalex = (vtkReal)diffx / 2.0; 
        int diffy = extent[3] - extent[2] + 1;
        vtkReal scaley = (vtkReal)diffy / 2.0;
        vtkCamera * cam = renderer->GetActiveCamera();
        renderer->ResetCamera();
        cam->ParallelProjectionOn();
        cam->SetParallelScale(scaley);
        double * prevPos = cam->GetPosition();
        double * prevFocal = cam->GetFocalPoint();
        cam->SetPosition( scalex, scaley, prevPos[2] );
        cam->SetFocalPoint( scalex, scaley, prevFocal[2] );
        renderer->Render();
    }
}


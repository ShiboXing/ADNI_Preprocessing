/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: doublewindowfileviewer.cpp,v $
  Language:  C++
  Date:      $Date: 2010-04-08 14:55:53 $
  Version:   $Revision: 1.11 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "doublewindowfileviewer.h"
#include <qstring.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "ignsmsg.h"
#include "vtkQtRenderWindow.h"
#include "vtkImageData.h"
#include "vtkTransform.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkRenderer.h"
#include "vtkImageReslice.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include "vtkLookupTable.h"
#include "vtkImageShiftScale.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageActor.h"
#include "vtkImageBlend.h"
#include "vtkImagePlaneActor.h"
#include "vtkProperty.h"
#include "vtkOutlineFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkImagePlaneActor.h"
#include "vtkImageMask.h"
#include "vtkGrid.h"
#include "vtkSmartPointer.h"
#include "MINCReader.h"

DoubleWindowFileViewer::DoubleWindowFileViewer( QWidget* parent, const char* name, WFlags fl )
    : DoubleViewWindow( parent, name, fl ),
    m_currentUsImage(0),
    m_greenUS(0),
    m_usWindowLevelColorsImage(0),
    m_maskedImage(0),
    m_initLeftViewDone(0),
    m_initRightViewDone(0)
{
    this->InitializeActors();
    m_imageTransform = vtkTransform::New();
    m_imageTransform->Identity();
    m_inputResliceTransform = vtkTransform::New();
    m_inputResliceTransform->Identity();
    m_gridScale[0] = 1.0;
    m_gridScale[1] = 1.0;
    m_gridScale[1] = 1.0;
}

DoubleWindowFileViewer::~DoubleWindowFileViewer()
{
    if (m_imageTransform)
        m_imageTransform->Delete();
    if (m_inputResliceTransform)
        m_inputResliceTransform->Delete();
    if (m_maskedImage)
        m_maskedImage->Delete();
}

void DoubleWindowFileViewer::InitializeActors()
{    
    m_usOutlineActor = 0;
    m_usOutlineMapper = 0;
    m_usOutlineFilter = 0;
    
    m_mriOutlineActor = 0;
    m_mriOutlineMapper = 0;
    m_mriOutlineFilter = 0;
    
    m_usActor = vtkImageActor::New();
    m_usActor->InterpolateOff();
    m_mriActor = vtkImageActor::New();
    m_mriActor->InterpolateOff();
}

void DoubleWindowFileViewer::InitLeftView()
{
    m_usRenderer->AddViewProp( m_usActor );
    m_usInteractor->Initialize();
    vtkInteractorStyleImage * style = vtkInteractorStyleImage::New();
    m_usInteractor->SetInteractorStyle( style );
    style->Delete();
    if (m_currentUsImage)
    {
        m_usWindowLevelColorsImage = vtkImageMapToWindowLevelColors::New();
        m_usWindowLevelColorsImage->SetInput(m_currentUsImage);
        m_us_win_level[0] = 255.0;
        m_us_win_level[1] = 127.5;

        if (m_usLut)
        {
            vtkReal range[2];
            m_usLut->GetTableRange( range );
            m_win_level[0] = range[1]-range[0];
            m_win_level[1] = (range[1]-range[0])*0.5;

        }
    }
    m_initLeftViewDone = 1;
}

void DoubleWindowFileViewer::SetUltrasoundImage(vtkImageData *image)
{
    m_currentUsImage = image;        
    if (!m_initLeftViewDone)
        this->InitLeftView();
    m_usWindowLevelColorsImage->SetInput(m_currentUsImage);
    m_usWindowLevelColorsImage->Update();
    m_usActor->SetInput(m_usWindowLevelColorsImage->GetOutput());
    m_usActor->VisibilityOn();
    SetDefaultView(m_usActor, m_usRenderer);
    this->UpdateViews();
    if (m_blending)
        this->UpdateBlendedImage();
}

void DoubleWindowFileViewer::SetResliceTransform(vtkTransform *resTransform)
{
    if (m_initRightViewDone)
    {
        m_inputResliceTransform->Identity();
        m_inputResliceTransform->Concatenate(resTransform->GetMatrix());
        m_reslice->UpdateInformation();
        m_displayWindowLevelColorsImage->Update();
        this->UpdateViews();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        vtkMatrix4x4 *mat;
        mat = m_inputResliceTransform->GetMatrix();
        igstkLogMacro( DEBUG, "DoubleWindowFileViewer m_inputResliceTransform:" << "\n" <<
        mat->Element[0][0] << ", " <<
        mat->Element[0][1] << ", " <<
        mat->Element[0][2] << ", " <<
        mat->Element[0][3] << "\n" <<
        mat->Element[1][0] << ", " <<
        mat->Element[1][1] << ", " <<
        mat->Element[1][2] << ", " <<
        mat->Element[1][3] << "\n" <<
        mat->Element[2][0] << ", " <<
        mat->Element[2][1] << ", " <<
        mat->Element[2][2] << ", " <<
        mat->Element[2][3] <<
        endl);
#endif
    }
    this->SetDefaultView(m_usActor, m_usRenderer);
}

void DoubleWindowFileViewer::RestoreViewsButtonClicked()
{
    this->SetDefaultView(m_usActor, m_usRenderer);
    this->SetDefaultView(m_mriActor, m_mriRenderer);
    this->RestoreGridAndBlending();
    this->UpdateViews();
}

void DoubleWindowFileViewer::RestoreGridAndBlending()
{
    if (m_grid)
    {
        this->InitializeGrid();
        gridStepSlider->setValue( m_grid->GetGridStep() );
    }
    m_opacity = defaultOpacity;
    if (opacitySlider->value() > 0)
    {
        opacitySlider->setValue( m_opacity );
    }
}

void DoubleWindowFileViewer::SetImageData(vtkImageData *data, vtkLookupTable *lut)
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
        this->InitRightView(lut);
    }
}

void DoubleWindowFileViewer::InitRightView(vtkLookupTable *lut)
{
    if (m_imageData)
    {
        vtkReal range[2];
        m_imageData->GetScalarRange( range );
        if (lut)
            lut->GetTableRange( range );
        m_win_level[0] = range[1]-range[0];
        m_win_level[1] = (range[1]-range[0])*0.5;
        
        vtkTransform * concat = vtkTransform::New();
        concat->Identity();
        concat->SetInput( m_imageTransform->GetLinearInverse() );
        concat->Concatenate( m_inputResliceTransform );
        
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        vtkMatrix4x4 *mat = m_imageTransform->GetMatrix();
        igstkLogMacro( DEBUG, "InitRightView m_imageTransform:" << "\n" <<
        mat->Element[0][0] << ", " << 
        mat->Element[0][1] << ", " <<
        mat->Element[0][2] << ", " << 
        mat->Element[0][3] << "\n" << 
        mat->Element[1][0] << ", " << 
        mat->Element[1][1] << ", " <<
        mat->Element[1][2] << ", " <<
        mat->Element[1][3] << "\n" <<
        mat->Element[2][0] << ", " << 
        mat->Element[2][1] << ", " <<
        mat->Element[2][2] << ", " <<
        mat->Element[2][3] << 
        endl);
        mat = m_inputResliceTransform->GetMatrix();
        igstkLogMacro( DEBUG, "InitRightView m_inputResliceTransform:" << "\n" <<
        mat->Element[0][0] << ", " << 
        mat->Element[0][1] << ", " <<
        mat->Element[0][2] << ", " << 
        mat->Element[0][3] << "\n" << 
        mat->Element[1][0] << ", " << 
        mat->Element[1][1] << ", " <<
        mat->Element[1][2] << ", " <<
        mat->Element[1][3] << "\n" <<
        mat->Element[2][0] << ", " << 
        mat->Element[2][1] << ", " <<
        mat->Element[2][2] << ", " <<
        mat->Element[2][3] << 
        endl);
        mat = concat->GetMatrix();
        igstkLogMacro( DEBUG, "InitRightView concat:" << "\n" <<
        mat->Element[0][0] << ", " << 
        mat->Element[0][1] << ", " <<
        mat->Element[0][2] << ", " << 
        mat->Element[0][3] << "\n" << 
        mat->Element[1][0] << ", " << 
        mat->Element[1][1] << ", " <<
        mat->Element[1][2] << ", " <<
        mat->Element[1][3] << "\n" <<
        mat->Element[2][0] << ", " << 
        mat->Element[2][1] << ", " <<
        mat->Element[2][2] << ", " <<
        mat->Element[2][3] << 
        endl);
#endif        
        m_reslice->SetInput( m_imageData );
        m_reslice->SetInterpolationModeToLinear( );
        m_reslice->SetOutputExtent(0, defaultWinX-1, 0, defaultWinY-1, 0, 1);
        m_reslice->SetOutputOrigin(0, 0, 0);
        m_reslice->SetOutputDimensionality(2);
        m_reslice->SetOutputSpacing(1,1,1);
        m_reslice->SetResliceTransform(concat);
        m_reslice->UpdateInformation();
        
        m_displayWindowLevelColorsImage = vtkImageMapToWindowLevelColors::New();
        int scalarType = m_reslice->GetOutput()->GetScalarType();
        vtkReal scalarRange[2];
        m_imageData->GetScalarRange(scalarRange );
        double scale = ((scalarRange[1]-scalarRange[0])/scalarRange[1])*(VTK_UNSIGNED_CHAR_MAX/scalarRange[1]);
        if (scalarType != VTK_UNSIGNED_CHAR)
        {
            m_displayImage = vtkImageShiftScale::New();
            m_displayImage->SetInput(m_reslice->GetOutput());
            m_displayImage->SetScale(scale);
            m_displayImage->SetOutputScalarTypeToUnsignedChar();

            m_displayWindowLevelColorsImage->SetInput(m_displayImage->GetOutput());
        }
        else
        {
            m_displayWindowLevelColorsImage->SetInput(m_reslice->GetOutput());
        }
        m_displayWindowLevelColorsImage->SetWindow(m_win_level[0]);
        m_displayWindowLevelColorsImage->SetLevel(m_win_level[1]);
        m_displayWindowLevelColorsImage->SetLookupTable(lut);

        m_mriActor->SetInput(m_displayWindowLevelColorsImage->GetOutput());
        m_mriRenderer->AddViewProp( m_mriActor );
        this->SetDefaultView(m_mriActor,m_mriRenderer);
        m_mriInteractor->Initialize();
        vtkInteractorStyleImage * style = vtkInteractorStyleImage::New();
        m_mriInteractor->SetInteractorStyle( style );
        style->Delete();
        lut->Delete();
        m_initRightViewDone = 1;
    }
}

void DoubleWindowFileViewer::SetRightImageWindow(int window)
{
    if (m_displayWindowLevelColorsImage)
    {
        m_win_level[0] = window;
        m_displayWindowLevelColorsImage->SetWindow(m_win_level[0]);
        this->UpdateViews();
    }
}

void DoubleWindowFileViewer::SetRightImageLevel(int level)
{
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "SetRightImageLevel :" << level <<
        ", " << m_win_level[0] << ", " << m_win_level[1] <<
        endl);
#endif
    if (m_displayWindowLevelColorsImage)
    {
        m_win_level[1] = level;
        m_displayWindowLevelColorsImage->SetLevel(m_win_level[1]);
        this->UpdateViews();
    }
}

void DoubleWindowFileViewer::SetRightImageColor(vtkLookupTable *lut)
{
    if (m_displayWindowLevelColorsImage)
    {
        vtkReal range[2];
        lut->GetTableRange( range );
        m_win_level[0] = range[1] - range[0];
        m_win_level[1] = 0.5 * (range[1] - range[0]);
        m_displayWindowLevelColorsImage->SetWindow(m_win_level[0]);
        m_displayWindowLevelColorsImage->SetLevel(m_win_level[1]);
        m_displayWindowLevelColorsImage->SetLookupTable(lut);
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "SetRightImageColor : range " << range[0] << ", " << range[1] <<
        "window, level " << m_win_level[0] << ", " << m_win_level[1] <<
        endl);
#endif
        this->UpdateViews();
    }
}

void DoubleWindowFileViewer::SetLeftImageWindow(int window)
{
    if (m_usWindowLevelColorsImage)
    {
        m_us_win_level[0] = window;
        m_usWindowLevelColorsImage->SetWindow(m_us_win_level[0]);
        this->UpdateViews();
    }
}

void DoubleWindowFileViewer::SetLeftImageLevel(int level)
{
    if (m_usWindowLevelColorsImage)
    {
        m_us_win_level[1] = level;
        m_usWindowLevelColorsImage->SetLevel(m_us_win_level[1]);
        this->UpdateViews();
    }
}

void DoubleWindowFileViewer::SetLeftImageColor(vtkLookupTable *lut)
{
    if (m_usWindowLevelColorsImage)
    {
        vtkReal range[2];
        lut->GetTableRange( range );
        m_us_win_level[0] = range[1] - range[0];
        m_us_win_level[1] = 0.5 * (range[1] - range[0]);
        m_usWindowLevelColorsImage->SetWindow(m_win_level[0]);
        m_usWindowLevelColorsImage->SetLevel(m_win_level[1]);
        m_usWindowLevelColorsImage->SetLookupTable(lut);
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "SetLeftImageColor : range " << range[0] << ", " << range[1] <<
        "window, level " << m_us_win_level[0] << ", " << m_us_win_level[1] <<
        endl);
#endif
        this->UpdateViews();
    }
}

void DoubleWindowFileViewer::SetResliceSpacing(double stepX, double stepY, double stepZ)
{
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    igstkLogMacro( DEBUG, "SetResliceSpacing: " << stepX << ", " << stepY << stepZ << endl);
#endif
    m_reslice->SetOutputSpacing(stepX, stepY, stepZ);
}

void DoubleWindowFileViewer::SetRightImageTransform(vtkTransform *t)
{
    m_imageTransform->Identity();
    m_imageTransform->Concatenate(t->GetMatrix());
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        vtkMatrix4x4 *mat = t->GetMatrix();
        igstkLogMacro( DEBUG, "DoubleWindowFileViewer input:" << "\n" <<
        mat->Element[0][0] << ", " << 
        mat->Element[0][1] << ", " <<
        mat->Element[0][2] << ", " << 
        mat->Element[0][3] << "\n" << 
        mat->Element[1][0] << ", " << 
        mat->Element[1][1] << ", " <<
        mat->Element[1][2] << ", " <<
        mat->Element[1][3] << "\n" <<
        mat->Element[2][0] << ", " << 
        mat->Element[2][1] << ", " <<
        mat->Element[2][2] << ", " <<
        mat->Element[2][3] << 
        endl);
#endif
}

void DoubleWindowFileViewer::SetRightImageTransform(vtkMatrix4x4 *transMat)
{
    m_imageTransform->Identity();
    m_imageTransform->Concatenate(transMat);
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "DoubleWindowFileViewer input:" << "\n" <<
        transMat->Element[0][0] << ", " <<
        transMat->Element[0][1] << ", " <<
        transMat->Element[0][2] << ", " <<
        transMat->Element[0][3] << "\n" <<
        transMat->Element[1][0] << ", " <<
        transMat->Element[1][1] << ", " <<
        transMat->Element[1][2] << ", " <<
        transMat->Element[1][3] << "\n" <<
        transMat->Element[2][0] << ", " <<
        transMat->Element[2][1] << ", " <<
        transMat->Element[2][2] << ", " <<
        transMat->Element[2][3] <<
        endl);
#endif
}

void DoubleWindowFileViewer::CloseButtonClicked()
{
    QWidget *parent = this->parentWidget(true);
    parent->close(TRUE);
}

void DoubleWindowFileViewer::SetDefaultView(vtkImageActor *actor, vtkRenderer *renderer)
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
        double spacing[3];
        input->GetSpacing(spacing);
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "SetDefaultView scalex, scaley:" << scalex << ", " << scaley << endl);
        igstkLogMacro( DEBUG, "SetDefaultView spacing:" << spacing[0] << ", " << spacing[1] << ", " << spacing[2] << endl);
#endif
        
        vtkCamera * cam = renderer->GetActiveCamera();
        renderer->ResetCamera();
        cam->ParallelProjectionOn();
/*        if (actor == m_usActor)*/
            cam->SetParallelScale( scaley * spacing[1]);
/*        else
        {            
            cam->SetParallelScale( scaley/spacing[1]);
            double * prevPos = cam->GetPosition();
            double * prevFocal = cam->GetFocalPoint();
            cam->SetPosition( scalex, scaley, prevPos[2] );
            cam->SetFocalPoint( scalex, scaley, prevFocal[2] );
        }*/
        renderer->Render();
    }
}

void DoubleWindowFileViewer::BlendCheckBoxToggled()
{
    m_blending = !m_blending;
    if (!m_blending)
    {
        decreaseOpacityPushButton->setEnabled( FALSE );
        increaseOpacityPushButton->setEnabled( FALSE );
        opacitySlider->setEnabled( FALSE );
        opacityStepLineEdit->setEnabled( FALSE );
        opacityStepLineEdit->setText( " " );
        if (m_blendedImage)
        {
            m_blendedImage->Delete();
            m_blendedImage = 0;
            m_greenUS->Delete();
            m_greenUS = 0;
            m_mriActor->SetInput(m_displayWindowLevelColorsImage->GetOutput());
            m_mriImageWindow->Render();
        }
    }
    else if (m_currentUsImage)
    {
        decreaseOpacityPushButton->setEnabled( TRUE );
        increaseOpacityPushButton->setEnabled( TRUE );
        opacitySlider->setEnabled( TRUE );
        opacityStepLineEdit->setEnabled( TRUE );
        opacityStepLineEdit->setText( QString::number( m_opacity ) );
        
        m_greenUS = vtkImageMapToColors::New();
        m_greenUS->SetInput(m_currentUsImage);
        m_greenUS->SetLookupTable(m_usLut);

        m_blendedImage = vtkImageBlend::New();
        opacitySlider->setValue( m_opacity );
        m_blendedImage->SetOpacity(0, 1.0 - m_opacity/100.0); // force setting, because OpacitySliderValueChanged may not be called
        m_blendedImage->SetOpacity(1, m_opacity/100.0);
        m_blendedImage->AddInput( 0, m_greenUS->GetOutput());
        m_blendedImage->AddInput( 0, m_displayWindowLevelColorsImage->GetOutput());
        m_blendedImage->Update();
        m_mriActor->SetInput(m_blendedImage->GetOutput());
        m_mriImageWindow->Render();
    }
}

void DoubleWindowFileViewer::UpdateBlendedImage()
{
    if (m_blendedImage && m_currentUsImage)
    {
        if (m_maskedImage)
        {
            m_maskedImage->SetImageInput(m_currentUsImage);
            m_maskedImage->Update();
            m_greenUS->SetInput(m_maskedImage->GetOutput());
        }
        else
            m_greenUS->SetInput(m_currentUsImage);
        m_blendedImage->Update();
    }
}

void DoubleWindowFileViewer::SetMask(const QString &maskFile)
{
    if (!maskFile.isEmpty())
    {
        MINCReader* mincReader = MINCReader::New();
        mincReader->SetFileName(maskFile.ascii());
        mincReader->SetDataType(VTK_UNSIGNED_CHAR);
        mincReader->Update();

        if (mincReader->GetErrorCode())
        {
            QMessageBox::warning( 0, "Warning", IGNS_MSG_MINC_CORRUPTED, 1, 0 );
        }
        else
        {
            vtkSmartPointer< vtkImageData > maskImage = vtkSmartPointer< vtkImageData >::New();
            maskImage->DeepCopy(mincReader->GetOutput());
            if (!m_maskedImage)
                m_maskedImage = vtkImageMask::New();
            m_maskedImage->SetMaskInput(maskImage);
        }
        mincReader->Delete();
    }
    else
    {
        if (m_maskedImage)
        {
            m_maskedImage->Delete();
            m_maskedImage = 0;
        }
    }
    this->UpdateBlendedImage();
}

void DoubleWindowFileViewer::InitializeGrid()
{
    m_grid->SetGridSize(defaultWinX, defaultWinY);
    m_grid->SetGridStep(defaultGridStep);
    m_grid->SetGridScale(m_gridScale);
    m_grid->MakeGrid();
}

void DoubleWindowFileViewer::SetGridScale(vtkReal scale[3])
{
    m_gridScale[0] = scale[0];
    m_gridScale[1] = scale[1];
    m_gridScale[2] = scale[2];
    this->InitializeGrid();
}

void DoubleWindowFileViewer::EnableGrid(int gridOn)
{
    if (!gridOn)
    {
        gridCheckBox->setChecked(FALSE);
    }
    gridCheckBox->setEnabled(gridOn);
}

void DoubleWindowFileViewer::GridCheckBoxToggled()
{
    if (m_gridOn)
    {
        restoreViewsPushButton->setEnabled( TRUE );
    }
    else
    {
        restoreViewsPushButton->setEnabled( FALSE );
    }
    DoubleViewWindow::GridCheckBoxToggled();
}

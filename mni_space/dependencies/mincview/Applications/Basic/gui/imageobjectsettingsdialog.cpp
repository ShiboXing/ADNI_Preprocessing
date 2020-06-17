/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: imageobjectsettingsdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-05-04 14:47:52 $
  Version:   $Revision: 1.47 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  Re-make from the old Simon's
  All rights reserved.

=========================================================================*/

#include "imageobjectsettingsdialog.h"

#include <qcombobox.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qmessagebox.h>
#include <qlistbox.h>

#include "qtutilities.h"
#include "imageobject.h"
#include "scenemanager.h"
#include "sceneobject.h"
#include "editvolumetransferfunctionwidget.h"
#include "vtkMatrix4x4.h"
#include "vtkQtMatrixDialog.h"
#include "usertransformations.h"
#include "view.h"

ImageObjectSettingsDialog::ImageObjectSettingsDialog( QWidget* parent, const char* name, WFlags fl )
    : ImageObjectSettingsDialogBase( parent, name, fl )
{
    if ( !name )
        setName( "ImageObjectSettingsDialog" );

    m_imageObject = 0;
    m_allPlanesHaveSameOpacity = TRUE;
    m_windowSliderStep = 1000;
    m_levelSliderStep = 1000;
    m_planeOpacitySliderStep = 1; //opacity slider is from 0 to 100%, but internally opacity is from 0 to 1
    xPlaneOpacitySlider->setMaxValue(100); 
    xPlaneOpacitySlider->setLineStep(m_planeOpacitySliderStep);
    yPlaneOpacitySlider->setMaxValue(100); 
    yPlaneOpacitySlider->setLineStep(m_planeOpacitySliderStep);
    zPlaneOpacitySlider->setMaxValue(100); 
    zPlaneOpacitySlider->setLineStep(m_planeOpacitySliderStep);
    m_maxRange[0] = 0;
    m_maxRange[1] = VTK_UNSIGNED_SHORT_MAX-1;
    appliedTransformsListBox->clear();
    windowSlider->blockSignals(TRUE);
    levelSlider->blockSignals(TRUE);
}

ImageObjectSettingsDialog::~ImageObjectSettingsDialog()
{
    if (m_imageObject)
    {
        m_imageObject->UnRegister( 0 );
    }
}

void ImageObjectSettingsDialog::SetImageObject( ImageObject * obj )
{
    if( obj == m_imageObject )
    {
        return;
    }
    
    if( m_imageObject )
    {
        m_imageObject->UnRegister( 0 );
    }
    
    m_imageObject = obj;
    
    if( m_imageObject )
    {
        m_imageObject->Register( 0 );
        ComputeWindowLevelSlidersStepsAndSetting();
        double opacity[3];
        m_imageObject->GetAllPlanesOpacity(opacity);
        if (opacity[0] == opacity[1] && opacity[0] == opacity[2])
        {
            m_allPlanesHaveSameOpacity = TRUE;
            xPlaneOpacitySlider->setValue(opacity[0] * 100.0);
            sameOpacityCheckBox->setChecked(TRUE);
            this->ShowAllOpacitySliders(FALSE);
        }
        else
        {
            m_allPlanesHaveSameOpacity = FALSE;
            xPlaneOpacitySlider->setValue(opacity[0] * 100.0);
            yPlaneOpacitySlider->setValue(opacity[1] * 100.0);
            zPlaneOpacitySlider->setValue(opacity[2] * 100.0);
            sameOpacityCheckBox->setChecked(FALSE);
            this->ShowAllOpacitySliders(TRUE);
        }
        SceneManager * man = m_imageObject->GetManager();
        for (int i=0; i < man->GetNumberOfTemplateLokupTables(); i++)
            selectColorTableComboBox->insertItem(man->GetTemplateLookupTableName(i));
        selectColorTableComboBox->setCurrentItem(m_imageObject->GetLutIndex());        
        this->SetUserTransforms();
        this->UpdateUI();
        if (m_imageObject->GetObjectType() & RESLICED_DATA_TYPE)
        {
            imageSettingsWidget->removePage(transformTabPage);
            imageSettingsWidget->removePage(fixedTransformTabPage);
            imageSettingsWidget->removePage(surfaceTabPage);
            imageSettingsWidget->removePage(volumeTabPage);
        }
        else if (m_imageObject->GetObjectType() & PREOP_TYPE)
        {
            imageSettingsWidget->removePage(transformTabPage);
            imageSettingsWidget->removePage(fixedTransformTabPage);
        }
        imageSettingsWidget->removePage(volumeTabPage); //TODO fix volume and allow the tab
   }
}

void ImageObjectSettingsDialog::SetWindowAndLevel(int window, int level)
{
    windowSlider->setValue(window);
    levelSlider->setValue(level);
    if (windowSlider->signalsBlocked())
        windowSlider->blockSignals(FALSE);
    if (levelSlider->signalsBlocked())
        levelSlider->blockSignals(FALSE);
}

void ImageObjectSettingsDialog::UpdateUI()
{
    if( m_imageObject )
    {
        this->AllControlsBlockSignals(TRUE);
        selectColorTableComboBox->setCurrentItem(m_imageObject->GetLutIndex());
        volumeCheckBox->setChecked(m_imageObject->GetViewVolume());
        int viewSurface = m_imageObject->GetViewSurface();
        if (m_imageObject->GetManager()->HasToDestroyPicker((SceneObject*)m_imageObject))
            surfaceCheckBox->setEnabled(FALSE);
        else
        {
            surfaceCheckBox->setEnabled(TRUE);
            surfaceCheckBox->setChecked(viewSurface);
        }
        if (viewSurface)
        {
            contourLineEdit->setText(QString::number(m_imageObject->GetSurfaceContourValue()));
            opacityLineEdit->setText(QString::number(m_imageObject->GetSurfaceOpacityValue()*100));
            if (m_imageObject->GetGaussianSmoothingFlag())
            {
                smoothingCheckBox->setChecked(TRUE);
                radiusFactorLineEdit->setEnabled(TRUE);
                radiusFactorLineEdit->setText(QString::number((double)m_imageObject->GetRadiusFactor()));
                standardDeviationLineEdit->setEnabled(TRUE);
                standardDeviationLineEdit->setText(QString::number((double)m_imageObject->GetStandardDeviation(), 'g',3));
            }
            else
            {
                smoothingCheckBox->setChecked(FALSE);
                radiusFactorLineEdit->setEnabled(FALSE);
                standardDeviationLineEdit->setEnabled(FALSE);
            }
        }
        double opacity[3];
        m_imageObject->GetAllPlanesOpacity(opacity);
        if (opacity[0] == opacity[1] && opacity[0] == opacity[2])
        {
            m_allPlanesHaveSameOpacity = TRUE;
            xPlaneOpacitySlider->setValue(opacity[0] * 100.0);
            sameOpacityCheckBox->setChecked(TRUE);
            this->ShowAllOpacitySliders(FALSE);
        }
        else
        {
            m_allPlanesHaveSameOpacity = FALSE;
            xPlaneOpacitySlider->setValue(opacity[0] * 100.0);
            yPlaneOpacitySlider->setValue(opacity[1] * 100.0);
            zPlaneOpacitySlider->setValue(opacity[2] * 100.0);
            sameOpacityCheckBox->setChecked(FALSE);
            this->ShowAllOpacitySliders(TRUE);
        }
        if (m_userTransforms)
        {
            transformComboBox->setCurrentItem(m_userTransforms->GetTransformIndex(m_imageObject->GetTransformName()));
            int type = m_userTransforms->GetTransformType(m_imageObject->GetTransformName());
            if (type == ABSOLUTE_TRANSFORM)
                currentTypeTextLabel->setText(IGNS_ABSOLUTE_TRANSFORM);
            else
                currentTypeTextLabel->setText(IGNS_INCREMENTAL_TRANSFORM);
        }
        this->AllControlsBlockSignals(FALSE);
    }
    else
    {
        this->setEnabled( FALSE );
    }
}

QWidget * ImageObjectSettingsDialog::CreateEditVolumeTransferFunctionDialog( )
{
    Q_ASSERT(m_imageObject);
    EditVolumeTransferFunctionWidget * res = new EditVolumeTransferFunctionWidget( 0, "EditVolumeTransferFunctionWidget", Qt::WDestructiveClose );
    res->InitVolumeTransferFunction( m_imageObject->GetVolumeProperty() );
    return res;
}

void ImageObjectSettingsDialog::ComputeWindowLevelSlidersStepsAndSetting()
{
    Q_ASSERT(m_imageObject);
    vtkReal range[2];
    m_imageObject->GetImageScalarRange(range);
    m_maxRange[0] = (double)range[0];
    m_maxRange[1] = (double)range[1];
    windowSlider->setMaxValue( (int)(m_maxRange[1]-m_maxRange[0]) ); 
    m_windowSliderStep = (int)(0.02*(m_maxRange[1] - m_maxRange[0])); //50 steps is enough 
    windowSlider->setLineStep( m_windowSliderStep );
    levelSlider->setMaxValue( (int)((m_maxRange[1]-m_maxRange[0])/2) ); 
    m_levelSliderStep = (int)(0.04*(m_maxRange[1] - m_maxRange[0])); //50 steps is enough 
    levelSlider->setLineStep( m_levelSliderStep );
    windowSlider->setValue(windowSlider->maxValue());
    levelSlider->setValue(levelSlider->maxValue());
}

void ImageObjectSettingsDialog::SelectColorTableComboBoxActivated(int index)
{
    Q_ASSERT(m_imageObject);
    m_imageObject->ChooseColorTable(index); 
}

void ImageObjectSettingsDialog::ViewVolumeCheckBoxToggled(bool viewVolumeOn)
{
    if( m_imageObject )
    {
        m_imageObject->SetViewVolume(viewVolumeOn);
        if (viewVolumeOn)
        {
            transferFunctionPushButton->setEnabled(TRUE);
            surfaceCheckBox->setEnabled(FALSE);
            contourLineEdit->setEnabled( FALSE );
            opacityLineEdit->setEnabled( FALSE );
        }
        else
        {
            transferFunctionPushButton->setEnabled(FALSE);
            surfaceCheckBox->setEnabled(TRUE);
            contourLineEdit->setEnabled( FALSE );
            opacityLineEdit->setEnabled( FALSE );
        }
    }
}

void ImageObjectSettingsDialog::DecreaseWindowPushButtonClicked()
{
    int sliderPosition, step;
    sliderPosition = windowSlider->value();
    step = windowSlider->lineStep();
    windowSlider->setValue(sliderPosition-step);
}

void ImageObjectSettingsDialog::WindowSliderValueChanged(int window)
{
    Q_ASSERT(m_imageObject);
    m_imageObject->SetUsedWindowLevel(window, levelSlider->value());
}

void ImageObjectSettingsDialog::IncreaseWindowPushButtonClicked()
{
    int sliderPosition, step;
    sliderPosition = windowSlider->value();
    step = windowSlider->lineStep();
    windowSlider->setValue(sliderPosition+step);
}

void ImageObjectSettingsDialog::DecreaseLevelPushButtonClicked()
{
    int sliderPosition, step;
    sliderPosition = levelSlider->value();
    step = levelSlider->lineStep();
    levelSlider->setValue(sliderPosition-step);
}

void ImageObjectSettingsDialog::LevelSilderValueChanged(int level)
{
    Q_ASSERT(m_imageObject);
    m_imageObject->SetUsedWindowLevel(windowSlider->value(), level);
}

void ImageObjectSettingsDialog::IncreaseLevelPushButtonClicked()
{
    int sliderPosition, step;
    sliderPosition = levelSlider->value();
    step = levelSlider->lineStep();
    levelSlider->setValue(sliderPosition+step);
}

void ImageObjectSettingsDialog::ShowAllOpacitySliders(bool showAllSliders)
{
    if(showAllSliders)
    {
        xPlaneOpacityTextLabel->setText(tr("X Plane:"));
        yPlaneOpacityTextLabel->show();
        decreaseYPlaneOpacityPushButton->show();
        yPlaneOpacitySlider->show();
        increaseYPlaneOpacityPushButton->show();
        zPlaneOpacityTextLabel->show();
        decreaseZPlaneOpacityPushButton->show();
        zPlaneOpacitySlider->show();
        increaseZPlaneOpacityPushButton->show();
    }
    else
    {
        xPlaneOpacityTextLabel->setText(tr("All:"));
        yPlaneOpacityTextLabel->hide();
        decreaseYPlaneOpacityPushButton->hide();
        yPlaneOpacitySlider->hide();
        increaseYPlaneOpacityPushButton->hide();
        zPlaneOpacityTextLabel->hide();
        decreaseZPlaneOpacityPushButton->hide();
        zPlaneOpacitySlider->hide();
        increaseZPlaneOpacityPushButton->hide();
    }
}

void ImageObjectSettingsDialog::SameOpacityCheckBoxToggled(bool flag)
{
    Q_ASSERT(m_imageObject);
    m_allPlanesHaveSameOpacity = flag;
    if (m_allPlanesHaveSameOpacity)
    {
        yPlaneOpacitySlider->setValue(xPlaneOpacitySlider->value());
        zPlaneOpacitySlider->setValue(xPlaneOpacitySlider->value());
    }
    else
    {
        double opacity[3];
        m_imageObject->GetAllPlanesOpacity(opacity);
        yPlaneOpacitySlider->setValue(opacity[1] * 100.0);
        zPlaneOpacitySlider->setValue(opacity[2] * 100.0);
    }
    ShowAllOpacitySliders(!m_allPlanesHaveSameOpacity);
}

void ImageObjectSettingsDialog::DecreaseXPlaneOpacityPushButtonClicked()
{
    int sliderPosition;
    sliderPosition = xPlaneOpacitySlider->value();
    xPlaneOpacitySlider->setValue(sliderPosition-m_planeOpacitySliderStep);
}

void ImageObjectSettingsDialog::XPlaneOpacitySliderValueChanged(int opacity)
{
    Q_ASSERT(m_imageObject);
    if (m_allPlanesHaveSameOpacity)
    {
        m_imageObject->SetAllPlanesOpacity((double)opacity/100.0);
    }
    else
    {
        m_imageObject->SetOnePlaneOpacity(0, (double)opacity/100.0);
    }
}

void ImageObjectSettingsDialog::IncreaseXPlaneOpacityPushButtonClicked()
{
    int sliderPosition;
    sliderPosition = xPlaneOpacitySlider->value();
    xPlaneOpacitySlider->setValue(sliderPosition+m_planeOpacitySliderStep);
}

void ImageObjectSettingsDialog::DecreaseYPlaneOpacityPushButtonClicked()
{
    int sliderPosition;
    sliderPosition = yPlaneOpacitySlider->value();
    yPlaneOpacitySlider->setValue(sliderPosition-m_planeOpacitySliderStep);
}

void ImageObjectSettingsDialog::YPlaneOpacitySliderValueChanged(int opacity)
{
    Q_ASSERT(m_imageObject);
    m_imageObject->SetOnePlaneOpacity(1, (double)opacity/100.0);
}

void ImageObjectSettingsDialog::IncreaseYPlaneOpacityPushButtonClicked()
{
    int sliderPosition;
    sliderPosition = yPlaneOpacitySlider->value();
    yPlaneOpacitySlider->setValue(sliderPosition+m_planeOpacitySliderStep);
}

void ImageObjectSettingsDialog::DecreaseZPlaneOpacityPushButtonClicked()
{
    int sliderPosition;
    sliderPosition = zPlaneOpacitySlider->value();
    zPlaneOpacitySlider->setValue(sliderPosition-m_planeOpacitySliderStep);
}

void ImageObjectSettingsDialog::ZPlaneOpacitySliderValueChanged(int opacity)
{
    Q_ASSERT(m_imageObject);
    m_imageObject->SetOnePlaneOpacity(2, (double)opacity/100.0);
}

void ImageObjectSettingsDialog::IncreaseZPlaneOpacityPushButtonClicked()
{
    int sliderPosition;
    sliderPosition = zPlaneOpacitySlider->value();
    zPlaneOpacitySlider->setValue(sliderPosition+m_planeOpacitySliderStep);
}

void ImageObjectSettingsDialog::ResetPlanesPushButtonClicked()
{
    if( m_imageObject )
    {
        SceneManager * man = m_imageObject->GetManager();
        if (man->GetViewMode() != VIEW_SYNCHRONIZED)
            m_imageObject->ResetPlanes();
        else
            man->AllImagesResetPlanes();
    }
}

void ImageObjectSettingsDialog::OutlineCheckBoxToggled(bool viewOn)
{
    Q_ASSERT(m_imageObject);
    if (viewOn && m_imageObject->GetViewOutline() == 0 || !viewOn && m_imageObject->GetViewOutline() == 1 )
        m_imageObject->SetViewOutline( viewOn? 1 : 0  );
}

void ImageObjectSettingsDialog::XPlaneCheckBoxToggled(bool viewOn)
{
    Q_ASSERT(m_imageObject);
    if (viewOn && m_imageObject->GetViewPlane(0) == 0 || !viewOn && m_imageObject->GetViewPlane(0) == 1 )
        m_imageObject->SetViewPlane( 0, viewOn? 1 : 0  );
}

void ImageObjectSettingsDialog::YPlaneCheckBoxToggled(bool viewOn)
{
    Q_ASSERT(m_imageObject);
    if (viewOn && m_imageObject->GetViewPlane(1) == 0 || !viewOn && m_imageObject->GetViewPlane(1) == 1 )
        m_imageObject->SetViewPlane( 1, viewOn? 1 : 0  );
}

void ImageObjectSettingsDialog::ZPlaneCheckBoxToggled(bool viewOn)
{
    Q_ASSERT(m_imageObject);
    if (viewOn && m_imageObject->GetViewPlane(2) == 0 || !viewOn && m_imageObject->GetViewPlane(2) == 1 )
        m_imageObject->SetViewPlane( 2, viewOn? 1 : 0 );
}

void ImageObjectSettingsDialog::Planes3DCheckBoxToggled(bool viewOn)
{
    Q_ASSERT(m_imageObject);
    if (viewOn && m_imageObject->GetView3DPlanes() == 0 || !viewOn && m_imageObject->GetView3DPlanes() == 1 )
        m_imageObject->SetView3DPlanes( viewOn? 1 : 0 );
}

void ImageObjectSettingsDialog::FrontPushButtonClicked()
{
    Q_ASSERT(m_imageObject);
    SceneManager * manager = m_imageObject->GetManager();
    manager->SetStandardView(SV_FRONT);
}

void ImageObjectSettingsDialog::BackPushButtonClicked()
{
    Q_ASSERT(m_imageObject);
    SceneManager * manager = m_imageObject->GetManager();
    manager->SetStandardView(SV_BACK);
}

void ImageObjectSettingsDialog::LeftPushButtonClicked()
{
    Q_ASSERT(m_imageObject);
    SceneManager * manager = m_imageObject->GetManager();
    manager->SetStandardView(SV_LEFT);
}

void ImageObjectSettingsDialog::RightPushButtonClicked()
{
    Q_ASSERT(m_imageObject);
    SceneManager * manager = m_imageObject->GetManager();
    manager->SetStandardView(SV_RIGHT);
}

void ImageObjectSettingsDialog::TopPushButtonClicked()
{
    Q_ASSERT(m_imageObject);
    SceneManager * manager = m_imageObject->GetManager();
    manager->SetStandardView(SV_TOP);
}

void ImageObjectSettingsDialog::BottomPushButtonClicked()
{
    Q_ASSERT(m_imageObject);
    SceneManager * manager = m_imageObject->GetManager();
    manager->SetStandardView(SV_BOTTOM);
}

void ImageObjectSettingsDialog::SurfaceCheckBoxToggled(bool viewSurfaceOn)
{
    Q_ASSERT(m_imageObject);
    m_imageObject->SetViewSurface(viewSurfaceOn);
    if (m_imageObject->GetViewError())
    {
        m_imageObject->ResetViewError();
        QMessageBox::warning( this, "Viewer", "At least one of the step is negative in the image.\nThe surface may not be displayed.\nTo changed use autocrop - isostep 1 <input_file> <output_file>\n" );
        surfaceCheckBox->setChecked(FALSE);
        contourLineEdit->setEnabled( FALSE );
        opacityLineEdit->setEnabled( FALSE );
        smoothingCheckBox->setChecked(FALSE);
        smoothingCheckBox->setEnabled(FALSE);
        return;
    }
    if (viewSurfaceOn)
    {
        double contourValue = m_imageObject->GetSurfaceContourValue( );
        contourLineEdit->setEnabled( TRUE );
        opacityLineEdit->setEnabled( TRUE );
        contourLineEdit->setText( QString::number( contourValue, 'g', 7  ) );
        double opacityValue = m_imageObject->GetSurfaceOpacityValue() * 100.0;
        opacityLineEdit->setText( QString::number( opacityValue, 'g', 3 ) );
        smoothingCheckBox->setEnabled(TRUE);
    }
    else
    {
        smoothingCheckBox->setChecked(FALSE);
        smoothingCheckBox->setEnabled(FALSE);
        contourLineEdit->setEnabled( FALSE );
        opacityLineEdit->setEnabled( FALSE );
    }
}

void ImageObjectSettingsDialog::ContourLineEditTextChanged()
{
    Q_ASSERT(m_imageObject);
    QString t = contourLineEdit->text();
    double newScalars = t.toDouble();
    if (newScalars > 0)
        m_imageObject->SetSurfaceContourValue( newScalars );
}

void ImageObjectSettingsDialog::OpacityLineEditTextChanged()
{
    Q_ASSERT(m_imageObject);
    QString t = opacityLineEdit->text();
    double newOpacity = t.toDouble();
    if (newOpacity < 0.0)
    {
        newOpacity = 0.0;
    }
    if (newOpacity > 100.0)
    {
        newOpacity = 100.0;
    }
    opacityLineEdit->setText( QString::number( newOpacity, 'g', 3 ) );
    m_imageObject->SetSurfaceOpacityValue( newOpacity/100.0 );
}

void ImageObjectSettingsDialog::SmoothingCheckBoxToggled(bool smooth)
{
    Q_ASSERT(m_imageObject);
    if (smooth)
    {
        radiusFactorLineEdit->setEnabled(TRUE);
        radiusFactorLineEdit->setText(QString::number((double)m_imageObject->GetRadiusFactor()));
        standardDeviationLineEdit->setEnabled(TRUE);
        standardDeviationLineEdit->setText(QString::number((double)m_imageObject->GetStandardDeviation(),'g',3));
    }
    else
    {
        radiusFactorLineEdit->setEnabled(FALSE);
        standardDeviationLineEdit->setEnabled(FALSE);
    }
    m_imageObject->SetGaussianSmoothingFlag(smooth? 1 : 0);
    QString t = standardDeviationLineEdit->text();
    double deviation = t.toDouble();
    m_imageObject->SetStandardDeviation(deviation);
}

void ImageObjectSettingsDialog::StandardDeviationLineEditTextChanged()
{
    Q_ASSERT(m_imageObject);
    QString t = standardDeviationLineEdit->text();
    double deviation = t.toDouble();
    m_imageObject->SetStandardDeviation(deviation);
}

void ImageObjectSettingsDialog::RadiusFactorLineEditTextChanged()
{
    Q_ASSERT(m_imageObject);
    QString t = radiusFactorLineEdit->text();
    double radius = t.toDouble();
    m_imageObject->SetRadiusFactor(radius);
}

void ImageObjectSettingsDialog::TransferFunctionPushButtonClicked()
{
    if( m_imageObject )
    {
        if (!m_imageObject->GetViewVolume())
            return;
        QWidget *res = GetWidgetByName( "EditVolumeTransferFunctionWidget" );
        if (!res)
        {
            res = this->CreateEditVolumeTransferFunctionDialog();
            res->show();
        }
        else
        {
            res->show();
            res->raise();
            res->setActiveWindow();
        }
    }
}

//void ImageObjectSettingsDialog::ImageSettingsWidgetTabChanged(const QString & tabLabel)

void ImageObjectSettingsDialog::ImageSettingsWidgetTabChanged(QWidget * page)
{
    Q_ASSERT(m_imageObject);
    this->AllControlsBlockSignals(TRUE);
    if (page == viewTabPage)
    {
        outlineCheckBox->setChecked((bool)m_imageObject->GetViewOutline());
        xPlaneCheckBox->setChecked((bool)m_imageObject->GetViewPlane(0));
        yPlaneCheckBox->setChecked((bool)m_imageObject->GetViewPlane(1));
        zPlaneCheckBox->setChecked((bool)m_imageObject->GetViewPlane(2));
        planes3DCheckBox->setChecked((bool)m_imageObject->GetView3DPlanes());
    }
    else if (page == colorTabPage)
    {
        double windowLevel[2];
        m_imageObject->GetUsedWindowLevel(windowLevel);
        levelSlider->setValue(windowLevel[1]);
        windowSlider->setValue(windowLevel[0]);      
    }
    else if (page == surfaceTabPage)
    {
        if (!m_imageObject->GetViewVolume())
        {
            if (m_imageObject->GetManager()->HasToDestroyPicker((SceneObject*)m_imageObject))
                surfaceCheckBox->setEnabled(FALSE);
            else
                surfaceCheckBox->setEnabled(TRUE);
            if (m_imageObject->GetViewSurface())
            {
                surfaceCheckBox->setChecked(TRUE);
                contourLineEdit->setEnabled( TRUE );
                opacityLineEdit->setEnabled( TRUE );
                smoothingCheckBox->setEnabled(TRUE);
                if (m_imageObject->GetGaussianSmoothingFlag())
                {
                    smoothingCheckBox->setChecked(TRUE);
                    radiusFactorLineEdit->setEnabled(TRUE);
                    radiusFactorLineEdit->setText(QString::number((double)m_imageObject->GetRadiusFactor()));
                    standardDeviationLineEdit->setEnabled(TRUE);
                    standardDeviationLineEdit->setText(QString::number((double)m_imageObject->GetStandardDeviation(), 'g',3));
                }
                else
                {
                    smoothingCheckBox->setChecked(FALSE);
                    radiusFactorLineEdit->setEnabled(FALSE);
                    standardDeviationLineEdit->setEnabled(FALSE);
                }
            }
            else
            {
                surfaceCheckBox->setChecked(FALSE);
                contourLineEdit->setEnabled( FALSE );
                opacityLineEdit->setEnabled( FALSE );
                smoothingCheckBox->setEnabled(FALSE);
                smoothingCheckBox->setChecked(FALSE);
                radiusFactorLineEdit->setEnabled(FALSE);
                standardDeviationLineEdit->setEnabled(FALSE);
            }
        }
        else
        {
            surfaceCheckBox->setEnabled(FALSE);
            surfaceCheckBox->setChecked(FALSE);
            contourLineEdit->setEnabled( FALSE );
            opacityLineEdit->setEnabled( FALSE );
            smoothingCheckBox->setEnabled(FALSE);
            smoothingCheckBox->setChecked(FALSE);
            radiusFactorLineEdit->setEnabled(FALSE);
            standardDeviationLineEdit->setEnabled(FALSE);
        }
    }
    else if (page == volumeTabPage)
    {
        if (!m_imageObject->GetViewSurface())
        {
            volumeCheckBox->setEnabled(TRUE);
            if (m_imageObject->GetViewVolume())
            {
                volumeCheckBox->setChecked(TRUE);
                transferFunctionPushButton->setEnabled(TRUE);
            }
            else
                transferFunctionPushButton->setEnabled(FALSE);
        }
        else
        {
            volumeCheckBox->setEnabled(FALSE);
            transferFunctionPushButton->setEnabled(FALSE);
        }
    }
    else if (page == transformTabPage || page == fixedTransformTabPage)
    {
        SetAppliedTransforms(m_imageObject->GetAppliedTransforms());
    }
    this->AllControlsBlockSignals(FALSE);
}

void ImageObjectSettingsDialog::TransformFromListSelected(const QString& transformName)
{
    if (m_userTransforms)
    {
        int type = m_userTransforms->GetTransformType(transformName);
        if (type == ABSOLUTE_TRANSFORM)
            currentTypeTextLabel->setText(IGNS_ABSOLUTE_TRANSFORM);
        else
            currentTypeTextLabel->setText(IGNS_INCREMENTAL_TRANSFORM);
        emit TransformationChanged(transformName, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::UndoLastTransformation()
{
    if (m_userTransforms)
    {
        emit UndoTransformation( imageSettingsWidget->currentPageIndex());
    }
}


void ImageObjectSettingsDialog::ShowTransformationMatrix()
{
    if (m_userTransforms)
    {
        QString name = transformComboBox->currentText();
        vtkMatrix4x4 *tmpMat = m_userTransforms->GetTransformMatrix(name);
        vtkQtMatrixDialog * dlg;
        dlg = new vtkQtMatrixDialog( tmpMat, true, 0, "" );
        dlg->show();
    }
}

void ImageObjectSettingsDialog::SetUserTransforms()
{
    m_userTransforms = m_imageObject->GetManager()->GetUserTransforms();
    if (m_userTransforms)
    {
        QStringList tmp;
        m_userTransforms->GetTransformList(tmp);
        transformComboBox->clear();
        transformComboBox->insertStringList(tmp);
        transformComboBox->setCurrentItem(0);
        int type = m_userTransforms->GetTransformType(0);
        if (type == ABSOLUTE_TRANSFORM)
            currentTypeTextLabel->setText(IGNS_ABSOLUTE_TRANSFORM);
        else
            currentTypeTextLabel->setText(IGNS_INCREMENTAL_TRANSFORM);
    }
    else
    {
        imageSettingsWidget->removePage(transformTabPage);
        imageSettingsWidget->removePage(fixedTransformTabPage);
    }
}

void ImageObjectSettingsDialog::SetAppliedTransforms(std::vector<int> &transformIds)
{
    if (m_userTransforms)
    {
        appliedTransformsListBox->clear();
    
        std::vector<int>::iterator it = transformIds.begin();
        QString tmp;
        for( ; it != transformIds.end(); ++it )
        {
            tmp = m_userTransforms->GetTransformName((*it));
            appliedTransformsListBox->insertItem(tmp);
        }
    }
}

void ImageObjectSettingsDialog::UpdatePlaneSettings(int viewType, int show)
{
    xPlaneCheckBox->blockSignals( TRUE );
    yPlaneCheckBox->blockSignals( TRUE );
    zPlaneCheckBox->blockSignals( TRUE );
    planes3DCheckBox->blockSignals( TRUE );
    outlineCheckBox->blockSignals( TRUE );
    switch(viewType)
    {
    case SAGITTAL_VIEW_TYPE:
        xPlaneCheckBox->setChecked(show==1);
        break;
    case CORONAL_VIEW_TYPE:
        yPlaneCheckBox->setChecked(show==1);
        break;
    case TRANSVERSE_VIEW_TYPE:
        zPlaneCheckBox->setChecked(show==1);
        break;
    case THREED_VIEW_TYPE:
        planes3DCheckBox->setChecked(show==1);
        break;
    case OUTLINE_VIEW_TYPE:
        outlineCheckBox->setChecked(show==1);
        break;
    }
    xPlaneCheckBox->blockSignals( FALSE );
    yPlaneCheckBox->blockSignals( FALSE );
    zPlaneCheckBox->blockSignals( FALSE );
    planes3DCheckBox->blockSignals( FALSE );
    outlineCheckBox->blockSignals( FALSE );
}

void ImageObjectSettingsDialog::XPlus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_PLUS_1, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::YPlus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_PLUS_1, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::ZPlus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_PLUS_1, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::XPlus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_PLUS_5, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::YPlus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_PLUS_5, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::ZPlus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_PLUS_5, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::XMinus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_MINUS_5, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::YMinus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_MINUS_5, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::ZMinus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_MINUS_5, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::XMinus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_MINUS_1, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::YMinus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_MINUS_1, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::ZMinus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_MINUS_1, imageSettingsWidget->currentPageIndex());
    }
}

void ImageObjectSettingsDialog::ShowLastUsedWidget(int index)
{
    imageSettingsWidget->showPage(imageSettingsWidget->page(index));
}

void ImageObjectSettingsDialog::AllControlsBlockSignals(bool yes)
{
    zPlaneCheckBox->blockSignals(yes);
    xPlaneCheckBox->blockSignals(yes);
    outlineCheckBox->blockSignals(yes);
    yPlaneCheckBox->blockSignals(yes);
    planes3DCheckBox->blockSignals(yes);
    sameOpacityCheckBox->blockSignals(yes);
    smoothingCheckBox->blockSignals(yes);
    surfaceCheckBox->blockSignals(yes);
    volumeCheckBox->blockSignals(yes);
    xPlaneOpacitySlider->blockSignals(yes);
    yPlaneOpacitySlider->blockSignals(yes);
    yPlaneOpacitySlider->blockSignals(yes);
    levelSlider->blockSignals(yes);
    windowSlider->blockSignals(yes);
}

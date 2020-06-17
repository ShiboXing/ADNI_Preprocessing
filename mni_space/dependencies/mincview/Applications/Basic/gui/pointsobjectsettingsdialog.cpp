/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: pointsobjectsettingsdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-04-28 19:05:52 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#include "pointsobjectsettingsdialog.h"

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcolordialog.h>
#include <qlabel.h>
#include <qslider.h>
#include <qlineedit.h>
#include "qtutilities.h"
#include "pointlistdialog.h"
#include "scenemanager.h"
#include "usertransformations.h"
#include "vtkQtMatrixDialog.h"

PointsObjectSettingsDialog::PointsObjectSettingsDialog( QWidget* parent, const char* name, WFlags fl )
    : PointsObjectSettingsDialogBase( parent, name, fl ),
    m_points(0)
{
    setName( "PointsObjectSettingsDialog" );
}

PointsObjectSettingsDialog::~PointsObjectSettingsDialog()
{
    if(m_points)
    {
        this->disconnect();
        m_points->UnRegister( 0 );
    }
    m_points = 0;
    m_userTransforms = 0;
}

void PointsObjectSettingsDialog::UpdateUI()
{
    if (m_points)
    {
        pointRadiusSpinBox->blockSignals(TRUE);
        pointRadiusSpinBox->setValue((int)m_points->GetPointRadius());
        pointRadiusSpinBox->blockSignals(FALSE);
        labelSizeSpinBox->blockSignals(TRUE);
        labelSizeSpinBox->setValue((int)m_points->GetLabelSize());
        labelSizeSpinBox->blockSignals(FALSE);
        numberOfPointsLineEdit->setText(QString::number(m_points->GetNumberOfPoints()));
        if(m_points->GetImageObject() &&
            !m_points->GetManager()->GetRegistrationFlag() &&
            m_points->GetPointsType() != REGISTRATION_TARGET)
            selectionModePushButton->setEnabled(TRUE);
        else
            selectionModePushButton->setEnabled(FALSE);
        if (m_points->GetCorrectionMode() > 0)
        {
            selectionModePushButton->setText("Exit Correction Mode");
        }
        else
        {
            selectionModePushButton->setText("Enter Correction Mode");
        }
        if (m_points->GetObjectType() & PREOP_TYPE || m_points->GetObjectManagedBySystem())
        {
            settingsTabWidget->removePage(transformTab);
            settingsTabWidget->removePage(fixedTransformTab);
        }
        QWidget *w = GetWidgetByName( "PointListDialog" );
        if (w && !w->isHidden())
            showListCheckBox->setChecked(TRUE);
        else
            showListCheckBox->setChecked(FALSE);
        vtkReal * color = m_points->GetDisabledColor();
        QColor disabled( (int)(color[0] * 255), (int)(color[1] * 255), (int)(color[2] * 255) );
        disabledColorSwatch->setBackgroundColor( disabled );
        color = m_points->GetSelectedColor();
        QColor selected( (int)(color[0] * 255), (int)(color[1] * 255), (int)(color[2] * 255) );
        selectedColorSwatch->setBackgroundColor( selected );
        color = m_points->GetEnabledColor();
        QColor enabled( (int)(color[0] * 255), (int)(color[1] * 255), (int)(color[2] * 255) );
        enabledColorSwatch->setBackgroundColor( enabled );
        this->UpdateOpacityUI();
        if (m_userTransforms)
        {
            transformComboBox->setCurrentItem(m_userTransforms->GetTransformIndex(m_points->GetTransformName()));
            int type = m_userTransforms->GetTransformType(m_points->GetTransformName());
            if (type == ABSOLUTE_TRANSFORM)
                currentTypeTextLabel->setText(IGNS_ABSOLUTE_TRANSFORM);
            else
                currentTypeTextLabel->setText(IGNS_INCREMENTAL_TRANSFORM);
        }
    }
}

void PointsObjectSettingsDialog::UpdateSettings()
{
    this->UpdateUI();
    emit UpdateList();
}

void PointsObjectSettingsDialog::ShowLastUsedWidget(int index)
{
    settingsTabWidget->showPage(settingsTabWidget->page(index));
}

void PointsObjectSettingsDialog::ResetPointListCheckBox()
{
    showListCheckBox->setChecked(FALSE);
    this->UpdateUI();
}

void PointsObjectSettingsDialog::SettingsTabWidgetCurrentChanged(QWidget* currentPage)
{
    if (currentPage == transformTab || currentPage == fixedTransformTab)
    {
        SetAppliedTransforms(m_points->GetAppliedTransforms());
    }
}

void PointsObjectSettingsDialog::SetPointsObject(PointsObject *obj)
{
    if(m_points == obj)
    {
        return;
    }
    
    if(m_points)
    {
        this->disconnect();
        m_points->UnRegister( 0 );
    }
    
    m_points = obj;
    
    if(m_points)
    {
        m_points->Register( 0 );
        this->SetUserTransforms();
        connect(m_points, SIGNAL(PointsChanged()), this, SLOT(UpdateSettings()));
        connect(m_points, SIGNAL(PointRemoved()), this, SLOT(UpdateSettings()));
    }
    this->UpdateUI();
}

// Properties
void PointsObjectSettingsDialog::LabelSizeChanged(int size)
{
    if(m_points)
    {
        m_points->SetLabelSize((double)size);
        m_points->UpdatePoints();
    }
}

void PointsObjectSettingsDialog::PointRadiusChanged(int size)
{
    if(m_points)
    {
        m_points->SetPointRadius((double)size);
        m_points->UpdatePoints();
    }
}

void PointsObjectSettingsDialog::HideLabelsPushButtonClicked()
{
    if(m_points)
    {
        if(m_points->GetShowLabel() > 0) //we have to hide the labels
        {
            m_points->SetShowLabel(0);
            hideLabelsPushButton->setText(tr("Show Labels"));
        }
        else
        {
            m_points->SetShowLabel(1);
            hideLabelsPushButton->setText(tr("Hide Labels"));
        }
    }
}

void PointsObjectSettingsDialog::SelectionModePushButtonClicked()
{
    if(m_points)
    {
        if (m_points->GetCorrectionMode() > 0)
        {
            selectionModePushButton->setText("Enter Correction Mode");
            m_points->SetCorrectionMode(0);
        }
        else
        {
            selectionModePushButton->setText("Exit Correction Mode");
            m_points->SetCorrectionMode(1);
        }
    }
}

void PointsObjectSettingsDialog::ShowListOfPointsCheckBoxToggled(bool show )
{
    QWidget *w = GetWidgetByName( "PointListDialog" );
    if(m_points && show)
    {
        if( !w )
        {
            PointListDialog *listDialog = new PointListDialog(0, "PointListDialog", FALSE, Qt::WDestructiveClose | Qt::WStyle_StaysOnTop);
            listDialog->SetPoints(m_points);
            if (!m_points->GetEnableRefining())
                m_points->ActivateCursorOnAllPlanes(1);
            int n = m_points->GetHighlightedPointIndex();
            if (n >= 0)
                listDialog->HighlightPoint(n);
            listDialog->UpdateUI();
            connect(listDialog, SIGNAL(PointListClosed()), this, SLOT(ResetPointListCheckBox()));
            connect(listDialog, SIGNAL(NumberOfPointsChanged()), this, SLOT(UpdateSettings()));
            connect(this, SIGNAL(UpdateList()), listDialog, SLOT(UpdateUI()));
            connect(m_points, SIGNAL(PointsChanged()), listDialog, SLOT(UpdateUI()));
            listDialog->show();
        }
        else
        {
            w->showNormal();
            w->raise();
            w->setActiveWindow();
        }
    }
    else if (w)
        w->hide();
}

//Color
void PointsObjectSettingsDialog::OpacitySliderValueChanged(int value)
{
    double newOpacity = ((double)value) / 100.0;
    m_points->SetOpacity( newOpacity );
    this->UpdateOpacityUI();
}

void PointsObjectSettingsDialog::OpacityTextEditChanged(const QString& text)
{
    double newOpacity = text.toDouble();
    m_points->SetOpacity( newOpacity );
    this->UpdateOpacityUI();
}

void PointsObjectSettingsDialog::UpdateOpacityUI()
{
    opacitySlider->blockSignals( TRUE );
    opacityEdit->blockSignals( TRUE );
    double opacity = m_points->GetOpacity();
    opacitySlider->setValue( (int)( opacity * 100 ) );
    opacityEdit->setText( QString::number( opacity, 'g', 2 ) );
    opacitySlider->blockSignals( FALSE );
    opacityEdit->blockSignals( FALSE );
}

void PointsObjectSettingsDialog::DisabledColorSetButtonClicked()
{
    vtkReal * oldColor = m_points->GetDisabledColor();
    QColor initial( (int)(oldColor[0] * 255), (int)(oldColor[1] * 255), (int)(oldColor[2] * 255) );
    QColor newColor = QColorDialog::getColor( initial );
    double newColorfloat[3] = { 1, 1, 1 };
    disabledColorSwatch->setBackgroundColor( newColor );
    newColorfloat[0] = vtkReal( newColor.red() ) / 255.0; 
    newColorfloat[1] = vtkReal( newColor.green() ) / 255.0;
    newColorfloat[2] = vtkReal( newColor.blue() ) / 255.0;
    m_points->SetDisabledColor( newColorfloat );
}

void PointsObjectSettingsDialog::EnabledColorSetButtonClicked()
{
    vtkReal * oldColor = m_points->GetEnabledColor();
    QColor initial( (int)(oldColor[0] * 255), (int)(oldColor[1] * 255), (int)(oldColor[2] * 255) );
    QColor newColor = QColorDialog::getColor( initial );
    double newColorfloat[3] = { 1, 1, 1 };
    enabledColorSwatch->setBackgroundColor( newColor );
    newColorfloat[0] = vtkReal( newColor.red() ) / 255.0; 
    newColorfloat[1] = vtkReal( newColor.green() ) / 255.0;
    newColorfloat[2] = vtkReal( newColor.blue() ) / 255.0;
    m_points->SetEnabledColor( newColorfloat );
}

void PointsObjectSettingsDialog::SelectedColorSetButtonClicked()
{
    vtkReal * oldColor = m_points->GetSelectedColor();
    QColor initial( (int)(oldColor[0] * 255), (int)(oldColor[1] * 255), (int)(oldColor[2] * 255) );
    QColor newColor = QColorDialog::getColor( initial );
    double newColorfloat[3] = { 1, 1, 1 };
    selectedColorSwatch->setBackgroundColor( newColor );
    newColorfloat[0] = vtkReal( newColor.red() ) / 255.0; 
    newColorfloat[1] = vtkReal( newColor.green() ) / 255.0;
    newColorfloat[2] = vtkReal( newColor.blue() ) / 255.0;
    m_points->SetSelectedColor( newColorfloat );
}

// transforms
void PointsObjectSettingsDialog::SetAppliedTransforms(std::vector<int> &transformIds)
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

void PointsObjectSettingsDialog::SetUserTransforms()
{
    m_userTransforms = m_points->GetManager()->GetUserTransforms();
    if (m_userTransforms)
    {
        QStringList tmp;
        m_userTransforms->GetTransformList(tmp);
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
        settingsTabWidget->removePage(transformTab);
        settingsTabWidget->removePage(fixedTransformTab);
    }
}

void PointsObjectSettingsDialog::TransformFromListSelected(const QString& transformName)
{
    if (m_userTransforms)
    {
        int type = m_userTransforms->GetTransformType(transformName);
        if (type == ABSOLUTE_TRANSFORM)
            currentTypeTextLabel->setText(IGNS_ABSOLUTE_TRANSFORM);
        else
            currentTypeTextLabel->setText(IGNS_INCREMENTAL_TRANSFORM);
        emit TransformationChanged(transformName, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::ShowTransformationMatrix()
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

void PointsObjectSettingsDialog::UndoLastTransformation()
{
    if (m_userTransforms)
    {
        emit UndoTransformation(settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::XMinus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_MINUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::YMinus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_MINUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::ZMinus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_MINUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::XMinus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_MINUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::YMinus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_MINUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::ZMinus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_MINUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::XPlus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_PLUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::YPlus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_PLUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::ZPlus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_PLUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::XPlus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_PLUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::YPlus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_PLUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PointsObjectSettingsDialog::ZPlus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_PLUS_5, settingsTabWidget->currentPageIndex());
    }
}


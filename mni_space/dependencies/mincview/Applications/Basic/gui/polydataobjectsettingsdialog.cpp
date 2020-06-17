#include "polydataobjectsettingsdialog.h"

#include <qbuttongroup.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qcolordialog.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qspinbox.h>

#include "polydataobject.h"
#include "vtkReal.h"
#include "vtkProperty.h"
#include "vtkMatrix4x4.h"
#include "vtkQtMatrixDialog.h"
#include "scenemanager.h"
#include "usertransformations.h"

PolyDataObjectSettingsDialog::PolyDataObjectSettingsDialog( QWidget* parent, const char* name, WFlags fl )
: PolyDataObjectSettingsDialogBase( parent, name, fl )
{
    if ( !name )
        setName( "PolyDataObjectSettingsDialog" );
    m_object = 0;
    m_userTransforms = 0;
    appliedTransformsListBox->clear();
}

PolyDataObjectSettingsDialog::~PolyDataObjectSettingsDialog()
{
    if( m_object )
    {
        m_object->UnRegister( 0 );
    }
}

void PolyDataObjectSettingsDialog::SetPolyDataObject( PolyDataObject * object )
{
    if( object == m_object )
    {
        return;
    }
    
    if( m_object )
    {
        m_object->UnRegister( 0 );
    }
    
    m_object = object;
    
    if( m_object )
    {
        m_object->Register( 0 );
        this->SetUserTransforms();
    }
    
    this->UpdateUI();
}


void PolyDataObjectSettingsDialog::OpacitySliderValueChanged( int value )
{
    double newOpacity = ((double)value) / 100.0;
    m_object->SetOpacity( newOpacity );
    this->UpdateOpacityUI();
}


void PolyDataObjectSettingsDialog::OpacityEditTextChanged( const QString & text )
{
    double newOpacity = text.toDouble();
    m_object->SetOpacity( newOpacity );
    this->UpdateOpacityUI();
}

void PolyDataObjectSettingsDialog::DisplayModeButtonPressed( int buttonPressed )
{
    m_object->SetRenderingMode( buttonPressed );
}

void PolyDataObjectSettingsDialog::UpdateSettings()
{
    this->UpdateUI();
}

void PolyDataObjectSettingsDialog::UpdateUI()
{
    // update Scalar visibility
    this->scalarVisibilityCheckBox->setChecked( m_object->GetScalarsVisible() );
    
    // Update color ui
    vtkReal * color = m_object->GetProperty()->GetColor();
    this->colorSwatch->setBackgroundColor( QColor( (int)(color[0] * 255), (int)(color[1] * 255), (int)(color[2] * 255 )) );
    
    this->displayModeGroupBox->setButton( m_object->GetRenderingMode() );
    if (m_object->GetObjectType() & PREOP_TYPE || m_object->GetObjectManagedBySystem())
    {
        settingsTabWidget->removePage(transformTab);
        settingsTabWidget->removePage(fixedTransformTab);
    }
    if (m_object->GetDTITracks())
    {
        radiusLineEdit->setText(QString::number(m_object->GetDTITubeRadius()));
        resolutionSpinBox->setValue(m_object->GetDTITubeResolution());
    }
    else
        settingsTabWidget->removePage(tracksTab);
            
    this->UpdateOpacityUI();
    if (m_userTransforms)
    {
        transformComboBox->setCurrentItem(m_userTransforms->GetTransformIndex(m_object->GetTransformName()));
        int type = m_userTransforms->GetTransformType(m_object->GetTransformName());
        if (type == ABSOLUTE_TRANSFORM)
            currentTypeTextLabel->setText(IGNS_ABSOLUTE_TRANSFORM);
        else
            currentTypeTextLabel->setText(IGNS_INCREMENTAL_TRANSFORM);
    }
}

void PolyDataObjectSettingsDialog::UpdateOpacityUI()
{
    this->opacitySlider->blockSignals( TRUE );
    this->opacityEdit->blockSignals( TRUE );
    double opacity = m_object->GetOpacity();
    this->opacitySlider->setValue( (int)( opacity * 100 ) );
    this->opacityEdit->setText( QString::number( opacity, 'g', 2 ) );
    this->opacitySlider->blockSignals( FALSE );
    this->opacityEdit->blockSignals( FALSE );
}

void PolyDataObjectSettingsDialog::ColorSwatchClicked()
{
    vtkReal * oldColor = m_object->GetProperty()->GetColor();
    QColor initial( (int)(oldColor[0] * 255), (int)(oldColor[1] * 255), (int)(oldColor[2] * 255) );
    QColor newColor = QColorDialog::getColor( initial );
    double newColorfloat[3] = { 1, 1, 1 };
    this->colorSwatch->setBackgroundColor( newColor );
    newColorfloat[0] = vtkReal( newColor.red() ) / 255.0; 
    newColorfloat[1] = vtkReal( newColor.green() ) / 255.0;
    newColorfloat[2] = vtkReal( newColor.blue() ) / 255.0;
    m_object->SetColor( newColorfloat );
}

void PolyDataObjectSettingsDialog::ScalarVisibilityCheckBoxToggled( bool isOn )
{
    m_object->SetScalarsVisible( isOn ? 1 : 0 );
}

void PolyDataObjectSettingsDialog::TransformFromListSelected(const QString& transformName)
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

void PolyDataObjectSettingsDialog::UndoLastTransformation()
{
    if (m_userTransforms)
    {
        emit UndoTransformation(settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::ShowTransformationMatrix()
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

void PolyDataObjectSettingsDialog::SetUserTransforms()
{
    m_userTransforms = m_object->GetManager()->GetUserTransforms();
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

void PolyDataObjectSettingsDialog::SetAppliedTransforms(std::vector<int> &transformIds)
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

void PolyDataObjectSettingsDialog::SettingsTabWidgetCurrentChanged(QWidget* currentPage)
{
    if (currentPage == transformTab || currentPage == fixedTransformTab)
    {
        SetAppliedTransforms(m_object->GetAppliedTransforms());
    }
}

void PolyDataObjectSettingsDialog::XPlus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_PLUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::YPlus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_PLUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::ZPlus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_PLUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::XPlus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_PLUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::YPlus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_PLUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::ZPlus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_PLUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::XMinus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_MINUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::YMinus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_MINUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::ZMinus5PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_MINUS_5, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::XMinus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_X_MINUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::YMinus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Y_MINUS_1, settingsTabWidget->currentPageIndex());
    }
}

void PolyDataObjectSettingsDialog::ZMinus1PushButtonClicked()
{
    if (m_userTransforms)
    {
        emit TransformationChanged(IGNS_Z_MINUS_1, settingsTabWidget->currentPageIndex());
    }
}
void PolyDataObjectSettingsDialog::ShowLastUsedWidget(int index)
{
    settingsTabWidget->showPage(settingsTabWidget->page(index));
}

void PolyDataObjectSettingsDialog::RadiusLineEditChanged()
{
    QString t = radiusLineEdit->text();
    double radius = t.toDouble();
    m_object->SetDTITubeRadius(radius);
}

void PolyDataObjectSettingsDialog::NewResolutionSelected(int n)
{
    m_object->SetDTITubeResolution(n);
}


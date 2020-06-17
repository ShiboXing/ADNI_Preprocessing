/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: simplevolumizedialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-03-31 14:27:27 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#include "simplevolumizedialog.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qslider.h>
#include <qbuttongroup.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qfiledialog.h>
#include "ignsconfig.h"
#include "ignsmsg.h"
#include "ignsglobals.h"
#include "usvolumereconstructionsettings.h"
#include "MINCReader.h"
#include "vtkMINCImageAttributes2.h"
#include "vtkMINC2.h"

SimpleVolumizeDialog::SimpleVolumizeDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : VolumizeDialogBase( parent, name, modal, fl )
{
    if ( !name )
        setName( "VolumizeDialog" );
    m_interpolationType = IGNS_GAUSSIAN_INTERPOLATION;
    m_maskType = NO_MASK;
    radiusLineEdit->setText(QString::number((double)IGNS_DEFAULT_REGRID_RADIUS));
    stepLineEdit->setText(QString::number((double)IGNS_DEFAULT_STEP_SIZE));
    customMaskLineEdit->setEnabled(false);
    customBrowsePushButton->setEnabled(false);
    previewPushButton->hide(); //TODO: enable once the function is in
    probeSettingsTextLabel->hide();
    currentSettingsTextLabel->hide();
    interpolationComboBox->insertItem(STR_IGNS_GAUSSIAN_INTERPOLATION, IGNS_GAUSSIAN_INTERPOLATION); 
    interpolationComboBox->insertItem(STR_IGNS_KAISERBESSEL_INTERPOLATION, IGNS_KAISERBESSEL_INTERPOLATION); 
    interpolationComboBox->insertItem(STR_IGNS_LINEAR_INTERPOLATION, IGNS_LINEAR_INTERPOLATION); 
    interpolationComboBox->setCurrentItem(IGNS_GAUSSIAN_INTERPOLATION);
    saturationThresholdSlider->setMaxValue(100); 
    saturationThresholdSlider->setLineStep(1);
    m_output = 0;
    m_onTheFly = false;
    m_saturationThreshold = IGNS_DEFAULT_SATURATION_THRESHOLD;
}

SimpleVolumizeDialog::~SimpleVolumizeDialog()
{
}

void SimpleVolumizeDialog::MaskTypeChanged(int type)
{
    switch (type)
    {
        case 1:
            m_maskType = CUSTOM_MASK;
            customMaskLineEdit->setEnabled(true);
            customBrowsePushButton->setEnabled(true);
            break;
        case 0:
        default:
            m_maskType = DEFAULT_MASK;
            customMaskLineEdit->setEnabled(false);
            customBrowsePushButton->setEnabled(false);
            break;
        case 2:
            m_maskType = NO_MASK;
            customMaskLineEdit->setEnabled(false);
            customBrowsePushButton->setEnabled(false);
            break;
            
    }
}

void SimpleVolumizeDialog::CustomMaskBrowseButtonClicked()
{
    QString mask_directory;
    QString customMask = customMaskLineEdit->text();
    if (customMask.isEmpty())
    {
        mask_directory = QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY;
    }
    else
    {
        QFileInfo info( customMask );
        mask_directory = (const char*)info.dirPath( TRUE );
    }
    if (!QFile::exists(mask_directory))
        mask_directory = QDir::homeDirPath();
    QString filename = QFileDialog::getOpenFileName(mask_directory, "Minc file (*.mnc)", 0, "open", "Load mask file"); 
    if (!filename.isEmpty())
    {
        QFileInfo fi(filename);
        if (!fi.exists())
        {
            QString accessError = IGNS_MSG_FILE_NOT_EXISTS + filename;
            QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
            customMaskLineEdit->setText(customMask);
            return;
        }
        if (!fi.isReadable())
        {
            QString accessError = IGNS_MSG_NO_READ + filename;
            QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
            customMaskLineEdit->setText(customMask);
            return;
        }
    }
    customMaskLineEdit->setText(filename);
}

void SimpleVolumizeDialog::CustomMaskLineEditChanged()
{
    QString filename = customMaskLineEdit->text();
    if (!filename.isEmpty())
    {
        QFileInfo fi(filename);
        if (!fi.exists())
        {
            QString accessError = IGNS_MSG_FILE_NOT_EXISTS + filename;
            QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
            return;
        }
        if (!fi.isReadable())
        {
            QString accessError = IGNS_MSG_NO_READ + filename;
            QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
            return;
        }
    }
}

void SimpleVolumizeDialog::PreviewButtonClicked()
{
    qWarning( "SimpleVolumizeDialog::PreviewButtonClicked(): Not implemented yet" );
}

void SimpleVolumizeDialog::InterpolationTypeChanged(int index)
{
    m_interpolationType = index;
}

void SimpleVolumizeDialog::InputFilesLineEditChanged()
{
}

void SimpleVolumizeDialog::OutputFileLineEditChanged()
{
    m_outputFile = outputLineEdit->text();
    QFileInfo info( m_outputFile );
    QString dirPath = (const char*)info.dirPath( TRUE );
    QFileInfo info1( dirPath );
    if (!info1.isWritable())
    {
        QString accessError = IGNS_MSG_NO_WRITE + dirPath;
        QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
        m_outputFile.truncate(0);
        outputLineEdit->setText(m_lastVisitedDir);
        return;
    }
}

void SimpleVolumizeDialog::ReconstructionTypeChanged(int state)
{
    if (state == 1)
    {
        saturationThresholdLineEdit->setEnabled(TRUE);
        decreaseSaturationThresholdPushButton->setEnabled(TRUE);
        increaseSaturationThresholdPushButton->setEnabled(TRUE);
        saturationThresholdSlider->setEnabled(TRUE);
        saturationThresholdLineEdit->setText(QString::number((double)m_saturationThreshold));
    }
    else
    {
        saturationThresholdLineEdit->setEnabled(FALSE);
        decreaseSaturationThresholdPushButton->setEnabled(FALSE);
        increaseSaturationThresholdPushButton->setEnabled(FALSE);
        saturationThresholdSlider->setEnabled(FALSE);
    }
}

void SimpleVolumizeDialog::IncreaseSaturationThresholdPushButtonClicked()
{
    int sliderPosition = saturationThresholdSlider->value();
    saturationThresholdSlider->setValue(sliderPosition + 1);
}

void SimpleVolumizeDialog::DecreaseSaturationThresholdPushButtonClicked()
{
    int sliderPosition = saturationThresholdSlider->value();
    saturationThresholdSlider->setValue(sliderPosition - 1);
}

void SimpleVolumizeDialog::SaturationThresholdSliderValueChanged(int threshold)
{
    saturationThresholdLineEdit->setText(QString::number((double)threshold/100.0));
}
 
void SimpleVolumizeDialog::SaturationThresholdLineEditChanged(const QString &threshold)
{
    double res = saturationThresholdLineEdit->text().toDouble();
    if ( res < 0.0 )
        res = 0.0;
    if (res >1.0)
        res = 1.0;
    m_saturationThreshold = res;
    saturationThresholdLineEdit->blockSignals(TRUE);
    saturationThresholdSlider->setValue((int)(res * 100.0));
    saturationThresholdLineEdit->blockSignals(FALSE);
}

void SimpleVolumizeDialog::InputFilesBrowseButtonClicked()
{
    m_inputFiles = QFileDialog::getOpenFileNames("Minc file (*.mnc)",
                                m_lastVisitedDir,
                                0,
                                "open",
                                "Import Video Frames");
    if (m_inputFiles.empty())
        return;
    QString tmp, params;
    QStringList::iterator itBeg = m_inputFiles.begin();
    // get US params from the first file on list
    MINCReader* mincReader = MINCReader::New();
    mincReader->SetFileName((*itBeg).ascii());
    mincReader->Update();
    if (mincReader->GetErrorCode())
    {
        QMessageBox::warning( 0, "Warning", IGNS_MSG_MINC_CORRUPTED, 1, 0 );
    }
    else
    {
        vtkMINCImageAttributes2 * attributes = mincReader->GetImageAttributes();
        QString depthStr(attributes->GetAttributeValueAsString(MIacquisition, MIultrasound_depth));
        QString colorStr(attributes->GetAttributeValueAsString(MIacquisition, MIultrasound_acquisition_color));
        cout << "depth: " << depthStr << ", color: " << colorStr << endl;
        if (!depthStr.isNull() && !depthStr.isEmpty() && !colorStr.isNull() && !colorStr.isEmpty())
        {
            probeSettingsTextLabel->show();
            currentSettingsTextLabel->show();
            if (QString::compare(colorStr, "RGB"))
            {
                params = "grayscale (b-mode), " + depthStr;
                m_output->SetUSMode(1);
            }
            else
            {
                params = "RGB (doppler), " + depthStr;
                m_output->SetUSMode(0);
            }
            m_output->SetUSDepth(depthStr);
        }
        else
        {  // we cannot have default mask
            m_maskType = NO_MASK;
            defaultMaskRadioButton->setEnabled(false);
            defaultMaskRadioButton->setChecked(false);
            customMaskRadioButton->setChecked(false);
            noMaskRadioButton->setChecked(true);
            customMaskLineEdit->setEnabled(false);
            customBrowsePushButton->setEnabled(false);
        }
        currentSettingsTextLabel->setText(params);
    }
    mincReader->Delete();
    for ( QStringList::Iterator it = itBeg; it != m_inputFiles.end(); ++it )
    {
        tmp += *it + " ";
    }
      
    inputFilesLineEdit->setText(tmp);
}

void SimpleVolumizeDialog::OutputFileBrowseButtonClicked()
{
    QString filename = QFileDialog::getSaveFileName(m_lastVisitedDir, "Minc file (*.mnc)", 0, "open", "Save US volume file" );
    
    if(filename.isEmpty())
        return;
    
    if (QFile::exists(filename)) 
    {
        int ret = QMessageBox::warning(this, "Save US volume file",
                                       tr("File %1 already exists.\n"
                                               "Do you want to overwrite it?")
                                               .arg(QDir::convertSeparators(filename)),
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::No | QMessageBox::Escape);
        if (ret == QMessageBox::No)
            return;
    }
    
    if(!filename.isEmpty())
    {
        QFileInfo info( filename );
        QString dirPath = (const char*)info.dirPath( TRUE );
        QFileInfo info1( dirPath );
        if (!info1.isWritable())
        {
            QString accessError = IGNS_MSG_NO_WRITE + dirPath;
            QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
            return;
        }
        m_outputFile = filename;
        outputLineEdit->setText(m_outputFile);
    }
}

void SimpleVolumizeDialog::Init( bool flag, QString dir) 
{
    if (m_output)
    {
//TODO: implement when data available in acquisition files
//        int grayscale = m_output->GetUSMode();
//        QString depth = m_output->GetUSDepth();
        radiusLineEdit->setText(QString::number((double)m_output->GetRegridRadius()));
        stepLineEdit->setText(QString::number((double)m_output->GetStepSize()));
        sigmaLineEdit->setText(QString::number((double)m_output->GetSigma()));
        QString tmp = m_output->GetCustomMaskFileName();
        if (m_output->GetMaskType() == CUSTOM_MASK && !tmp.isEmpty())
        {
            m_maskType = CUSTOM_MASK;
            customMaskLineEdit->setText(tmp);
            customMaskLineEdit->setEnabled(true);
            customBrowsePushButton->setEnabled(true);
            customMaskRadioButton->setChecked(true);
            defaultMaskRadioButton->setChecked(false);
            noMaskRadioButton->setChecked(false);
        }
        else
        {
            m_maskType = NO_MASK;
            noMaskRadioButton->setChecked(true);
            customMaskRadioButton->setChecked(false);
            customMaskLineEdit->setEnabled(false);
            customBrowsePushButton->setEnabled(false);
        }
        m_interpolationType = m_output->GetInterpolationType();
        interpolationComboBox->setCurrentItem(m_interpolationType);
//TODO: implement when data available in acquisition files
/*        if (grayscale)
            tmp = "grayscale (b-mode), " + depth;
        else
            tmp = "RGB (doppler), " + depth;
        defaultMaskTextLabel->setText(tmp);*/
        m_saturationThreshold = m_output->GetSaturationThreshold();
        if (m_output->GetReconstructionType() == 1)
        {
            vesselsOnlyRadioButton->setChecked(TRUE);
            saturationThresholdLineEdit->setEnabled(TRUE);
            decreaseSaturationThresholdPushButton->setEnabled(TRUE);
            increaseSaturationThresholdPushButton->setEnabled(TRUE);
            saturationThresholdSlider->setEnabled(TRUE);
            saturationThresholdLineEdit->setText(QString::number((double)m_saturationThreshold));
        }
        else
        {
            wholeVolumeRadioButton->setChecked(TRUE);
            saturationThresholdLineEdit->setEnabled(FALSE);
            decreaseSaturationThresholdPushButton->setEnabled(FALSE);
            increaseSaturationThresholdPushButton->setEnabled(FALSE);
            saturationThresholdSlider->setEnabled(FALSE);
        }
        m_onTheFly = flag;
        if (m_onTheFly)
        {
            inputFilesBrowseBushButton->setEnabled(FALSE);
            outputBrowseBushButton->setEnabled(FALSE);
            outputLineEdit->setEnabled(FALSE);
            outputLineEdit->setText(tr("Output file name will be set later."));
            inputFilesLineEdit->setEnabled(FALSE);
            inputFilesLineEdit->setText(tr("Captured frames are used as input."));
        }
        else
        {
            if (!dir.isEmpty())
                m_lastVisitedDir = dir;
            else
            {
                m_lastVisitedDir = QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY;
                if (!QFile::exists(m_lastVisitedDir))
                    dir = QDir::homeDirPath();
            }
        }
    }
}

void SimpleVolumizeDialog::OKButtonClicked()
{
    //transfer the data back to USAcquisitionOutput class
    if (m_output)
    {
        m_output->SetMaskType(m_maskType);
        m_output->SetCustomMaskFileName(customMaskLineEdit->text());
        double res = radiusLineEdit->text().toDouble();
        if ( res > 0.0 && res <= IGNS_MAX_REGRID_RADIUS)
            m_output->SetRegridRadius(res);
        else 
        {
            m_output->SetRegridRadius(IGNS_DEFAULT_REGRID_RADIUS);
            QString tmp = IGNS_MSG_POSITIVE_LIMITED + QString::number((double)IGNS_MAX_REGRID_RADIUS);
            QMessageBox::warning( 0, "Error: ", tmp, 1, 0 );
            return;
        }
        res = stepLineEdit->text().toDouble();
        if ( res > 0.0 && res <= IGNS_MAX_STEP_SIZE)
            m_output->SetStepSize(res);
        else 
        {
            m_output->SetStepSize(IGNS_DEFAULT_STEP_SIZE);
            QString tmp = IGNS_MSG_POSITIVE_LIMITED + QString::number((double)IGNS_MAX_STEP_SIZE);
            QMessageBox::warning( 0, "Error: ", tmp, 1, 0 );
            return;
        }
        res = sigmaLineEdit->text().toDouble();
        if ( res > 0.0)
            m_output->SetSigma(res);
        else 
        {
            m_output->SetSigma(2 * m_output->GetRegridRadius());
            QMessageBox::warning( 0, "Error: ", IGNS_MSG_POSITIVE, 1, 0 );
            return;
        }
        m_output->SetInterpolationType(m_interpolationType);
        int selected = reconstructButtonGroup->selectedId();
        m_output->SetReconstructionType(selected);
        if (selected == 1)
        {
            res = saturationThresholdLineEdit->text().toDouble();
            if ( res >= 0.0)
                m_output->SetSaturationThreshold(res);
            else
            {
                m_output->SetSaturationThreshold(IGNS_DEFAULT_SATURATION_THRESHOLD);
                QString tmp = IGNS_MSG_POSITIVE_OR_ZERO + QString::number((double)1.0);
                QMessageBox::warning( 0, "Error: ", tmp, 1, 0 );
                return;
            }
        }
        if (!m_onTheFly)
        {
            // Make sure there are actually files selected
            if(m_inputFiles.size() > 0)
            {
                m_output->SetInputFiles(m_inputFiles);
            }
            else
            {
                QMessageBox::warning( 0, "Error: ", "Please select input files.", 1, 0 );
                return;
            }
            if (!m_outputFile.isEmpty())
                m_output->SetOutputFile(m_outputFile);
            // we allow empty output file name, as user may not want to save
        }
    }
    accept();
}

void SimpleVolumizeDialog::SetUSReconstructionParams(USVolumeReconstructionSettings *params)
{
    if (params)
    {
        m_output = params;
    }
}

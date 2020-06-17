/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: multiplefileviewerdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-04-08 14:56:03 $
  Version:   $Revision: 1.16 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/

#include "multiplefileviewerdialog.h"

#include <sstream>
#include <stdlib.h>

#include <qpushbutton.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "ignsmsg.h"
#include "doublewindowfileviewer.h"
#include "scenemanager.h"
#include "vtkTransform.h"
#include "vtkQtMatrixDialog.h"
#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkMINC2.h"
#include "mincinfowidget.h"
#include "yesnodialog.h"
#include "maskdialog.h"
#include "usvolumereconstructionsettings.h"
#include "vtkSmartPointer.h"

#include "MINCReader.h"
#include "vtkMINCImageAttributes2.h"

AcquisitionData::AcquisitionData():
    m_imageData(0),
    m_transform(0),
    m_startX(0.0),
    m_startY(0.0),
    m_startZ(0.0),
    m_directionCosines(0)
{
    m_transform = vtkTransform::New();
    m_transform->Identity();
}

AcquisitionData::~AcquisitionData()
{
    if (m_transform)
        m_transform->Delete();
    if (m_directionCosines)
        m_directionCosines->UnRegister(0);        
}

void AcquisitionData::SetImage( vtkImageData * image )
{
    if( m_imageData == image )
    {
        return;
    }

    if( m_imageData )
    {
        m_imageData->UnRegister( 0 );
    }
    m_imageData = image;
    if( m_imageData )
    {
        m_imageData->Register( 0 );
    }
}

void AcquisitionData::SetTransform(vtkMatrix4x4 * trans)
{
    if( trans)
    {
        m_transform->Identity();
        m_transform->Concatenate(trans);
    }
}

void AcquisitionData::SetStartCoordinates(double x, double y, double z)
{
    m_startX = x;
    m_startY = y;
    m_startZ = z;
}

void AcquisitionData::GetStartCoordinates(double *x, double *y, double *z)
{
    *x = m_startX;
    *y = m_startY;
    *z = m_startZ;
}

void AcquisitionData::SetStep(double x, double y, double z)
{
    m_stepX = x;
    m_stepY = y;
    m_stepZ = z;
}

void AcquisitionData::GetStep(double *x, double *y, double *z)
{
    *x = m_stepX;
    *y = m_stepY;
    *z = m_stepZ;
}

void AcquisitionData::SetDirectionCosines(vtkMatrix4x4 *dc)
{
    if( m_directionCosines == dc )
    {
        return;
    }

    if( m_directionCosines )
    {
        m_directionCosines->UnRegister( 0 );
    }
    m_directionCosines = dc;
    if( m_directionCosines )
    {
        m_directionCosines->Register( 0 );
    }
}

vtkMatrix4x4 * AcquisitionData::GetDirectionCosines()
{
    return m_directionCosines;
}

MultipleFileViewerDialog::MultipleFileViewerDialog( QWidget* parent, const char* name, WFlags fl )
    : MultipleFileViewerDialogBase( parent, name, fl ),
    m_viewer(0),
    m_usParamsFromFile(0),
    m_manager(0),
    m_discardedFilesDirectory(""),
    m_acquisitionImageCounter(0),
    m_current_index(0),
    m_timeInterval(DEFAULT_TIME_INTERVAL * 1000),
    m_slideShowOn(false)
{
    if ( !name )
        setName( "MultipleFileViewerDialog" );

    m_timer = new QTimer( NULL );     
    connect( m_timer, SIGNAL(timeout()), this, SLOT(SetNextImage()) );
    m_rightWindowLut = vtkLookupTable::New();
    m_rightWindowLut->SetTableRange(0, 255);
    m_rightWindowLut->SetHueRange( 1, 1 );
    m_rightWindowLut->SetSaturationRange( 0, 0 );
    m_rightWindowLut->SetValueRange( 1, 1 );
    m_rightWindowLut->Build();

    m_leftWindowLut = vtkLookupTable::New();
    m_leftWindowLut->SetTableRange(0, 255);
    m_leftWindowLut->SetHueRange( 1, 1 );
    m_leftWindowLut->SetSaturationRange( 0, 0 );
    m_leftWindowLut->SetValueRange( 1, 1 );
    m_leftWindowLut->Build();
}

MultipleFileViewerDialog::~MultipleFileViewerDialog()
{
    this->CleanAcquisition();
    if (m_manager != NULL) 
    { 
        m_manager->UnRegister(0); 
    }
    if (m_usParamsFromFile)
        delete m_usParamsFromFile;
    m_rightWindowLut->Delete();
    m_leftWindowLut->Delete();
    delete  m_timer;
}

void MultipleFileViewerDialog::LoadFilesButtonClicked()
{
    QStringList inputFiles;
    QString  filename;
    double x, y, z;
    bool readUSParams = false;
    if (m_acquisitionImageCounter > 0)
        this->CleanAcquisition();
    inputFiles = QFileDialog::getOpenFileNames("Minc file (*.mnc)",
                                m_lastVisitedDirectory,
                                0,
                                "open",
                                "Read Frames");
    for ( QStringList::Iterator it = inputFiles.begin(); it != inputFiles.end(); ++it )
    {
        filename = (*it);
        MINCReader * reader = MINCReader::New();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        reader->SetLogger(this->GetLogger());
#endif
        if( reader->CanReadFile( filename ) )
        {
            AcquisitionData *data = AcquisitionData::New();
            reader->SetFileName(filename);
            reader->SetDataType(VTK_UNSIGNED_CHAR);
            reader->SetUseTransformForStartAndStep(1);
            reader->Update();
            vtkMINCImageAttributes2 *attributes = reader->GetImageAttributes();
            x = attributes->GetAttributeValueAsDouble(MIxspace, MIstart);
            y = attributes->GetAttributeValueAsDouble(MIyspace, MIstart);
            z = attributes->GetAttributeValueAsDouble(MIzspace, MIstart);
            data->SetFileName(filename);
            data->SetImage(reader->GetOutput());
            data->SetStartCoordinates(x, y, z);
            x = attributes->GetAttributeValueAsDouble(MIxspace, MIstep);
            y = attributes->GetAttributeValueAsDouble(MIyspace, MIstep);
            z = attributes->GetAttributeValueAsDouble(MIzspace, MIstep);
            data->SetStep(x, y, z);
            data->SetDirectionCosines(reader->GetDirectionCosines());
            data->SetTransform(reader->GetTransformationMatrix());
            m_acquisition.push_back( data );
            // prepare for mask and grid
            if (!readUSParams)
            {
                // set scale
                vtkReal scale[3];
                vtkSmartPointer< vtkTransform > tmp = vtkSmartPointer< vtkTransform >::New();
                tmp->Identity();
                tmp->SetMatrix( reader->GetTransformationMatrix() );
                tmp->Inverse();
                tmp->GetScale(scale);
                m_viewer->SetGridScale(scale);
                m_usParamsFromFile = new USVolumeReconstructionSettings;
                QString depthStr(attributes->GetAttributeValueAsString(MIacquisition, MIultrasound_depth));
                QString colorStr(attributes->GetAttributeValueAsString(MIacquisition, MIultrasound_acquisition_color));
                if (!depthStr.isNull() && !depthStr.isEmpty() && !colorStr.isNull() && !colorStr.isEmpty())
                {
                    m_usParamsFromFile->SetUSDepth(depthStr);
                    if (QString::compare(colorStr, "RGB"))
                    {
                        m_usParamsFromFile->SetUSMode(1);
                    }
                    else
                    {
                        m_usParamsFromFile->SetUSMode(0);
                    }
                }
            }
            setMaskPushButton->setEnabled(TRUE);    
            readUSParams = true;
        }
        //TODO else {} error message
        reader->Delete();
    }
    m_acquisitionImageCounter = m_acquisition.size();
    if (m_acquisitionImageCounter > 0)
    {
        framesSlider->setMaxValue(m_acquisitionImageCounter-1);
        m_viewer->SetUltrasoundImage(m_acquisition[0]->GetImage());
        m_acquisition[0]->GetStep(&x, &y, &z);
        m_viewer->SetResliceTransform(m_acquisition[0]->GetTransform());
        filenameLineEdit->setText(m_acquisition[0]->GetFileName());
        m_acquisition[0]->GetStartCoordinates(&x, &y, &z);
        startXLineEdit->setText(QString::number(x));
        startYLineEdit->setText(QString::number(y));
        startZLineEdit->setText(QString::number(z));
        m_current_index = 0;
    }
}

void MultipleFileViewerDialog::LoadVolumeButtonClicked()
{
    if (m_lastVisitedDirectory.isEmpty())
        m_lastVisitedDirectory = QDir::homeDirPath();

    QString filename = QFileDialog::getOpenFileName( m_lastVisitedDirectory, "Minc file (*.mnc)", 0 , "open", "Open minc file" ); 
    if( !filename.isNull() )
    {
        vtkImageData *image = 0;
        if( QFile::exists( filename ) )
        {
//             vtkMINCImageReader2 * reader = vtkMINCImageReader2::New();
            MINCReader * reader = MINCReader::New();
            reader->SetFileName( filename );

            reader->Update();
            image = reader->GetOutput();
            m_viewer->SetImageData(image, m_rightWindowLut);
            reader->Delete();
            QFileInfo info( filename );
            m_lastVisitedDirectory = info.dirPath( true );
        }
        else
            QMessageBox::warning( this, "Error: can't open file: ", filename, 1, 0 );
        vtkReal range[2];
        m_rightWindowLut->GetTableRange(range);
        rightWindowSlider->blockSignals(true);
        rightLevelSlider->blockSignals(true);
        rightWindowSlider->setMaxValue( (int)(range[1]-range[0]) ); 
        rightWindowSlider->setLineStep((int)(0.02*(range[1] - range[0]))); //50 steps is enough 
        rightLevelSlider->setMaxValue( (int)((range[1]-range[0])/2) ); 
        rightLevelSlider->setLineStep((int)(0.04*(range[1] - range[0]))); //50 steps is enough 
        rightWindowSlider->setValue((int)(0.5*rightWindowSlider->maxValue()));
        rightLevelSlider->setValue((int)(0.5*rightLevelSlider->maxValue()));
        rightWindowSlider->blockSignals(false);
        rightLevelSlider->blockSignals(false);
    }
}

void MultipleFileViewerDialog::DeleteCurrentButtonClicked()
{
    AcquisitionDataList::iterator it = std::find( m_acquisition.begin(), m_acquisition.end(), m_acquisition[m_current_index] );
    if( it != m_acquisition.end() )
    {
        QFileInfo info( m_acquisition[m_current_index]->GetFileName() );
        QString initialPath = info.dirPath( true );
        QString fileName = info.fileName();
        if (m_discardedFilesDirectory.isEmpty())
        {
            char tempName[512];
            sprintf(tempName, "%s/%s", initialPath.ascii(), DISCARDED_ACQUISITIONS_DIR);
            if (mkdtemp(tempName))
            {
                m_discardedFilesDirectory = tempName;
                QString infoMsg = "Discarded files will be moved to " + m_discardedFilesDirectory;
                QMessageBox::information( 0, "Moving Files", infoMsg, 1, 0 );
            }
        }
        if (!m_discardedFilesDirectory.isEmpty())
        {
            QString newName(m_discardedFilesDirectory);
            newName.append("/");
            newName.append(fileName);
            if (rename((const char*)(m_acquisition[m_current_index]->GetFileName()), (const char*)newName) != 0)
            {
                QString  accessError = IGNS_MSG_CANT_REMOVE + newName;
                QMessageBox::warning( 0, "Warning", accessError, 1, 0 );
            }
        }
        else
        {
            YesNoDialog *dlg = new YesNoDialog(0, "removeFile", TRUE, Qt::WDestructiveClose );
            QString tmp = "File " + m_acquisition[m_current_index]->GetFileName() +
                        "\ncannot be moved to e temporary directory.\nDelete immediately?";
            dlg->SetQuestion(tmp);
            if (dlg->exec() == QDialog::Accepted)
            {
                std::remove(m_acquisition[m_current_index]->GetFileName());
                m_acquisition[m_current_index]->Delete();
            }
        }
        m_acquisition.erase( it );
        m_acquisitionImageCounter = m_acquisition.size();
        framesSlider->setMaxValue(m_acquisitionImageCounter-1);
        FrameSliderValueChanged(m_current_index);
    }
}

void MultipleFileViewerDialog::FirstButtonClicked()
{
    m_current_index = 0;
    framesSlider->setValue(0);
}

void MultipleFileViewerDialog::PreviousButtonClicked()
{
    if  (m_current_index > 0)
    {
        m_current_index--;
        framesSlider->setValue(m_current_index);
    }
}

void MultipleFileViewerDialog::FrameSliderValueChanged(int index)
{
    double x, y, z;
    m_current_index = index;
    if (m_acquisitionImageCounter > 0)
    {
        m_viewer->SetUltrasoundImage(m_acquisition[m_current_index]->GetImage());
        m_viewer->SetResliceTransform(m_acquisition[m_current_index]->GetTransform());
        filenameLineEdit->setText(m_acquisition[m_current_index]->GetFileName());
        m_acquisition[m_current_index]->GetStartCoordinates(&x, &y, &z);
        startXLineEdit->setText(QString::number(x));
        startYLineEdit->setText(QString::number(y));
        startZLineEdit->setText(QString::number(z));
    }
}

void MultipleFileViewerDialog::SetNextImage()
{
    if  (m_current_index < m_acquisitionImageCounter-1)
    {
        m_current_index++;
        framesSlider->setValue(m_current_index);
    }
    else
    {
        m_timer->stop();
        m_slideShowOn = !m_slideShowOn;
        startPushButton->setText( "Start" );
        this->SetButtonsEnabled(true);
    }
}

void MultipleFileViewerDialog::TimeComboBoxActivated(int timeInt)
{
    m_timeInterval = (timeInt + SHORTEST_TIME_INTERVAL)*1000;
}

void MultipleFileViewerDialog::StartButtonClicked()
{
    if (m_slideShowOn)
    {
        m_timer->stop();
        startPushButton->setText( "Start" );
        this->SetButtonsEnabled(true);
        m_viewer->EnableGrid(1);
    }
    else
    {
        if  (m_current_index == m_acquisitionImageCounter-1)
        {
            m_current_index = 0;
            framesSlider->setValue(0);
        }
        m_timer->start( m_timeInterval, false );
        startPushButton->setText( "Stop" );
        this->SetButtonsEnabled(false);
        m_viewer->EnableGrid(0);
    }
    m_slideShowOn = !m_slideShowOn;
}

void MultipleFileViewerDialog::SetTransformButtonClicked()
{
    vtkMatrix4x4 *tmpMat;
    tmpMat = vtkMatrix4x4::New();
    vtkQtMatrixDialog * dlg;
    dlg = new vtkQtMatrixDialog( tmpMat, false, 0, "" );
    dlg->SetDirectory(m_lastVisitedDirectory);
    if (dlg->exec() == QDialog::Accepted)
    {
        m_viewer->SetRightImageTransform(tmpMat);
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "MultipleFileViewerDialog registrationTransform:" << "\n" <<
        tmpMat->Element[0][0] << ", " <<
        tmpMat->Element[0][1] << ", " <<
        tmpMat->Element[0][2] << ", " <<
        tmpMat->Element[0][3] << "\n" <<
        tmpMat->Element[1][0] << ", " <<
        tmpMat->Element[1][1] << ", " <<
        tmpMat->Element[1][2] << ", " <<
        tmpMat->Element[1][3] << "\n" <<
        tmpMat->Element[2][0] << ", " <<
        tmpMat->Element[2][1] << ", " <<
        tmpMat->Element[2][2] << ", " <<
        tmpMat->Element[2][3] <<
        endl);
#endif
        tmpMat->Delete();
    }
}

void MultipleFileViewerDialog::SetMaskButtonClicked()
{
    MaskDialog *setMaskDialog = new MaskDialog;
    setMaskDialog->SetMaskParameters(m_usParamsFromFile);
    QString maskFile;
    if (setMaskDialog->exec() == QDialog::Accepted)
    {
        // mask dialog will save mask name as customMaskFileName independently of mask type
        maskFile = m_usParamsFromFile->GetCustomMaskFileName();
        m_viewer->SetMask(maskFile);
    }
}

void MultipleFileViewerDialog::NextButtonClicked()
{
    if  (m_current_index < m_acquisitionImageCounter-1)
    {
        m_current_index++;
        framesSlider->setValue(m_current_index);
    }
}

void MultipleFileViewerDialog::LastButtonClicked()
{
    m_current_index = m_acquisitionImageCounter-1;
    framesSlider->setValue(m_current_index);
}

void MultipleFileViewerDialog::LeftWindowSliderValueChanged(int window)
{
    m_viewer->SetLeftImageWindow(window);
}

void MultipleFileViewerDialog::LeftLevelSliderValueChanged(int level)
{
    m_viewer->SetLeftImageLevel(level);
}

void MultipleFileViewerDialog::LeftColorComboBoxActivated(int index)
{
    vtkReal range[2] = {0, 255};
    QString name = m_manager->GetTemplateLookupTableName(index);
    m_manager->CreateLookupTable(name, range, m_leftWindowLut, false);
    leftWindowSlider->blockSignals(true);
    leftLevelSlider->blockSignals(true);
    leftWindowSlider->setValue(rightWindowSlider->maxValue());
    leftLevelSlider->setValue(rightLevelSlider->maxValue());
    leftWindowSlider->blockSignals(false);
    leftLevelSlider->blockSignals(false);
    m_viewer->SetLeftImageColor(m_leftWindowLut);
}

void MultipleFileViewerDialog::RightWindowSliderValueChanged(int window)
{
    m_viewer->SetRightImageWindow(window);
}

void MultipleFileViewerDialog::RightLevelSliderValueChanged(int level)
{
    m_viewer->SetRightImageLevel(level);
}

void MultipleFileViewerDialog::RightColorComboBoxActivated(int index)
{
    vtkReal range[2] = {0, 255};
    QString name = m_manager->GetTemplateLookupTableName(index);
    m_manager->CreateLookupTable(name, range, m_rightWindowLut, false);
    rightWindowSlider->blockSignals(true);
    rightLevelSlider->blockSignals(true);
    rightWindowSlider->setValue(rightWindowSlider->maxValue());
    rightLevelSlider->setValue(rightLevelSlider->maxValue());
    rightWindowSlider->blockSignals(false);
    rightLevelSlider->blockSignals(false);
    m_viewer->SetRightImageColor(m_rightWindowLut);
}
    
void MultipleFileViewerDialog::SetViewer(DoubleWindowFileViewer *viewer)
{
    m_viewer = viewer;
}

void MultipleFileViewerDialog::SetLastVisitedDirectory(const QString & dir)
{
    m_lastVisitedDirectory = dir;
    if( !QFile::exists( m_lastVisitedDirectory ) )
    {
        m_lastVisitedDirectory = QDir::homeDirPath();
    }
}

void MultipleFileViewerDialog::SetSceneManager( SceneManager *manager )
{
    if (m_manager != manager)                                       
    {                                                           
        if (m_manager != NULL) 
        { 
            m_manager->UnRegister(0); 
        }   
        m_manager = manager;                                          
        if (m_manager != NULL) 
        { 
            m_manager->Register(0); 
        }     
    }                                                           
}

void MultipleFileViewerDialog::Initialize()
{
    int i;
    if (m_manager != NULL) 
    {
        for (i = 0; i < m_manager->GetNumberOfTemplateLokupTables(); i++)
        {
            leftColorComboBox->insertItem(m_manager->GetTemplateLookupTableName(i));
            rightColorComboBox->insertItem(m_manager->GetTemplateLookupTableName(i));
        }
    }
    for (i = SHORTEST_TIME_INTERVAL ; i <= LONGEST_TIME_INTERVAL; i++)
        timeComboBox->insertItem(QString::number(i));
    timeComboBox->setCurrentItem(DEFAULT_TIME_INTERVAL-SHORTEST_TIME_INTERVAL);
    setMaskPushButton->setEnabled(FALSE);    
}

void MultipleFileViewerDialog::SetButtonsEnabled(bool enable)
{
    firstPushButton->setEnabled(enable);
    previousPushButton->setEnabled(enable);
    nextPushButton->setEnabled(enable);
    lastPushButton->setEnabled(enable);
    deletePushButton->setEnabled(enable);
    loadVolumePushButton->setEnabled(enable);
    timeComboBox->setEnabled(enable);
    rightWindowSlider->setEnabled(enable);
    rightLevelSlider->setEnabled(enable);
    rightColorComboBox->setEnabled(enable);
    loadFilesPushButton->setEnabled(enable);
    setTransformPushButton->setEnabled(enable);
    leftWindowSlider->setEnabled(enable);
    leftLevelSlider->setEnabled(enable);
    leftLevelSlider->setEnabled(enable);
    leftColorComboBox->setEnabled(enable);
    infoPushButton->setEnabled(enable);
}
    
void MultipleFileViewerDialog::CleanAcquisition()
{
    AcquisitionDataList::iterator it = m_acquisition.begin();
    AcquisitionDataList::iterator itEnd = m_acquisition.end();
    while( it != itEnd )
    {
        (*it)->Delete();
        ++it;
    }
    m_acquisition.clear();
    m_acquisitionImageCounter = 0;
}

void MultipleFileViewerDialog::ShowMincInfo()
{
    std::ostrstream info;
    QString  filename = m_acquisition[m_current_index]->GetFileName();
    MINCReader * reader = MINCReader::New();
    if( reader->CanReadFile( filename ) )
    {
        reader->SetFileName(filename);
        reader->Update();
        vtkMINCImageAttributes2 *attributes = reader->GetImageAttributes();
        if (attributes)
        {
            attributes->PrintFileHeader(info);
            MincInfoWidget *infoWidget = new MincInfoWidget(0, "MincInfo", Qt::WDestructiveClose);
            infoWidget->SetInfoText(info.str());
            infoWidget->show();
        }
    }
    reader->Delete();
}


/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: volumecreator.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:34 $
  Version:   $Revision: 1.3 $

  Copyright (c) 2008-2008 IPL, BIC, MNI, McGill, Sean J S Chen 
  All rights reserved.
  Extracted from mainwindow.cpp and modified by Anka
=========================================================================*/
#include "volumecreator.h"

#include <qapplication.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include "vtkMatrix4x4.h"
#include "vtkMINCWriter.h"
#include "scenemanager.h"
#include "regridvolume.h"
#include "imageobject.h"
#include "usvolumereconstructionsettings.h"
#include "usacquisitioninfo.h"
#include "ignsmsg.h"

VolumeCreator::VolumeCreator():
    m_sceneManager(0),
    m_reconstructionParams(0)
{
    m_lastVisitedDirectory = QDir::homeDirPath();
}

VolumeCreator::~VolumeCreator()
{
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
}

void VolumeCreator::SetSceneManager(SceneManager *manager)
{
    if (m_sceneManager == manager)
        return;
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
    m_sceneManager = manager;
    if (m_sceneManager)
        m_sceneManager->Register(0);
}

void VolumeCreator::SetLastVisitedDirectory(const QString & dir)
{
    m_lastVisitedDirectory = dir;
    if( !QFile::exists( m_lastVisitedDirectory ) )
    {
        m_lastVisitedDirectory = QDir::homeDirPath();
    }
}

void VolumeCreator::CreateVolumeObject(const RegridVolume* const  p_pTheRegridder, QString p_theName, bool p_doExportFile, USAcquisitionInfo *info)
{
    if (m_reconstructionParams)
    {
        QString l_volumeName;
        l_volumeName = m_reconstructionParams->GetOutputFile();
        // Create an empty image object and send it into the regridder to be filled
        // with tasty goodness.
        ImageObject* l_theRegriddedImage = ImageObject::New();
        QFile tempFile;
        QDir l_theDirectory;
        bool changeName = true;
        if (l_volumeName.isEmpty())
        {
            tempFile.setName(p_theName);
            QFileInfo l_info(tempFile.name());

            l_theDirectory = l_info.dir();
            l_theRegriddedImage->SetName(l_theDirectory.dirName());
        }
        else
        {
            tempFile.setName(l_volumeName);
            QFileInfo l_info(tempFile.name());

            l_theDirectory = l_info.dir();

            l_theRegriddedImage->SetName(l_info.fileName());
            changeName = false;
        }
        if (m_sceneManager->GetObject(INTRAOP_ROOT_OBJECT_NAME))
            l_theRegriddedImage->SetObjectType(MINC_TYPE | INTRAOP_TYPE);
        else
            l_theRegriddedImage->SetObjectType(MINC_TYPE | PREOP_TYPE);
        l_theRegriddedImage->SetManager(m_sceneManager);


        bool allGood = p_pTheRegridder->GenerateVolume(*l_theRegriddedImage, changeName);
        //The method that actually regrids (and takes a million years)
        //By the way, the regridder may also modify the filename if you call GenerateVolume(*l_theRegriddedImage, true);
        if(allGood)
        {
        //Display your newly created volume
        m_sceneManager->AddObject(l_theRegriddedImage, m_sceneManager->GetObject(INTRAOP_ROOT_OBJECT_NAME));

        if(p_doExportFile)
        {

            if(l_volumeName.isEmpty())
            {
                //Create the dialog to write the volume to a minc file
                l_theDirectory.cdUp();
                QFileDialog* l_pSaveFileDialog = new QFileDialog(l_theDirectory.absPath(),
                                        "Minc file (*.mnc)",
                                        qApp->mainWidget(),
                                        "save frame volume",
                                        "Save Generated Volume");
                l_pSaveFileDialog->setMode( QFileDialog::AnyFile );
                l_pSaveFileDialog->setSelection(l_theRegriddedImage->GetName()+".mnc");
                l_pSaveFileDialog->setCaption("Save generated volume");
                l_pSaveFileDialog->show();
        
                if(l_pSaveFileDialog->exec() == QDialog::Accepted)
                {
                    l_volumeName = l_pSaveFileDialog->selectedFile();
                }
                delete l_pSaveFileDialog;
            }
            //After you get the file name, write the darn volume out as a minc file
            if(!l_volumeName.isEmpty())
            {
            tempFile.setName(l_volumeName);
            QFileInfo l_info(tempFile.name());
            l_theRegriddedImage->SetName(l_info.fileName());
            
            vtkMatrix4x4* l_theIdentity = vtkMatrix4x4::New();
            l_theIdentity->Identity();

            vtkMINCWriter* l_fileWriter = vtkMINCWriter::New();
            l_fileWriter->SetUseTransformForStartAndStep(0);
            if (info)
            {
                info->PrepareForMINCOutput();
                l_fileWriter->SetAdditionalMINCAttributes(info->GetStudyAttributes(),info->GetAcquisitionAttributes(), info->GetPatientAttributes(), info->GetComment());
            }
            l_fileWriter->SetInput(l_theRegriddedImage->GetImage());
            l_fileWriter->SetFilename(l_volumeName);
            l_fileWriter->Write();

            //cleanup
            l_theIdentity->Delete(); 
            l_fileWriter->Delete();
            QString tmp = IGNS_MSG_VOLUME_SAVED + l_volumeName;
            QMessageBox::warning( 0, "Info: ", tmp, 1, 0 );
            }
        }
        }
        else
        {
        QMessageBox::warning( 0, "Error: ", "Regridding failed due to no images to regrid or overly large step/scaling parameters", 1, 0 );
        }
    }
}


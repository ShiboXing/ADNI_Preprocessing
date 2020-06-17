/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: volumereconstruction.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:34 $
  Version:   $Revision: 1.4 $

  Copyright (c) 2008-2008 IPL, BIC, MNI, McGill, Sean J S Chen 
  All rights reserved.
  Extracted from mainwindow.cpp and modified by Anka
=========================================================================*/
#include "volumereconstruction.h"

#include <qapplication.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include "vtkMatrix4x4.h"
#include "scenemanager.h"
#include "regridvolume.h"
#include "imageobject.h"
#include "usvolumereconstructionsettings.h"
#include "usacquisitioninfo.h"
#include "ignsmsg.h"

VolumeReconstruction::VolumeReconstruction()
{
}

VolumeReconstruction::~VolumeReconstruction()
{
}

void VolumeReconstruction::VolumeReconstructionFromImportedVideoFrames(USAcquisitionInfo *info)
{
    if (m_reconstructionParams)
    {
        if(!QFile::exists(m_lastVisitedDirectory))
        {
            m_lastVisitedDirectory = QDir::homeDirPath();
        }

        // Create the volume regridder and send it thelist of files
        RegridVolume* l_pTheRegridder = RegridVolume::New();
        QStringList l_filenames = m_reconstructionParams->GetInputFiles();

        // Make sure there are actually files selected
        bool l_processOK = false;
        if(l_filenames.size() > 0)
        {
            QProgressDialog* l_pProgress = new QProgressDialog("Fetching frame files",
                                    "Cancel",
                                    l_filenames.size(),
                                    0, 0, TRUE,
                                    Qt::WDestructiveClose);
        
            l_pProgress->setCaption("Loading...");
            l_pProgress->show();
        
            int l_counter=0;
            l_processOK = true;
            for(QStringList::Iterator it=l_filenames.begin();
                l_processOK && it!=l_filenames.end();
                ++it)
            {
                cout << (*it) << endl;
                if(QFile::exists(*it)) l_pTheRegridder->InputFile(*it, l_counter, info);
            
                l_pProgress->setProgress(++l_counter);
                qApp->processEvents();
                if ( l_pProgress->wasCanceled() )
                {
                    QMessageBox::information( 0, "Loading", "Process cancelled", 1, 0 );
                    l_processOK = false;
                }
            }
            l_pProgress->close();
        }
        if (!l_processOK)
        {
            l_pTheRegridder->Delete();
            return;
        }
        //Set the masking file
        QString l_maskFile;
        if (m_reconstructionParams->GetMaskType() == CUSTOM_MASK)
        {
            l_maskFile = m_reconstructionParams->GetCustomMaskFileName();
        }
        else if (m_reconstructionParams->GetMaskType() == DEFAULT_MASK)
        {
            m_reconstructionParams->GetDefaultMaskName(l_maskFile);
        }
        if (!l_maskFile.isEmpty())
            l_pTheRegridder->SetImageMask(l_maskFile);
        l_pTheRegridder->SetInterpolationType(m_reconstructionParams->GetInterpolationType());
        l_pTheRegridder->SetInterpolationRadius(m_reconstructionParams->GetRegridRadius());
        l_pTheRegridder->SetStepSize(m_reconstructionParams->GetStepSize());
        l_pTheRegridder->SetSigma(m_reconstructionParams->GetSigma());
        l_pTheRegridder->SetSaturationBasedExtraction(m_reconstructionParams->GetReconstructionType()==1);
        l_pTheRegridder->SetValueThreshold(m_reconstructionParams->GetSaturationThreshold());


        //Create and activate the progress counter widget
        QProgressDialog * l_pProgress = new QProgressDialog("Processing frames",
                                "Cancel",
                                l_filenames.size(),
                                0, 0, TRUE,
                                Qt::WDestructiveClose);

        l_pProgress->setCaption("Processing...");
        l_pProgress->show();

        //Start sticking everything into the volume
        //THIS IS WHERE EVERYTHING HAPPENS
        int l_counter=0;
        for(QStringList::Iterator it=l_filenames.begin();
            l_processOK && it!=l_filenames.end();
            ++it)
        {
            if(QFile::exists(*it))
            {
//                cout << (*it) << endl;
                if(!l_pTheRegridder->RegridFile(*it))
                {
                    l_processOK = false;
                    break;
                }
            }

            l_pProgress->setProgress(++l_counter);
            qApp->processEvents();
            if ( l_pProgress->wasCanceled() ) 
            {
                QMessageBox::information( 0, "Processing", "Process cancelled", 1, 0 );
                l_processOK = false;
            }
        }
        l_pProgress->close();

        if (!l_processOK)
        {
            l_pTheRegridder->Delete();
            return;
        }

        //Go create the volume and specify to ask for file exportation
        this->CreateVolumeObject(l_pTheRegridder, *(l_filenames.begin()), true, info);

        //delete the regridder, which is no longer needed
        l_pTheRegridder->Delete();
    }
}


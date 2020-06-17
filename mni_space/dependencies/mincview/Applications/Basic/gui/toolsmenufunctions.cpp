/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: toolsmenufunctions.cpp,v $
  Language:  C++
  Date:      $Date: 2010-04-08 14:56:03 $
  Version:   $Revision: 1.9 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "toolsmenufunctions.h"

#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qlayout.h>
#include "qtutilities.h"
#include "pickingdialog.h"
#include "regridvolume.h"
#include "imageobject.h"
#include "simplevolumizedialog.h"
#include "usvolumereconstructionsettings.h"
#include "volumereconstruction.h"
#include "doublewindowfileviewer.h"
#include "multiplefileviewerdialog.h"

ToolsMenuFunctions::ToolsMenuFunctions():
    m_sceneManager(0),
    m_reconstructionParams(0)
{
    m_lastVisitedDirectory = QDir::homeDirPath();
}

ToolsMenuFunctions::~ToolsMenuFunctions()
{
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
}

void ToolsMenuFunctions::SetManager(SceneManager *manager)
{
    if (m_sceneManager == manager)
        return;
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
    m_sceneManager = manager;
    if (m_sceneManager)
        m_sceneManager->Register(0);
}

void ToolsMenuFunctions::SetLastVisitedDirectory(const QString & dir)
{
    m_lastVisitedDirectory = dir;
    if( !QFile::exists( m_lastVisitedDirectory ) )
    {
        m_lastVisitedDirectory = QDir::homeDirPath();
    }
}

void  ToolsMenuFunctions::EnablePicker(QWidget *parent)
{
    if (m_sceneManager)
    {
        PickingDialog *pickDialog = new PickingDialog(parent, "PickingDialog", FALSE, Qt::WDestructiveClose | Qt::WStyle_StaysOnTop);
        pickDialog->SetSceneManager(m_sceneManager);
        pickDialog->show();
    }
}

void ToolsMenuFunctions::OpenVolumeReconstructionDialog(QWidget *parent)
{
    if (m_sceneManager)
    {
        m_reconstructionParams = new USVolumeReconstructionSettings;
        SimpleVolumizeDialog *dlg = new SimpleVolumizeDialog(parent);
        dlg->SetUSReconstructionParams(m_reconstructionParams);
        dlg->Init(false, m_lastVisitedDirectory);
        if (dlg->exec() == QDialog::Accepted)
        {
            VolumeReconstruction *volume  = VolumeReconstruction::New();
            volume->SetLastVisitedDirectory(m_lastVisitedDirectory);
            volume->SetSceneManager(m_sceneManager);
            volume->SetUSReconstructionParams(m_reconstructionParams);
            volume->VolumeReconstructionFromImportedVideoFrames();
            volume->Delete();
        }
        delete m_reconstructionParams;
        m_reconstructionParams = 0;
    }   
}

void ToolsMenuFunctions::OpenShowMultipleFilesDialog(QWidget *parent)
{
    if(m_sceneManager)
    {
        QWidget *w = GetWidgetByName( "MultipleFileViewerParentDialog" );
        if( !w )
        {
            QDialog * dialog = new QDialog( parent, "MultipleFileViewerParentDialog", FALSE, Qt::WDestructiveClose);
            dialog->setCaption( "Ultrasound View" );
            QVBoxLayout * layout = new QVBoxLayout( dialog, 0, 0, "MultipleFileViewerParentDialogLayout" );
            DoubleWindowFileViewer *dw = new DoubleWindowFileViewer(dialog, "MultipleFileView", Qt::WDestructiveClose);
            layout->addWidget( dw );
            MultipleFileViewerDialog * viewerControl = new MultipleFileViewerDialog( dialog, "Viewer Control", Qt::WDestructiveClose );
            viewerControl->SetLastVisitedDirectory(m_lastVisitedDirectory);
            viewerControl->SetSceneManager(m_sceneManager);
            viewerControl->SetViewer(dw);
            viewerControl->Initialize();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
            dw->SetLogger(this->GetLogger());
            viewerControl->SetLogger(this->GetLogger());
#endif
            layout->addWidget( viewerControl );
            dialog->show();
        }
        else
        {
            w->showNormal();
            w->raise();
            w->setActiveWindow();
        }
    }
}


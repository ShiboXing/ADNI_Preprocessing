/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: savepointdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-05-05 19:00:37 $
  Version:   $Revision: 1.7 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#include "savepointdialog.h"

#include <strstream>
#include "vtkTagReader.h"
#include "vtkPoints.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

#include "scenemanager.h"
#include "ignsmsg.h"
#include "pointsobject.h"

SavePointDialog::SavePointDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : SavePointDialogBase( parent, name, modal, fl )
{
    m_directory = QDir::homeDirPath();
    m_points = vtkPoints::New();
    m_manager = 0;
    m_pointsObject = 0;
    m_saveTime = QDateTime::currentDateTime();
}

SavePointDialog::~SavePointDialog()
{
    m_points->Delete();
    if (m_manager)
        m_manager->UnRegister(0);
}

void SavePointDialog::OkButtonClicked()
{
    if (m_outputFile.isEmpty())
        return;
    this->AddPoint(m_currentPointCoords); // we add the new point to the list of points here, to make sure it is the last on the list
    QString name = pointNameLineEdit->text();
    m_displayPointObjectName = displayNameLineEdit->text();
    if (name.isEmpty() || m_points->GetNumberOfPoints() == 0)
        return;
    m_pointNames.push_back(std::string(name.ascii()));
    QFileInfo info(m_outputFile);
    if (m_displayPointObjectName.isEmpty())
    {
        m_displayPointObjectName = info.fileName();
    }

    if (m_manager)
    {
        SceneObject *attachTo = m_manager->GetObject(PREOP_ROOT_OBJECT_NAME);
        if (m_manager->GetRegistrationFlag())
            attachTo = m_manager->GetObject(INTRAOP_ROOT_OBJECT_NAME);
        m_pointsObject = m_manager->CreatePointsObject(m_displayPointObjectName, attachTo);
        if (m_pointsObject)
        {
            m_pointsObject->SetDataFileName(info.fileName());
            m_pointsObject->SetFullFileName(m_outputFile);
            m_pointsObject->SetObjectType(POINTS_OBJ_TYPE | PREOP_TYPE);
            m_pointsObject->AddPoint(name, m_currentPointCoords);
        }
    }
    QString saveTimeStamp = m_saveTime.toString(Qt::ISODate);
    m_pointsObject->WriteTagFile(m_outputFile, saveTimeStamp);
    accept();
}

void SavePointDialog::BrowsePushButtonClicked()
{
    if (!m_manager || m_directory.isNull() || m_directory.isEmpty())
        m_directory = QDir::homeDirPath();
    QString filename = QFileDialog::getSaveFileName(m_directory, "Tag file (*.tag)", 0, "open", "Save Point Tag File" );
    
    if(filename.isEmpty())
        return;
    
    if (QFile::exists(filename)) 
    {
        if (addToTagCheckBox->isChecked())
        {// read the tag file
            vtkTagReader * reader = vtkTagReader::New();
            if( reader->CanReadFile( filename ) )
            {    
                reader->SetFileName( filename );
                reader->Update();
                this->SetPointNames(reader->GetPointNames());
                this->SetVolumeNames(reader->GetVolumeNames());
                {
                    vtkPoints *pts = reader->GetVolume( 0 );
                    int i, n = pts->GetNumberOfPoints();
                    SceneObject *obj = m_manager->GetObject(m_volumeNames[0]);
                    for (i = 0; i < n; i++)
                    {
                        this->AddPoint(pts->GetPoint(i));
                    }
                    if (!obj)
                    {
                        SceneObject *attachTo = m_manager->GetObject(PREOP_ROOT_OBJECT_NAME);
                        if (m_manager->GetRegistrationFlag())
                            attachTo = m_manager->GetObject(INTRAOP_ROOT_OBJECT_NAME);
                        m_pointsObject = m_manager->CreatePointsObject(m_volumeNames[0]);
                        if (m_pointsObject)
                            for (i = 0; i < n; i++)
                            {
                                m_pointsObject->AddPoint(m_pointNames[i], pts->GetPoint(i));
                            }
                    }
                    else
                    {
                        if (obj->GetObjectType() & POINTS_OBJ_TYPE)
                            m_pointsObject = reinterpret_cast<PointsObject*>(obj);
                        else
                        {
                            reader->Delete();
                            QMessageBox::warning(this, "Save Point Tag File",
                                                        tr("Cannot read from file: ")
                                                                .arg(QDir::convertSeparators(filename)),
                                                        QMessageBox::Yes | QMessageBox::Default,
                                                        QMessageBox::No | QMessageBox::Escape);
                            return;
                        }
                    }
                }
                reader->Delete();
            }
            else
            {
                reader->Delete();
                QMessageBox::warning(this, "Save Point Tag File",
                                            tr("Cannot read from file: ")
                                                    .arg(QDir::convertSeparators(filename)),
                                            QMessageBox::Yes | QMessageBox::Default,
                                            QMessageBox::No | QMessageBox::Escape);
                return;
            }
        }
        else
        {
            int ret = QMessageBox::warning(this, "Save Point Tag File",
                                        tr("File %1 already exists.\n"
                                                "Do you want to overwrite it?")
                                                .arg(QDir::convertSeparators(filename)),
                                        QMessageBox::Yes | QMessageBox::Default,
                                        QMessageBox::No | QMessageBox::Escape);
            if (ret == QMessageBox::No)
                return;
        }
    }
    
    if(!filename.isEmpty())
    {
        QFileInfo info(filename);
        QString dirPath = (const char*)info.dirPath( TRUE );
        QFileInfo info1(dirPath);
        if (!info1.isWritable())
        {
            QString accessError = IGNS_MSG_NO_WRITE + dirPath;
            QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
            return;
        }
        m_outputFile = filename;
        fileNameLineEdit->setText(m_outputFile);
        if (displayNameLineEdit->text().isNull() || displayNameLineEdit->text().isEmpty())
        {
            if (addToTagCheckBox->isChecked())
                displayNameLineEdit->setText(m_volumeNames[0]);
            else
                displayNameLineEdit->setText(info.fileName());
        }
    }
}

void SavePointDialog::FileNameEntered()
{
    m_outputFile = fileNameLineEdit->text();
}

void SavePointDialog::SetSaveDirectory(const QString & dir)
{
    m_directory = dir;
    if( !QFile::exists( m_directory ) )
    {
        m_directory = QDir::homeDirPath();
    }
}

void SavePointDialog::AddPoint(double x, double y, double z)
{
    m_points->InsertNextPoint(x, y, z);
}

void SavePointDialog::AddPoint(double xyz[3])
{
    m_points->InsertNextPoint(xyz);
}

void SavePointDialog::SetCurrentPointCoords(double xyz[3])
{
    for(int i = 0; i < 3; i++)
        m_currentPointCoords[i] = xyz[i];
}

void SavePointDialog::SetPointNames(std::vector<std::string> & pointNames)
{
    m_pointNames.clear();
    std::vector<std::string>::iterator it = pointNames.begin();
    for( ; it != pointNames.end(); ++it )
    {
        m_pointNames.push_back( QString( (*it).c_str() ) );
    }
}

void SavePointDialog::SetVolumeNames(std::vector<std::string> & volumeNames)
{
    m_volumeNames.clear();
    std::vector<std::string>::iterator it = volumeNames.begin();
    for( ; it != volumeNames.end(); ++it )
    {
        m_volumeNames.push_back( QString( (*it).c_str() ) );
    }
}

void SavePointDialog::SetManager( SceneManager *manager )
{
    if (m_manager != manager)                                       
    {                                                           
        if (m_manager) 
            m_manager->UnRegister(0);
        m_manager = manager;
        if (m_manager) 
            m_manager->Register(0);
    }
}

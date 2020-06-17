/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: opendatafiledialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-04-08 19:02:13 $
  Version:   $Revision: 1.13 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/

#include "opendatafiledialog.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qdir.h>

#include "vtkTransform.h"
#include "sceneobject.h"
#include "usertransformations.h"
#include "ignsconfig.h"

OpenFileOutputParams::OpenFileOutputParams()
{
    type = PREOP_TYPE;
    userTransforms = 0;
    referenceObjectAlreadySet = FALSE;
}

OpenFileOutputParams::~OpenFileOutputParams()
{
}
    
OpenDataFileDialog::OpenDataFileDialog( QWidget* parent, const char* name, bool modal, WFlags fl, OpenFileOutputParams *params )
    : OpenDataFileDialogBase( parent, name, modal, fl )
{
    if ( !name )
        setName( "OpenDataFileDialog" );
    m_fileParams = params;
    m_expertMode = 0;
    QStringList tmp;
    if (m_fileParams->userTransforms)
    {
        m_fileParams->userTransforms->GetTransformList(tmp);
        transformComboBox->insertStringList(tmp);
    }
    transformComboBox->setCurrentItem(0);
    if (m_fileParams->referenceObjectAlreadySet)
    {
        referenceImageTypeCheckBox->setChecked(FALSE);
        referenceImageTypeCheckBox->setEnabled(FALSE);
    }
    else
        referenceImageTypeCheckBox->setChecked(TRUE);
}

OpenDataFileDialog::~OpenDataFileDialog()
{
}

void OpenDataFileDialog::BrowsePushButtonClicked()
{
    if (m_expertMode && m_expertMode->GetExpertLevel() > 1)
    {
        m_fileParams->inputFiles = QFileDialog::getOpenFileNames("All valid files(*.mnc;*.obj;*.tag;*.vtk);;Minc file (*.mnc);;Object file (*.obj);;Tag file (*.tag);;VTK file (*.vtk)",
                                m_fileParams->lastVisitedDir,
                                0,
                                "open",
                                "Open files");
    }
    else  
    {
        m_fileParams->inputFiles = QFileDialog::getOpenFileNames("All valid files(*.mnc;*.obj;*.tag);;Minc file (*.mnc);;Object file (*.obj)Tag file *.tag);;",
                                m_fileParams->lastVisitedDir,
                                0,
                                "open",
                                "Open files");
    }                            
    QString tmp;
    for ( QStringList::Iterator it = m_fileParams->inputFiles.begin(); it != m_fileParams->inputFiles.end(); ++it )
    {
        tmp += *it + " ";
    }
      
    openFileLineEdit->setText(tmp);
}

void OpenDataFileDialog::OKButtonClicked()
{
    m_fileParams->type = PREOP_TYPE;
    if (dataTypeButtonGroup->selectedId() > 0)
    {
        m_fileParams->type = INTRAOP_TYPE;
        m_fileParams->transformName = transformComboBox->currentText();
    }
    m_fileParams->dataObjectName = objectNameLineEdit->text();
    if (referenceImageTypeCheckBox->isChecked())
        m_fileParams->type |= REFERENCE_DATA_TYPE;
    
    accept();
}

void OpenDataFileDialog::DataTypeSelected(int type)
{
    if (type > 0)
    {
        m_fileParams->type = INTRAOP_TYPE;
        transformComboBox->setEnabled(TRUE);
        referenceImageTypeCheckBox->setChecked(FALSE);
        referenceImageTypeCheckBox->setEnabled(FALSE);
    }
    else
    {
        m_fileParams->type = PREOP_TYPE;
        transformComboBox->setEnabled(FALSE);
    }
}

void OpenDataFileDialog::TransformComboBoxActivated(const QString& name)
{
}

void OpenDataFileDialog::ReferenceImageTypeCheckBoxToggled(bool)
{
}

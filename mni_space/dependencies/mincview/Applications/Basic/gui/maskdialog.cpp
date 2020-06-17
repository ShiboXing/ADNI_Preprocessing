/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: maskdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-03-31 14:27:27 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "maskdialog.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "usvolumereconstructionsettings.h"
#include "ignsconfig.h"
#include "ignsmsg.h"

MaskDialog::MaskDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : MaskDialogBase( parent, name, modal, fl ),
    m_params(0),
    m_maskType(DEFAULT_MASK)
{
    if ( !name )
	setName( "MaskDialog" );
    this->MaskTypeChanged(DEFAULT_MASK);       
}

MaskDialog::~MaskDialog()
{
}

void MaskDialog::OKButtonClicked()
{
    //transfer the data back to USVolumeReconstructionSettings class
    if (m_params)
    {
        m_params->SetMaskType(m_maskType);
        m_params->SetCustomMaskFileName(maskLineEdit->text());
    }
    accept();
}

void MaskDialog::MaskTypeChanged(int type)
{
    QString maskFile;
    switch (type)
    {
        case DEFAULT_MASK:
        default:
            m_maskType = DEFAULT_MASK;
            if (m_params)
                m_params->GetDefaultMaskName(maskFile);
            maskLineEdit->setText(maskFile);
            maskLineEdit->setEnabled(false);
            customBrowsePushButton->setEnabled(false);
            break;
        case CUSTOM_MASK:
            m_maskType = CUSTOM_MASK;
            maskLineEdit->setEnabled(true);
            customBrowsePushButton->setEnabled(true);
            break;
        case NO_MASK:
            m_maskType = NO_MASK;
            maskLineEdit->clear();
            maskLineEdit->setEnabled(false);
            customBrowsePushButton->setEnabled(false);
            break;
        }
}

void MaskDialog::CustomMaskBrowseButtonClicked()
{
    QString mask_directory;
    QString customMask = maskLineEdit->text();
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
            maskLineEdit->setText(customMask);
            return;
        }
        if (!fi.isReadable())
        {
            QString accessError = IGNS_MSG_NO_READ + filename;
            QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
            maskLineEdit->setText(customMask);
            return;
        }
    }
    maskLineEdit->setText(filename);
    if (m_params)
        m_params->SetCustomMaskFileName(filename);
}

void MaskDialog::SetMaskParameters(USVolumeReconstructionSettings *params)
{
    bool cheat = false;
    m_params = params;
    QString tmp = "no settings";
    if (m_params->GetUSMode() > 0)
    {
        tmp = "grayscale (b-mode), " + m_params->GetUSDepth();
    }
    else
    {
        tmp = "RGB (doppler), " + m_params->GetUSDepth();
        m_params->SetUSMode(1);
        cheat = true;
    }
    QString maskFile;
    m_params->GetDefaultMaskName(maskFile);
    maskLineEdit->setText(maskFile);
    currentSettingsTextLabel->setText(tmp);
    if (cheat)
        m_params->SetUSMode(0);
}


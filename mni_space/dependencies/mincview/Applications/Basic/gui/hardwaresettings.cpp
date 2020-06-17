/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: hardwaresettings.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.11 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#include <qdir.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include "hardwaresettings.h"
#include "ignsconfig.h"
#include "ignsmsg.h"
#include "qevent.h"

HardwareSettings::HardwareSettings(QWidget * parent, const char * name, WFlags f, const char * defaultCfg) :
HardwareSettingsBase(parent, name, f)
{
    QString tmp;
    if (defaultCfg)
        tmp = QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY + "/" + defaultCfg;
    else
        tmp = QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY + "/" + IGNS_IBIS_CONFIG;
    QFile settings(tmp);
    if (settings.exists())
    {
        DefaultHardwareSettingsPath = tmp;
        UserHardwareSettingsPath = DefaultHardwareSettingsPath;
        SelectedHardwareSettingsPath = DefaultHardwareSettingsPath;
        DefaultHardwareSettingsFilename->setText(DefaultHardwareSettingsPath);
        UserSettingsLineEdit->setText(UserHardwareSettingsPath);
        settingsOK = true;
    }
    else
    {
        DefaultHardwareSettingsPath = "";
        UserHardwareSettingsPath = DefaultHardwareSettingsPath;
        SelectedHardwareSettingsPath = DefaultHardwareSettingsPath;
        DefaultHardwareSettingsFilename->setText(IGNS_MSG_NO_CONFIG_FILE);
        DefaultHardwareSettingsFilename->setPaletteForegroundColor ( Qt::red );
        UserSettingsLineEdit->setText(UserHardwareSettingsPath);
        LoadUserSettingsButton->setEnabled(0);
        settingsOK = false;
    }
    checkOnClose = true;
}

HardwareSettings::~HardwareSettings()
{
}

void HardwareSettings::UserSettingsChanged()
{
    QString tmp;
    UserHardwareSettingsPath = UserSettingsLineEdit->text();
    if (!UserHardwareSettingsPath.isEmpty())
    {
        LoadUserSettingsButton->setEnabled(1);
        QFile f(UserHardwareSettingsPath);
        if (f.exists())
            settingsOK = true;
    }
    else
    {
        if (!DefaultHardwareSettingsPath.isEmpty())
        {
            tmp = IGNS_MSG_FILE_NOT_EXISTS + UserHardwareSettingsPath + IGNS_MSG_RESTORE_DEFAULT;
            UserSettingsLineEdit->setText(DefaultHardwareSettingsPath);
            QMessageBox::critical( this, IGNS_MSG_ERROR, tmp, 1, 0 );
            settingsOK = true;
            LoadUserSettingsButton->setEnabled(1);
        }
    }
    if (!settingsOK)
    {
        if (UserHardwareSettingsPath.isEmpty())
            tmp = IGNS_MSG_MAKE_CONFIG;
        else
            tmp = IGNS_MSG_NO_CONFIG_FILE;
        QMessageBox::critical( this, IGNS_MSG_ERROR, tmp, 1, 0 );
        LoadUserSettingsButton->setEnabled(0);
    }
    else
        LoadUserSettings();
    
}

void HardwareSettings::LoadUserSettings()
{
    QFile f(UserSettingsLineEdit->text());
    if (f.exists())
    {
        SelectedHardwareSettingsPath = UserHardwareSettingsPath;
        checkOnClose = false;
        settingsOK = true;
        accept(); 
    } 
    else                 
        UserSettingsChanged();
}

void HardwareSettings::FindUserSettingsFile()
{
    QString filename;
    QString dir(QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY);
    filename = QFileDialog::getOpenFileName( dir, IGNS_MSG_CFG_FILES, 0 , IGNS_MSG_OPEN, IGNS_MSG_OPEN_CFG_FILE ); 
    if( !filename.isNull() )
    {
        UserHardwareSettingsPath = filename;
        UserSettingsLineEdit->setText(filename);
    }
}

void HardwareSettings::ActivateLoadButton()
{
    UserHardwareSettingsPath = UserSettingsLineEdit->text();
    if (!UserHardwareSettingsPath.isEmpty())
        LoadUserSettingsButton->setEnabled(1);
    else
        LoadUserSettingsButton->setEnabled(0);
}

void HardwareSettings::closeEvent( QCloseEvent* ce )
{
    if (checkOnClose)
    {
        QString tmp;
        settingsOK = false;
        QFile f(UserSettingsLineEdit->text());
        if (f.exists())
        {
            tmp = IGNS_MSG_LOAD_USER_CFG;
            QMessageBox::critical( this, IGNS_MSG_INFO, tmp, 1, 0 );
            SelectedHardwareSettingsPath = UserSettingsLineEdit->text();
            settingsOK = true;
        }
        else if (!DefaultHardwareSettingsPath.isEmpty())
        {
            tmp = IGNS_MSG_LOAD_DEFAULT_CFG;
            QMessageBox::critical( this, IGNS_MSG_INFO, tmp, 1, 0 );
            SelectedHardwareSettingsPath = DefaultHardwareSettingsPath;
            settingsOK = true;
        }
    }
    ce->accept();                   
}

bool HardwareSettings::GetSettingsStatus()
{
    if (!settingsOK)
        QMessageBox::critical( this, IGNS_MSG_INFO, IGNS_MSG_MAKE_CONFIG, 1, 0 );
    return settingsOK;
}

void HardwareSettings::QuitApplication()
{
    reject();
}

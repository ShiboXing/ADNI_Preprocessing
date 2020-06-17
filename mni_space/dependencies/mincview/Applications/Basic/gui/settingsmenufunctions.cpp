/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: settingsmenufunctions.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "settingsmenufunctions.h"

#include <qdir.h>

SettingsMenuFunctions::SettingsMenuFunctions():
    m_sceneManager(0)    
{
    m_lastVisitedDirectory = QDir::homeDirPath();
}

SettingsMenuFunctions::~SettingsMenuFunctions()
{
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
}

void SettingsMenuFunctions::SetManager(SceneManager *manager)
{
    if (m_sceneManager == manager)
        return;
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
    m_sceneManager = manager;
    if (m_sceneManager)
        m_sceneManager->Register(0);
}

void SettingsMenuFunctions::SetLastVisitedDirectory(const QString & dir)
{
    m_lastVisitedDirectory = dir;
    if( !QFile::exists( m_lastVisitedDirectory ) )
    {
        m_lastVisitedDirectory = QDir::homeDirPath();
    }
}

void SettingsMenuFunctions::SetBackgroundColor()
{
    if (m_sceneManager)
    {
        QWidget *res = GetWidgetByName( "BackgroundSettings" );
        if( !res )
        {
            res = m_sceneManager->CreateSettingsDialog( 0 );
            res->show();
        }
        else
        {
            res->show();
            res->raise();
            res->setActiveWindow();
        }
    }
}

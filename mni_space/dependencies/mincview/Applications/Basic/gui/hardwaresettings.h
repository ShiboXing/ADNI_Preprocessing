/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: hardwaresettings.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.8 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/
#ifndef TAG_HARDWARESETTINGS_H
#define TAG_HARDWARESETTINGS_H

#include "hardwaresettingsbase.h"
#include <qstring.h>

class QCloseEvent;

class HardwareSettings : public HardwareSettingsBase
{
Q_OBJECT

public:

    HardwareSettings( QWidget * parent = 0, const char * name = 0, WFlags f = 0, const char * defaultCfg = 0 );
    virtual ~HardwareSettings();
    
    const char *  GetDefaultHardwareSettingsPath() {return DefaultHardwareSettingsPath;}
    const char *  GetUserHardwareSettingsPath() {return UserHardwareSettingsPath;}   
    const char *  GetSelectedHardwareSettingsPath() {return SelectedHardwareSettingsPath;}   
    bool GetSettingsStatus();
       
public slots:
    virtual void UserSettingsChanged();
    virtual void LoadUserSettings();
    virtual void FindUserSettingsFile();
    virtual void ActivateLoadButton();
    virtual void closeEvent( QCloseEvent* ce );
    virtual void QuitApplication();

protected:
    
    QString DefaultHardwareSettingsPath;
    QString UserHardwareSettingsPath;
    QString SelectedHardwareSettingsPath;
    bool checkOnClose;
    bool settingsOK; 
};


#endif  // TAG_HARDWARESETTINGS_H

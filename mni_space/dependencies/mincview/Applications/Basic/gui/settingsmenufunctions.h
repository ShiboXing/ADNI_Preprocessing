/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: settingsmenufunctions.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
// .NAME SettingsMenuFunctions - all the functions serving the settings menu in MainWindow
// .SECTION Description
// This file is created in order to remove the bulk of the code from MainWindow
// in ibis and MincView
#ifndef TAG_SETTINGSMENUFUNCTIONS_H_
#define TAG_SETTINGSMENUFUNCTIONS_H_

#include <qobject.h>
#include "qtutilities.h"
#include "scenemanager.h"

class SettingsMenuFunctions : public QObject
{

    Q_OBJECT

public:

    SettingsMenuFunctions();
    virtual ~SettingsMenuFunctions();
    
    // Description:
    // Set view windows background color
    virtual void SetBackgroundColor();
    
    // Description:
    // Change view mode
    void SetManager(SceneManager *manager);
    // Description:
    // Set starting data directory
    void SetLastVisitedDirectory(const QString & dir);

protected:

    QString m_lastVisitedDirectory;  
    SceneManager    * m_sceneManager;
};

#endif //TAG_SETTINGSMENUFUNCTIONS_H_

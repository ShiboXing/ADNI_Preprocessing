/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: setopacitydialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef SETOPACITYDIALOG_H
#define SETOPACITYDIALOG_H

#include "setopacitydialogbase.h"
#include "scenemanager.h"


class SetOpacityDialog : public SetOpacityDialogBase
{
    Q_OBJECT

public:
    SetOpacityDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~SetOpacityDialog();

    // Description:
    // Set scene manager
    void SetManager(SceneManager *manager);
    
public slots:
    virtual void OkButtonClicked();
    
protected:
    SceneManager    * m_sceneManager;
};

#endif // SETOPACITYDIALOG_H

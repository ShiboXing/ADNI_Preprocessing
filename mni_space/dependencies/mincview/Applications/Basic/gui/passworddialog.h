/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: passworddialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/
#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include "passworddialogbase.h"

class PasswordDialog : public PasswordDialogBase
{
    Q_OBJECT

public:
    PasswordDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PasswordDialog();

    const char *GetWord();
    
public slots:
    virtual void OkButtonClicked();

protected:
    QString m_passwd;
};

#endif // PASSWORDDIALOG_H

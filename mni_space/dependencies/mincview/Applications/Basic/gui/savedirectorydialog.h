/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: savedirectorydialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef TAG_SAVEDIRECTORYDIALOG_H
#define TAG_SAVEDIRECTORYDIALOG_H

#include "entertextdialogbase.h"
#include <qdialog.h>

class SaveDirectoryDialog : public EnterTextDialogBase
{
    Q_OBJECT

public:
    SaveDirectoryDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~SaveDirectoryDialog();

    virtual void SetInfoText(const QString &);
    virtual void SetInputLineText(QString & text);
    
public slots:
    virtual void OKButtonClicked();
    virtual void TextEntered();

protected:

};

#endif // TAG_SAVEDIRECTORYDIALOG_H

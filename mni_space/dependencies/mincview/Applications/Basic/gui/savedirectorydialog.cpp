/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: savedirectorydialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "savedirectorydialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

SaveDirectoryDialog::SaveDirectoryDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : EnterTextDialogBase( parent, name, modal, fl )
{
    setName( "SaveDirectoryDialog" );
    setSizeGripEnabled( FALSE );

}

SaveDirectoryDialog::~SaveDirectoryDialog()
{
    // no need to delete child widgets, Qt does it all for us
}


void SaveDirectoryDialog::OKButtonClicked()
{
    accept();
}

void SaveDirectoryDialog::SetInfoText(const QString & info)
{
    infoTextLabel->setText(info);
}

void SaveDirectoryDialog::SetInputLineText(QString & text)
{
    inputTextLineEdit->setText(text);
}

void SaveDirectoryDialog::TextEntered()
{
}


/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: passworddialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/
#include "passworddialog.h"
#include "qlineedit.h"

PasswordDialog::PasswordDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : PasswordDialogBase( parent, name, modal, fl )
{
    if ( !name )
	setName( "PasswordDialog" );
    passwordLineEdit->setEchoMode(QLineEdit::Password);
}

PasswordDialog::~PasswordDialog()
{
}


void PasswordDialog::OkButtonClicked()
{
    m_passwd = passwordLineEdit->text();
    accept();
}

const char * PasswordDialog::GetWord()
{
    if (!m_passwd.isEmpty())
        return m_passwd.ascii();
    return 0;        
}

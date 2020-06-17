/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: setopacitydialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#include "setopacitydialog.h"

#include <qlineedit.h>
#include <qstring.h>


SetOpacityDialog::SetOpacityDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : SetOpacityDialogBase( parent, name, modal, fl ),
    m_sceneManager(0)
{
    if ( !name )
	setName( "SetOpacityDialog" );
}

SetOpacityDialog::~SetOpacityDialog()
{
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
}

void SetOpacityDialog::OkButtonClicked()
{
    QString t = opacityLineEdit->text();
    double opacity = t.toInt()/100.0;
    if (m_sceneManager)
        m_sceneManager->SetAllObjectsOpacity(opacity);
    accept();        
}

void SetOpacityDialog::SetManager(SceneManager *manager)
{
    if (m_sceneManager == manager)
        return;
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
    m_sceneManager = manager;
    if (m_sceneManager)
        m_sceneManager->Register(0);
    opacityLineEdit->setText(QString::number((int)(m_sceneManager->GetGlobalOpacity()*100)));
}

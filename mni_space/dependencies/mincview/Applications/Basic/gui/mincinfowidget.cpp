/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/

#include "mincinfowidget.h"
#include <qtextedit.h>

MincInfoWidget::MincInfoWidget( QWidget* parent, const char* name, WFlags fl )
    : MincInfoWidgetBase( parent, name, fl )
{
    setCaption( "Minc Header" );
}

MincInfoWidget::~MincInfoWidget()
{
}

void MincInfoWidget::SetInfoText(QString info)
{
    mincInfoTextEdit->setText(info);
}


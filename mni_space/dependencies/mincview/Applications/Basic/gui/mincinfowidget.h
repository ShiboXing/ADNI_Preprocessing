/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: mincinfowidget.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/

#ifndef MINCINFOWIDGET_H
#define MINCINFOWIDGET_H

#include "mincinfowidgetbase.h"
#include <qstring.h>

class MincInfoWidget : public MincInfoWidgetBase
{
    Q_OBJECT

public:
    MincInfoWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~MincInfoWidget();

    void SetInfoText(QString info);

};

#endif // MINCINFOWIDGET_H

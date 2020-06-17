/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: removepointdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef REMOVEPOINTDIALOG_H
#define REMOVEPOINTDIALOG_H

#include "removepointdialogbase.h"

class PointsObject;

class RemovePointDialog : public RemovePointDialogBase
{
    Q_OBJECT

public:
    RemovePointDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~RemovePointDialog();

    void SetPointsObject(PointsObject *o) {m_points = o;}
    void SetSelectedPointIndex(int num);

public slots:
    void OKButtonClicked();
    void CancelButtonClicked();

protected:
    QHBoxLayout* removePointLayout;

    int m_selectedPointIndex;
    PointsObject *m_points;
    
};

#endif // REMOVEPOINTDIALOG_H

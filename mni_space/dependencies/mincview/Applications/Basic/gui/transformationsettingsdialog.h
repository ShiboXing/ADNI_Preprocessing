/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: transformationsettingsdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:07 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef TRANSFORMATIONSETTINGSDIALOG_H
#define TRANSFORMATIONSETTINGSDIALOG_H

#include "transformationsettingsdialogbase.h"

#include <qdialog.h>


class TransformationSettingsDialog : public TransformationSettingsDialogBase
{
    Q_OBJECT

public:
    TransformationSettingsDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~TransformationSettingsDialog();

//    QLineEdit* transformationNameLineEdit;
//    QPushButton* transformMatrixPushButton;

public slots:
    virtual void TransformationMatrixPushButtonClicked();
    virtual void OKPushButtonClicked();
    virtual void NewMatrixDialogClosed();

protected:
    QDialog * m_parentNewMatrixDialog;
};

#endif // TRANSFORMATIONSETTINGSDIALOG_H

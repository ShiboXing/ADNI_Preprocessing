/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: transformationsettingsdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:07 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#include "transformationsettingsdialog.h"

#include <qmessagebox.h>
#include <qwidget.h>
#include <qlineedit.h>
#include "vtkQtMatrixDialog.h"
#include "vtkMatrix4x4.h"

TransformationSettingsDialog::TransformationSettingsDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : TransformationSettingsDialogBase( parent, name, modal, fl ),
    m_parentNewMatrixDialog(0)
{
    if ( !name )
        setName( "TransformationSettingsDialog" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
TransformationSettingsDialog::~TransformationSettingsDialog()
{
    // no need to delete child widgets, Qt does it all for us
}


void TransformationSettingsDialog::TransformationMatrixPushButtonClicked()
{
    m_parentNewMatrixDialog = new QDialog( this, "NewMatrix", TRUE );
    m_parentNewMatrixDialog->setCaption( "Matrix" );
    vtkQtMatrixDialog * matrixWidget; // vtkQtMatrixDialog is subclassed from QWidget
    vtkMatrix4x4 *tmpMat = vtkMatrix4x4::New();
    tmpMat->Identity();
    matrixWidget = new vtkQtMatrixDialog( tmpMat, FALSE, m_parentNewMatrixDialog, "New Matrix" );
    connect( matrixWidget, SIGNAL( destroyed() ), SLOT( NewMatrixDialogClosed() ) );
    m_parentNewMatrixDialog->show();
}

void TransformationSettingsDialog::OKPushButtonClicked()
{
    QString name = transformationNameLineEdit->text();
    if (name.isEmpty())
    {
        QMessageBox::warning( this, "Error", tr("Please, enter transformation name."), 1, 0 );
        return;
    }
    accept();
}

void TransformationSettingsDialog::NewMatrixDialogClosed()
{
    delete m_parentNewMatrixDialog;
    m_parentNewMatrixDialog = 0;
}


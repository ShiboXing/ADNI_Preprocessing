/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: transformlistdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:07 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  Re-make from the old Simon's
  All rights reserved.
  
=========================================================================*/

#include <qstring.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlabel.h>
#include "vtkMatrix4x4.h"
#include "vtkQtMatrixDialog.h"
#include "transformlistdialog.h"
#include "usertransformations.h"

TransformListDialog::TransformListDialog( QWidget* parent, const char* name, WFlags fl )
    : TransformListDialogBase( parent, name, fl )
{
    if ( !name )
        setName( "TransformListDialog" );
    appliedTransformsListBox->clear();
    m_userTransforms = 0;
}

TransformListDialog::~TransformListDialog()
{
}

void TransformListDialog::TransformFromListSelected(const QString& transformName)
{
    if (m_userTransforms)
    {
        int type = m_userTransforms->GetTransformType(transformName);
        if (type == ABSOLUTE_TRANSFORM)
            currentTypeTextLabel->setText(IGNS_ABSOLUTE_TRANSFORM);
        else
            currentTypeTextLabel->setText(IGNS_INCREMENTAL_TRANSFORM);
        emit TransformationChanged(transformName);
    }
}

void TransformListDialog::ShowTransformationMatrix()
{
    if (m_userTransforms)
    {
        QString name = transformComboBox->currentText();
        vtkMatrix4x4 *tmpMat = m_userTransforms->GetTransformMatrix(name);
        vtkQtMatrixDialog * dlg;
        dlg = new vtkQtMatrixDialog( tmpMat, true, 0, "" );
        dlg->show();
    }
}

void TransformListDialog::UndoLastTransformation()
{
    if (m_userTransforms)
    {
        emit UndoTransformation();
    }
}

void TransformListDialog::SetUserTransforms( UserTransformations * tr )
{
    this->m_userTransforms = tr;
    if (m_userTransforms)
    {
        QStringList tmp;
        m_userTransforms->GetTransformList(tmp);
        transformComboBox->clear();
        transformComboBox->insertStringList(tmp);
        transformComboBox->setCurrentItem(0);
        int type = m_userTransforms->GetTransformType(0);
        if (type == ABSOLUTE_TRANSFORM)
            currentTypeTextLabel->setText(IGNS_ABSOLUTE_TRANSFORM);
        else
            currentTypeTextLabel->setText(IGNS_INCREMENTAL_TRANSFORM);
    }
}

void TransformListDialog::SetAppliedTransforms(std::vector<int> &transformIds)
{
    if (m_userTransforms)
    {
        appliedTransformsListBox->clear();

        std::vector<int>::iterator it = transformIds.begin();
        QString tmp;
        for( ; it != transformIds.end(); ++it )
        {
            tmp = m_userTransforms->GetTransformName((*it));
            appliedTransformsListBox->insertItem(tmp);
        }
    }
}

void TransformListDialog::SelectTransform(QString transformName)
{
    if (m_userTransforms)
        transformComboBox->setCurrentItem(m_userTransforms->GetTransformIndex(transformName));
}


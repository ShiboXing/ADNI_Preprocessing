/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: edittransformdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-04-28 19:05:52 $
  Version:   $Revision: 1.9 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "edittransformdialog.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include "vtkTransform.h"
#include "usertransformations.h"
#include "ignsmsg.h"
#include "vtkQtMatrixDialog.h"
#include "vtkMatrix4x4.h"

EditTransformDialog::EditTransformDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : EditTransformDialogBase( parent, name, modal, fl )
{
    if ( !name )
    setName( "EditTransformDialog" );
    transformTypeComboBox->insertItem(IGNS_ABSOLUTE_TRANSFORM, 0);
    transformTypeComboBox->insertItem(IGNS_INCREMENTAL_TRANSFORM, 1);
    m_editing = true;
    m_userTransforms = 0;
    m_matrix = vtkMatrix4x4::New();
    m_matrix->Identity();
    m_postMultiply = false;
}

EditTransformDialog::~EditTransformDialog()
{
    m_matrix->Delete();
}

void EditTransformDialog::OKButtonClicked()
{
    Q_ASSERT(m_userTransforms);
    QString name = transformNameLineEdit->text();
    int type = transformTypeComboBox->currentItem();

    if (!m_editing)
    {
        m_userTransforms->AddUserTransform(name, type, m_postMultiply, m_matrix);  
    }
    else if (!m_userTransforms->TransformIsPredefined(name))
    {
        QString oldName = transformListComboBox->currentText();
        m_userTransforms->UpdateUserTransform(oldName, name, type, m_postMultiply, m_matrix);  
    }
    emit TransformationAdded();
    accept();
}

void EditTransformDialog::SelectionButtonGroupClicked(int selection)
{
    if (selection == 0) // new
    {
        transformNameLineEdit->setText("");
        transformTypeComboBox->setCurrentItem(0);
        setTransformMatrixPushButton->setText(tr("Set Transformation Matrix"));
        transformNameLineEdit->setEnabled(TRUE);
        if (m_userTransforms->GetExpertLevel() == 0)
            postMultiplyCheckBox->hide();
        else
            postMultiplyCheckBox->setEnabled(TRUE);            
        m_matrix->Identity();
        m_editing = false;
    }
    else
    {
        m_editing = true;
    }                            
    this->UpdateUI();
}

void EditTransformDialog::TransformListComboBoxActivated(const QString& currentName)
{
    Q_ASSERT(m_userTransforms);
    vtkMatrix4x4 *tmpMat = m_userTransforms->GetTransformMatrix(currentName);
    if (tmpMat)
        m_matrix->DeepCopy(tmpMat);
    else
        m_matrix->Identity();
    this->UpdateUI();
}

void EditTransformDialog::TransformNameLineEditChanged()
{
    Q_ASSERT(m_userTransforms);
    QString name = transformNameLineEdit->text();
    if (m_userTransforms->GetTransformIndex(name) != -1)
    {
        transformNameLineEdit->setText("");
        QString tmp = IGNS_TRANSFORM_EXISTS + name;
        QMessageBox::warning( this, "Error: ", tmp, 1, 0 );
    }
}

void EditTransformDialog::TransformTypeComboBoxActivated(int)
{
    // nothing to do yet
}

void EditTransformDialog::SetTransformMatrixPushButtonClicked()
{
    Q_ASSERT(m_userTransforms);
    QString name = transformListComboBox->currentText();
    vtkMatrix4x4 *tmpMat = m_userTransforms->GetTransformMatrix(name);
    if (tmpMat)
        m_matrix->DeepCopy(tmpMat);
    else
        m_matrix->Identity();
    vtkQtMatrixDialog * dlg;
    if (m_editing  && m_userTransforms->TransformIsPredefined(name))
    {
        dlg = new vtkQtMatrixDialog( m_matrix, true, 0, "" );
    }
    else
    {
        dlg = new vtkQtMatrixDialog( m_matrix, false, 0, "" );
    }
    dlg->show();
}

void EditTransformDialog::SetUserTransforms( UserTransformations * trans )
{
    m_userTransforms = trans;
    Q_ASSERT(m_userTransforms);
    QStringList tmp;
    m_userTransforms->GetTransformList(tmp);
    transformListComboBox->insertStringList(tmp);
    transformListComboBox->setCurrentItem(0);
    transformTypeComboBox->setCurrentItem(0);
    this->UpdateUI();
    editRadioButton->setChecked(FALSE);
    newRadioButton->setChecked(FALSE);
    if (m_userTransforms->GetExpertLevel() == 0)
        postMultiplyCheckBox->hide();
    else
        postMultiplyCheckBox->setEnabled(TRUE);            
}

void EditTransformDialog::UpdateUI()
{
    Q_ASSERT(m_userTransforms);
    if (m_editing)
    {
        QString name = transformListComboBox->currentText();
        transformNameLineEdit->setText(name);
        bool predefined = m_userTransforms->TransformIsPredefined(name);
        int type = m_userTransforms->GetTransformType(name);
        if (type == INCREMENTAL_TRANSFORM)
        {
            transformTypeComboBox->setCurrentItem(1);
        }
        else
        {
            transformTypeComboBox->setCurrentItem(0);
        }
        if (!predefined)
        {
            setTransformMatrixPushButton->setText(tr("Set Transformation Matrix"));
            editRadioButton->setEnabled(TRUE);
            transformNameLineEdit->setEnabled(TRUE);
            if (m_userTransforms->GetExpertLevel() == 0)
                postMultiplyCheckBox->hide();
            else                
                postMultiplyCheckBox->setEnabled(TRUE);
            if (m_userTransforms->GetReferenceCounter(name) > 0)
                deleteTransformPushButton->setEnabled(FALSE);
            else
                deleteTransformPushButton->setEnabled(TRUE);
        }
        else
        {
            editRadioButton->setEnabled(FALSE);
            setTransformMatrixPushButton->setText(tr("Show Transformation Matrix"));
            transformNameLineEdit->setEnabled(FALSE);
            if (m_userTransforms->GetExpertLevel() == 0)
                postMultiplyCheckBox->hide();
            else                
                postMultiplyCheckBox->setEnabled(FALSE);            
            deleteTransformPushButton->setEnabled(FALSE);
        }
    }
}

void EditTransformDialog::PostMultiplyCheckBoxToggled(bool postMultiply)
{
    m_postMultiply = postMultiply;
}

void EditTransformDialog::DeleteTransformPushButtonClicked()
{
    Q_ASSERT(m_userTransforms);
    QString name = transformNameLineEdit->text();
    m_userTransforms->DeleteUserTransform(name);
}

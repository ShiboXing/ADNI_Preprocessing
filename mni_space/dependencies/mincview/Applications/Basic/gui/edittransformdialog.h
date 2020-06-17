/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: edittransformdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-04-28 19:05:52 $
  Version:   $Revision: 1.8 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/
/* Post Multiply check button sets the internal state of the transform to PostMultiply. All subsequent operations will occur after those already represented in the current transformation. In homogeneous matrix notation, M = A*M where M is the current transformation matrix and A is the applied matrix. The default is PreMultiply M = M*A.*/

#ifndef EDITTRANSFORMDIALOG_H
#define EDITTRANSFORMDIALOG_H

#include "edittransformdialogbase.h"
#include <qvariant.h>
#include <qdialog.h>

class UserTransformations;
class vtkMatrix4x4;

class EditTransformDialog : public EditTransformDialogBase
{
    Q_OBJECT

public:
    EditTransformDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~EditTransformDialog();
    
    virtual void SetUserTransforms( UserTransformations * tr );
    
public slots:
    virtual void OKButtonClicked();
    virtual void SelectionButtonGroupClicked(int);
    virtual void TransformListComboBoxActivated(const QString &);
    virtual void TransformNameLineEditChanged();
    virtual void TransformTypeComboBoxActivated(int);
    virtual void SetTransformMatrixPushButtonClicked();
    virtual void PostMultiplyCheckBoxToggled(bool);
    virtual void DeleteTransformPushButtonClicked();

signals:
    void TransformationAdded();
    
protected:
    void UpdateUI();
    UserTransformations *m_userTransforms;
    vtkMatrix4x4 *m_matrix;

private:
    bool m_editing;
    bool m_postMultiply;
};

#endif // EDITTRANSFORMDIALOG_H

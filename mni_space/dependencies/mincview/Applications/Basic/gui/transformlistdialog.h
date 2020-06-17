/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: transformlistdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:07 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  Re-make from the old Simon's
  All rights reserved.
  
=========================================================================*/

#ifndef TAG_TRANSFORMLISTDIALOG_H
#define TAG_TRANSFORMLISTDIALOG_H

#include "transformlistdialogbase.h"
#include <qwidget.h>
#include <vector>

class QString;
class UserTransformations;

class TransformListDialog : public TransformListDialogBase
{
    Q_OBJECT

public:
    TransformListDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~TransformListDialog();

    void SetUserTransforms( UserTransformations * tr );
    void SelectTransform(QString transformName);
    void SetAppliedTransforms(std::vector<int> &transformIds);
    
public slots:
    virtual void TransformFromListSelected(const QString&);
    virtual void ShowTransformationMatrix();
    virtual void UndoLastTransformation();

signals:
    void TransformationChanged(QString);
    void UndoTransformation();

protected:
    
    UserTransformations *m_userTransforms;

};

#endif // TAG_TRANSFORMLISTDIALOG_H

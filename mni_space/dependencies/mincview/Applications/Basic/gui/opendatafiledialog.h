/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: opendatafiledialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.10 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef OPENDATAFILEDIALOG_H
#define OPENDATAFILEDIALOG_H

#include "opendatafiledialogbase.h"
#include <qstring.h>
#include "expertmode.h"

class UserTransformations;

class OpenFileOutputParams
{
public:
    OpenFileOutputParams();
    ~OpenFileOutputParams();
    int type;
    QStringList inputFiles;
    QString dataObjectName;
    QString transformName;
    QString lastVisitedDir;
    UserTransformations *userTransforms;
    bool referenceObjectAlreadySet;
};

class OpenDataFileDialog : public OpenDataFileDialogBase
{
    Q_OBJECT

public:
    OpenDataFileDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0, OpenFileOutputParams *params = 0 );
    virtual ~OpenDataFileDialog();
    
    void SetExpertMode(ExpertMode *mode) {m_expertMode = mode;}
    
public slots:
    virtual void BrowsePushButtonClicked();
    virtual void OKButtonClicked();
    virtual void DataTypeSelected(int);
    virtual void TransformComboBoxActivated(const QString &);
    virtual void ReferenceImageTypeCheckBoxToggled(bool);

protected:

    OpenFileOutputParams  *m_fileParams;
    ExpertMode *m_expertMode;
};

#endif // OPENDATAFILEDIALOG_H

/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: maskdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-03-31 14:27:27 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#ifndef MASKDIALOG_H
#define MASKDIALOG_H

#include "maskdialogbase.h"

class USVolumeReconstructionSettings;

class MaskDialog : public MaskDialogBase
{
    Q_OBJECT

public:
    MaskDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~MaskDialog();

    enum MASK_TYPE {DEFAULT_MASK, CUSTOM_MASK, NO_MASK};
//     QLabel* currentSettingsTextLabel;
//     QPushButton* customBrowsePushButton;
//     QLineEdit* maskLineEdit;
//     QPushButton* buttonOk;
//     QPushButton* buttonCancel;
//     QButtonGroup* maskButtonGroup;
//     QRadioButton* noMaskRadioButton;
//     QRadioButton* defaultMaskRadioButton;
//     QRadioButton* customMaskRadioButton;
//     QLabel* probeSettingsTextLabel;
//     QLabel* maskTextLabel;

    void SetMaskParameters(USVolumeReconstructionSettings *params);
    
public slots:
    virtual void OKButtonClicked();
    virtual void MaskTypeChanged(int);
    virtual void CustomMaskBrowseButtonClicked();

protected:
    USVolumeReconstructionSettings *m_params;
    int m_maskType;
};

#endif // MASKDIALOG_H

/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: simplevolumizedialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
// The parameters will be transferred via USAcquisitionOutput class

#ifndef SIMPLEVOLUMIZEDIALOG_H
#define SIMPLEVOLUMIZEDIALOG_H

#include "volumizedialogbase.h"
#include <qstring.h>
#include <qstringlist.h>

class USVolumeReconstructionSettings;

class SimpleVolumizeDialog : public VolumizeDialogBase
{
    Q_OBJECT

public:
    SimpleVolumizeDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~SimpleVolumizeDialog();

    enum MASK_TYPE {DEFAULT_MASK, CUSTOM_MASK, NO_MASK};

    void Init(bool flag, QString dir);
    void SetUSReconstructionParams(USVolumeReconstructionSettings *params);
            
public slots:
    virtual void OKButtonClicked();
    virtual void MaskTypeChanged(int);
    virtual void CustomMaskBrowseButtonClicked();
    virtual void PreviewButtonClicked();
    virtual void InterpolationTypeChanged(int);
    virtual void CustomMaskLineEditChanged();
    virtual void InputFilesLineEditChanged();
    virtual void OutputFileLineEditChanged();
    virtual void InputFilesBrowseButtonClicked();
    virtual void OutputFileBrowseButtonClicked();
    virtual void ReconstructionTypeChanged(int);
    virtual void IncreaseSaturationThresholdPushButtonClicked();
    virtual void DecreaseSaturationThresholdPushButtonClicked();
    virtual void SaturationThresholdSliderValueChanged(int);
    virtual void SaturationThresholdLineEditChanged(const QString &);

protected:
    USVolumeReconstructionSettings *m_output;
    
    int m_interpolationType;
    int m_maskType;
    double m_saturationThreshold;
    bool m_onTheFly;
    QStringList m_inputFiles;
    QString m_outputFile;
    QString m_lastVisitedDir;
};

#endif // SIMPLEVOLUMIZEDIALOG_H


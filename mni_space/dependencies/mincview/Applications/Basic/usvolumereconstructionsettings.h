/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: usvolumereconstructionsettings.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:34 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.
  
=========================================================================*/
#ifndef TAG_USVOLUMERECONSTRUCTIONSETTINGS_H_
#define TAG_USVOLUMERECONSTRUCTIONSETTINGS_H_
#include "serializer.h"
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

enum MASKTYPE {DEFAULT_MASK, CUSTOM_MASK, NO_MASK};
    
class USVolumeReconstructionSettings : public QObject
{
Q_OBJECT

public:
    USVolumeReconstructionSettings();
    virtual ~USVolumeReconstructionSettings();
    void Serialize( Serializer * serializer );
    
    void    SetMaskType(int);
    int     GetMaskType();
    void    SetCustomMaskFileName(QString);
    QString GetCustomMaskFileName();
    void    GetDefaultMaskName(QString &maskFile);
    void    SetRegridRadius(double);
    double  GetRegridRadius();
    void    SetStepSize(double);
    double  GetStepSize();
    void    SetSigma(double);
    double  GetSigma();
    void    SetInterpolationType(int);
    int     GetInterpolationType();
    void    SetReconstructionType(int);
    int     GetReconstructionType();
    void    SetSaturationThreshold(double);
    double  GetSaturationThreshold();
    void    SetOutputFile(QString);
    QString GetOutputFile();
    QString GetUSDepth();
    void    SetUSDepth(QString depth);
    int     GetUSMode();
    void    SetUSMode(int mode);
    void    SetInputFiles(QStringList);
    QStringList GetInputFiles();

protected:
    int m_maskType;
    QString m_customMaskFileName;
    double m_regridRadius;
    double m_stepSize;
    double m_sigma;
    int m_interpolationType;
    QStringList m_inputFiles;
    QString m_outputFile;
    int m_reconstructionType; // 0 - whole, 1 vessels only
    double m_saturationThreshold;
    int m_usModeIsBMode; // grayscale/b-mode = 1, rgb = 0
    QString m_usDepth;
    
    QStringList m_bmodeMaskFiles;
    QStringList m_rgbMaskFiles;
};

ObjectSerializationHeaderMacro( USVolumeReconstructionSettings );


#endif //TAG_USVOLUMERECONSTRUCTIONSETTINGS_H_
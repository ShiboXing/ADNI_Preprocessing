/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: usvolumereconstructionsettings.cpp,v $
  Language:  C++
  Date:      $Date: 2010-03-31 14:16:47 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.
  
=========================================================================*/

#include "usvolumereconstructionsettings.h"
#include "ignsconfig.h"
#include "ignsglobals.h"
#include <qdir.h> 

ObjectSerializationMacro( USVolumeReconstructionSettings );

USVolumeReconstructionSettings::USVolumeReconstructionSettings()
{
    m_maskType = DEFAULT_MASK;
    m_regridRadius = 1.0;
    m_stepSize = 1.0;
    m_sigma = 2 * m_regridRadius;
    m_interpolationType = IGNS_GAUSSIAN_INTERPOLATION;
    m_reconstructionType = 0;
    m_saturationThreshold = IGNS_DEFAULT_SATURATION_THRESHOLD;
    m_usModeIsBMode = true;
    m_bmodeMaskFiles.append(STR_IGNS_MASK_4CM_BMODE);
    m_bmodeMaskFiles.append(STR_IGNS_MASK_5CM_BMODE);
    m_bmodeMaskFiles.append(STR_IGNS_MASK_6CM_BMODE);
    m_bmodeMaskFiles.append(STR_IGNS_MASK_8CM_BMODE);
    m_bmodeMaskFiles.append(STR_IGNS_MASK_9CM_BMODE);
    m_rgbMaskFiles.append(STR_IGNS_MASK_4CM_RGB);
    m_rgbMaskFiles.append(STR_IGNS_MASK_5CM_RGB);
    m_rgbMaskFiles.append(STR_IGNS_MASK_6CM_RGB);
    m_rgbMaskFiles.append(STR_IGNS_MASK_8CM_RGB);
    m_rgbMaskFiles.append(STR_IGNS_MASK_9CM_RGB);
}


USVolumeReconstructionSettings::~USVolumeReconstructionSettings()
{
}

void USVolumeReconstructionSettings::Serialize(Serializer * serializer)
{
    ::Serialize( serializer, "maskType", m_maskType );
    ::Serialize( serializer, "customMaskFileName", m_customMaskFileName );
    ::Serialize( serializer, "regridRadius", m_regridRadius );
    ::Serialize( serializer, "stepSize", m_stepSize );
    ::Serialize( serializer, "sigma", m_sigma );
    ::Serialize( serializer, "interpolationType", m_interpolationType );
    ::Serialize( serializer, "reconstructionType", m_reconstructionType );
    ::Serialize( serializer, "saturationThreshold", m_saturationThreshold );
    ::Serialize( serializer, "usModeIsBMode", m_usModeIsBMode );
    ::Serialize( serializer, "usDepth", m_usDepth );
    ::Serialize( serializer, "outputFile", m_outputFile );
    // the list of input files is not saved, it may be very long...
}

// volume reconstruction
void USVolumeReconstructionSettings::SetMaskType(int useMask)
{
    if (m_maskType != useMask)
    {
        m_maskType = useMask;
    }
}

int USVolumeReconstructionSettings::GetMaskType()
{
    return m_maskType;
}

void USVolumeReconstructionSettings::SetCustomMaskFileName(QString name)
{
    if (m_customMaskFileName != name)
    {
        m_customMaskFileName = name;
   }
}

QString USVolumeReconstructionSettings::GetCustomMaskFileName()
{
    return m_customMaskFileName;
}

void USVolumeReconstructionSettings::SetRegridRadius(double radius)
{
    if (m_regridRadius != radius)
    {
        m_regridRadius = radius;
    }
}

double USVolumeReconstructionSettings::GetRegridRadius()
{
    return m_regridRadius;
}

void USVolumeReconstructionSettings::SetStepSize(double step)
{
    if (m_stepSize != step)
    {
        m_stepSize = step;
    }
}

double USVolumeReconstructionSettings::GetStepSize()
{
    return m_stepSize;
}

void USVolumeReconstructionSettings::SetSigma(double s)
{
    if (m_sigma != s)
    {
        m_sigma = s;
    }
}

double USVolumeReconstructionSettings::GetSigma()
{
    return m_sigma;
}

void USVolumeReconstructionSettings::SetInterpolationType(int type)
{
    if (m_interpolationType != type)
    {
        m_interpolationType = type;
    }
}

int USVolumeReconstructionSettings::GetInterpolationType()
{
    return m_interpolationType;
}

void USVolumeReconstructionSettings::SetReconstructionType(int type)
{
    if (m_reconstructionType != type)
    {
        m_reconstructionType = type;
    }
}

int USVolumeReconstructionSettings::GetReconstructionType()
{
    return m_reconstructionType;
}

void USVolumeReconstructionSettings::SetSaturationThreshold(double threshold)
{
    if (m_saturationThreshold != threshold)
    {
        m_saturationThreshold = threshold;
    }
}

double USVolumeReconstructionSettings::GetSaturationThreshold()
{
    return m_saturationThreshold;
}

void USVolumeReconstructionSettings::SetInputFiles(QStringList list)
{
    if (m_inputFiles != list)
    {
        m_inputFiles = list;
    }
}

QStringList USVolumeReconstructionSettings::GetInputFiles()
{
    return m_inputFiles;
}

void USVolumeReconstructionSettings::SetOutputFile(QString filename)
{
    if (m_outputFile != filename)
    {
        m_outputFile = filename;
    }
}

QString USVolumeReconstructionSettings::GetOutputFile()
{
    return m_outputFile;
}

QString USVolumeReconstructionSettings::GetUSDepth()
{
    return m_usDepth;
}

void USVolumeReconstructionSettings::SetUSDepth(QString depth)
{
    if (m_usDepth != depth)
    {
        m_usDepth = depth;
    }
}

int USVolumeReconstructionSettings::GetUSMode()
{
    return m_usModeIsBMode;
}

void USVolumeReconstructionSettings::SetUSMode(int mode)
{
    if (m_usModeIsBMode != mode)
    {
        m_usModeIsBMode = mode;
    }
}

void USVolumeReconstructionSettings::GetDefaultMaskName(QString &maskFile)
{
    if (!m_usDepth.isEmpty())
    {
        QString t = m_usDepth;
        t.remove(1,3);
        int index, currentDepth = t.toInt();
        if (currentDepth < 5)
            index = 0;
        else 
            index = currentDepth-5;
        QString maskDir(QDir::homeDirPath());
        if (m_usModeIsBMode > 0)
        {
            maskFile = maskDir + IGNS_BMODE_MASK_SUBDIRECTORY + m_bmodeMaskFiles[index];
        }
        else
        {
            maskFile = maskDir + IGNS_DOPPLER_MASK_SUBDIRECTORY + m_rgbMaskFiles[index];
        }
    }
}

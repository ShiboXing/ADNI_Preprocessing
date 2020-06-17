/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: attrib2usacquisitioninfo.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:33 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "attrib2usacquisitioninfo.h"

#include "vtkMINCImageAttributes.h"
#include "vtkMINC.h"
#include "vtkMINC2.h"


Attrib2USAcquisitionInfo::Attrib2USAcquisitionInfo()
{
}

Attrib2USAcquisitionInfo::~Attrib2USAcquisitionInfo()
{
}

void Attrib2USAcquisitionInfo::FillUSAcquisitionInfo( vtkMINCImageAttributes * attributes )
{
    if (!(attributes && m_acquisitionInfo))
        return;
    // patient
    m_acquisitionInfo->SetPatientName(attributes->GetAttributeValueAsString(MIpatient, MIfull_name));
    m_acquisitionInfo->SetPatientIdentification(attributes->GetAttributeValueAsString(MIpatient, MIidentification));
    // study
    m_acquisitionInfo->SetStudentName(attributes->GetAttributeValueAsString(MIstudy, MIoperator));
    m_acquisitionInfo->SetSurgeonName(attributes->GetAttributeValueAsString(MIstudy, MIattending_physician));
    m_acquisitionInfo->SetUltrasoundModel(attributes->GetAttributeValueAsString(MIstudy, MIdevice_model));
    m_acquisitionInfo->SetTrackerModel(attributes->GetAttributeValueAsString(MIstudy, MItracker_model));
    m_acquisitionInfo->SetTrackerVersion(attributes->GetAttributeValueAsString(MIstudy, MItracker_version));
    m_acquisitionInfo->SetApplication(attributes->GetAttributeValueAsString(MIstudy, MIapplication));
    m_acquisitionInfo->SetApplicationVersion(attributes->GetAttributeValueAsString(MIstudy, MIapplication_version));
    m_acquisitionInfo->SetProbeModel(attributes->GetAttributeValueAsString(MIstudy, MItransducer_model));
    m_acquisitionInfo->SetProbeRomFile(attributes->GetAttributeValueAsString(MIstudy, MItransducer_rom));
    m_acquisitionInfo->SetProbeCalibrationFile(attributes->GetAttributeValueAsString(MIstudy, MItransducer_calibration));
    m_acquisitionInfo->SetPointerModel(attributes->GetAttributeValueAsString(MIstudy, MIpointer_model));
    m_acquisitionInfo->SetPointerRomFile(attributes->GetAttributeValueAsString(MIstudy, MIpointer_rom));
    m_acquisitionInfo->SetPointerCalibrationFile(attributes->GetAttributeValueAsString(MIstudy, MIpointer_calibration));
    m_acquisitionInfo->SetReferenceModel(attributes->GetAttributeValueAsString(MIstudy, MIreference_model));
    m_acquisitionInfo->SetReferenceRomFile(attributes->GetAttributeValueAsString(MIstudy, MIreference_rom));
    m_acquisitionInfo->SetCalibrationMethod(attributes->GetAttributeValueAsString(MIstudy, MIcalibration_method));
    m_acquisitionInfo->SetCalibrationVersion(attributes->GetAttributeValueAsString(MIstudy, MIcalibration_version));
    m_acquisitionInfo->SetCalibrationDate(attributes->GetAttributeValueAsString(MIstudy, MIcalibration_date));
    // acquisition
    m_acquisitionInfo->SetUsDepth(attributes->GetAttributeValueAsString(MIacquisition, MIultrasound_depth));
    m_acquisitionInfo->SetUsScale(attributes->GetAttributeValueAsString(MIacquisition, MIultrasound_scale));
    m_acquisitionInfo->SetAcquisitionType(attributes->GetAttributeValueAsString(MIacquisition, MIultrasound_acquisition_type));
    m_acquisitionInfo->SetAcquisitionColor(attributes->GetAttributeValueAsString(MIacquisition, MIultrasound_acquisition_color));
    m_acquisitionInfo->SetComment(attributes->GetAttributeValueAsString("", MIcomments));
}

void Attrib2USAcquisitionInfo::SetUSAcquisitionInfo(USAcquisitionInfo * info )
{
    if (info)
    {
        m_acquisitionInfo = info;
    }
}

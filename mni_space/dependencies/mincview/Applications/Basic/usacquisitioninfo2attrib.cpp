/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: usacquisitioninfo2attrib.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-22 14:55:28 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "usacquisitioninfo2attrib.h"

#include "vtkMINCImageAttributes.h"
#include "vtkMINC.h"
#include "vtkMINC2.h"


USAcquisitionInfo2Attrib::USAcquisitionInfo2Attrib() 
{
}

USAcquisitionInfo2Attrib::~USAcquisitionInfo2Attrib()
{
}

void USAcquisitionInfo2Attrib::FillImageAttributes( vtkMINCImageAttributes * attributes )
{
    if (!(attributes && m_acquisitionInfo))
        return;
    // patient
    if (!m_acquisitionInfo->GetPatientName().isEmpty())
        attributes->SetAttributeValueAsString(MIpatient, MIfull_name, m_acquisitionInfo->GetPatientName());
    if (!m_acquisitionInfo->GetPatientIdentification().isEmpty())
        attributes->SetAttributeValueAsString(MIpatient, MIidentification, m_acquisitionInfo->GetPatientIdentification());
    // study
    if (!m_acquisitionInfo->GetStudentName().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIoperator, m_acquisitionInfo->GetStudentName());
    if (!m_acquisitionInfo->GetSurgeonName().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIattending_physician, m_acquisitionInfo->GetSurgeonName());
    attributes->SetAttributeValueAsString(MIstudy, MImodality, "Ultrasound");
    attributes->SetAttributeValueAsString(MIstudy, MImanufacturer, "ATL");
    if (!m_acquisitionInfo->GetUltrasoundModel().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIdevice_model, m_acquisitionInfo->GetUltrasoundModel());
    if (!m_acquisitionInfo->GetTrackerModel().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MItracker_model, m_acquisitionInfo->GetTrackerModel());
    if (!m_acquisitionInfo->GetTrackerVersion().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MItracker_version, m_acquisitionInfo->GetTrackerVersion());
    if (!m_acquisitionInfo->GetApplication().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIapplication, m_acquisitionInfo->GetApplication());
    if (!m_acquisitionInfo->GetApplicationVersion().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIapplication_version, m_acquisitionInfo->GetApplicationVersion());
    if (!m_acquisitionInfo->GetProbeModel().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MItransducer_model, m_acquisitionInfo->GetProbeModel());
    if (!m_acquisitionInfo->GetProbeRomFile().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MItransducer_rom, m_acquisitionInfo->GetProbeRomFile());
    if (!m_acquisitionInfo->GetProbeCalibrationFile().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MItransducer_calibration, m_acquisitionInfo->GetProbeCalibrationFile());
    if (!m_acquisitionInfo->GetPointerModel().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIpointer_model, m_acquisitionInfo->GetPointerModel());
    if (!m_acquisitionInfo->GetPointerRomFile().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIpointer_rom, m_acquisitionInfo->GetPointerRomFile());
    if (!m_acquisitionInfo->GetPointerCalibrationFile().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIpointer_calibration, m_acquisitionInfo->GetPointerCalibrationFile());
    if (!m_acquisitionInfo->GetReferenceModel().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIreference_model, m_acquisitionInfo->GetReferenceModel());
    if (!m_acquisitionInfo->GetReferenceRomFile().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIreference_rom, m_acquisitionInfo->GetReferenceRomFile());
    if (!m_acquisitionInfo->GetCalibrationMethod().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIcalibration_method, m_acquisitionInfo->GetCalibrationMethod());
    if (!m_acquisitionInfo->GetCalibrationVersion().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIcalibration_version, m_acquisitionInfo->GetCalibrationVersion());
    if (!m_acquisitionInfo->GetCalibrationDate().isEmpty())
        attributes->SetAttributeValueAsString(MIstudy, MIcalibration_date, m_acquisitionInfo->GetCalibrationDate());
    // acquisition
    if (!m_acquisitionInfo->GetUsDepth().isEmpty())
        attributes->SetAttributeValueAsString(MIacquisition, MIultrasound_depth, m_acquisitionInfo->GetUsDepth() );
    if (!m_acquisitionInfo->GetUsScale().isEmpty())
        attributes->SetAttributeValueAsString(MIacquisition, MIultrasound_scale, m_acquisitionInfo->GetUsScale() );
    if (!m_acquisitionInfo->GetAcquisitionType().isEmpty())
        attributes->SetAttributeValueAsString(MIacquisition, MIultrasound_acquisition_type, m_acquisitionInfo->GetAcquisitionType() );
    if (!m_acquisitionInfo->GetAcquisitionColor().isEmpty())
        attributes->SetAttributeValueAsString(MIacquisition, MIultrasound_acquisition_color, m_acquisitionInfo->GetAcquisitionColor() );
    if (!m_acquisitionInfo->GetComment().isEmpty())
        attributes->SetAttributeValueAsString("", MIcomments, m_acquisitionInfo->GetComment() );
}

void USAcquisitionInfo2Attrib::SetUSAcquisitionInfo(USAcquisitionInfo * info )
{
    if (info)
    {
        m_acquisitionInfo = info;
    }
}


/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: usacquisitioninfo.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:34 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "usacquisitioninfo.h"

#include <stdio.h>
#include <time.h>
#include <qdir.h>
#include <qfiledialog.h>
#include "ignsconfig.h"
#include "version.h"
#include "vtkMINCImageAttributes.h"
#include "vtkMINC.h"

ObjectSerializationMacro( USAcquisitionInfo );

USAcquisitionInfo::USAcquisitionInfo() 
{
    char tmp[100];
    time_t result;
    result = time(NULL);
    this->acquisitionDate = asctime(localtime(&result));
    this->studentName = cuserid(tmp);
    this->ultrasoundModel = IGNS_ULTRASOUND_MODEL;
    this->trackerModel = IGNS_TRACKER_MODEL;
    this->application = "ibis";
    this->applicationVersion = IGNS_VERSION;
    this->lastVisitedDirectory = (const char*)(QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY);
}

USAcquisitionInfo::~USAcquisitionInfo() 
{
}

void USAcquisitionInfo::Serialize( Serializer * serializer )
{
    ::Serialize( serializer, "PatientIdentification", patientIdentification );
    ::Serialize( serializer, "PatientName", patientName );
    ::Serialize( serializer, "StudentName", studentName );
    ::Serialize( serializer, "SurgeonName", surgeonName );
    ::Serialize( serializer, "AcquisitionDate", acquisitionDate );
    ::Serialize( serializer, "Comment", comment );
    ::Serialize( serializer, "UltrasoundModel", ultrasoundModel );
    ::Serialize( serializer, "TrackerModel", trackerModel );
    ::Serialize( serializer, "TrackerVersion", trackerVersion );
    ::Serialize( serializer, "PointerModel", pointerModel );
    ::Serialize( serializer, "PointerCalibrationFile", pointerCalibrationFile );
    ::Serialize( serializer, "PointerRomFile", pointerRomFile );
    ::Serialize( serializer, "ReferenceModel", referenceModel );
    ::Serialize( serializer, "ReferenceRomFile", referenceRomFile );
    ::Serialize( serializer, "ProbeModel", probeModel );
    ::Serialize( serializer, "ProbeCalibrationFile", probeCalibrationFile );
    ::Serialize( serializer, "ProbeRomFile", probeRomFile );
    ::Serialize( serializer, "usDepth", usDepth );
    ::Serialize( serializer, "usScale", usScale );
    ::Serialize( serializer, "application", application );
    ::Serialize( serializer, "applicationVersion", applicationVersion );
    ::Serialize( serializer, "calibrationDate", calibrationDate );
    ::Serialize( serializer, "calibrationMethod", calibrationMethod );
    ::Serialize( serializer, "calibrationVersion", calibrationVersion );
    ::Serialize( serializer, "acquisitionType", acquisitionType );
    ::Serialize( serializer, "acquisitionColor", acquisitionColor );
    ::Serialize( serializer, "lastVisitedDirectory", lastVisitedDirectory );
}

void USAcquisitionInfo::PrepareForMINCOutput( )
{
    studyAttributes.clear();
    acquisitionAttributes.clear();
    patientAttributes.clear();
    // patient
    if (!patientName.isEmpty())
        patientAttributes[MIfull_name] = patientName;
    if (!patientIdentification.isEmpty())
        patientAttributes[MIidentification] = patientIdentification;
    // study
    if (!studentName.isEmpty())
        studyAttributes[MIoperator] = studentName;
    if (!surgeonName.isEmpty())
        studyAttributes[MIattending_physician] = surgeonName;
    studyAttributes[MImodality] = "Ultrasound";
    studyAttributes[MImanufacturer] = "ATL";
    if (!ultrasoundModel.isEmpty())
        studyAttributes[MIdevice_model] = ultrasoundModel;
    if (!trackerModel.isEmpty())
        studyAttributes[MItracker_model] = trackerModel;
    if (!trackerVersion.isEmpty())
        studyAttributes[MItracker_version] = trackerVersion;
    if (!application.isEmpty())
        studyAttributes[MIapplication] = application;
    if (!applicationVersion.isEmpty())
        studyAttributes[MIapplication_version] = applicationVersion;
    if (!probeModel.isEmpty())
        studyAttributes[MItransducer_model] = probeModel;
    if (!probeRomFile.isEmpty())
        studyAttributes[MItransducer_rom] = probeRomFile;
    if (!probeCalibrationFile.isEmpty())
        studyAttributes[MItransducer_calibration] = probeCalibrationFile;
    if (!pointerModel.isEmpty())
        studyAttributes[MIpointer_model] = pointerModel;
    if (!pointerRomFile.isEmpty())
        studyAttributes[MIpointer_rom] = pointerRomFile;
    if (!pointerCalibrationFile.isEmpty())
        studyAttributes[MIpointer_calibration] = pointerCalibrationFile;
    if (!referenceModel.isEmpty())
        studyAttributes[MIreference_model] = referenceModel;
    if (!referenceRomFile.isEmpty())
        studyAttributes[MIreference_rom] = referenceRomFile;
    if (!calibrationMethod.isEmpty())
        studyAttributes[MIcalibration_method] = calibrationMethod;
    if (!calibrationVersion.isEmpty())
        studyAttributes[MIcalibration_version] = calibrationVersion;
    if (!calibrationDate.isEmpty())
        studyAttributes[MIcalibration_date] = calibrationDate;
   // acquisition
    if (!usDepth.isEmpty())
        acquisitionAttributes[MIultrasound_depth] = usDepth;
    if (!usScale.isEmpty())
        acquisitionAttributes[MIultrasound_scale] = usScale;
    if (!acquisitionType.isEmpty())
        acquisitionAttributes[MIultrasound_acquisition_type] = acquisitionType;
    if (!acquisitionColor.isEmpty())
        acquisitionAttributes[MIultrasound_acquisition_color] = acquisitionColor;
/*    if (!comment.isEmpty())
        attributes->SetAttributeValueAsString("", MIcomments, comment );*/
}


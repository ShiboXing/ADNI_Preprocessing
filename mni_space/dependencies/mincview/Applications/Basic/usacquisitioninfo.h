/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: usacquisitioninfo.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:34 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef USACQUISITIONINFO_H
#define USACQUISITIONINFO_H

#include "serializer.h"
#include <qobject.h>
#include <qstring.h>
#include "vtkMINC2.h"

enum ACQ_TYPE {ACQ_B_MODE, ACQ_DOPPLER, ACQ_POWER_DOPPLER};
enum ACQ_COLOR {ACQ_UNDEFINED, ACQ_GRAYSCALE, ACQ_RGB};
    
#define ACQ_TYPE_B_MODE "B-mode"
#define ACQ_TYPE_DOPPLER "Doppler"
#define ACQ_TYPE_POWER_DOPPLER "Power Doppler"
#define ACQ_COLOR_RGB "RGB"
#define ACQ_COLOR_GRAYSCALE "Grayscale"

//class vtkMINCImageAttributes;

class USAcquisitionInfo : public QObject 
{
    Q_OBJECT

public:
    USAcquisitionInfo();
    ~USAcquisitionInfo();

    void SetPatientIdentification(const QString id) {patientIdentification = id;}
    const QString GetPatientIdentification() {return patientIdentification;}
    void SetPatientName(const QString name) {patientName = name;}
    const QString GetPatientName() {return patientName;}
    void SetComment(const QString c) {comment = c;}
    const QString GetComment() {return comment;}
    void SetStudentName(const QString name) {studentName = name;}
    const QString GetStudentName() {return studentName;}
    void SetSurgeonName(const QString name) {surgeonName = name;}
    const QString GetSurgeonName() {return surgeonName;}
    void SetUltrasoundModel(const QString model) {ultrasoundModel = model;}
    const QString GetUltrasoundModel() {return ultrasoundModel;}
    void SetTrackerModel(const QString model) {trackerModel = model;}
    const QString GetTrackerModel() {return trackerModel;}
    void SetTrackerVersion(const QString version) {trackerVersion = version;}
    const QString GetTrackerVersion() {return trackerVersion;}
    void SetPointerModel(const QString model) {pointerModel = model;}
    const QString GetPointerModel() {return pointerModel;}
    void SetPointerCalibrationFile(const QString filename) {pointerCalibrationFile = filename;}
    const QString GetPointerCalibrationFile() {return pointerCalibrationFile;}
    void SetPointerRomFile(const QString filename) {pointerRomFile = filename;}
    const QString GetPointerRomFile() {return pointerRomFile;}
    void SetReferenceModel(const QString model) {referenceModel = model;}
    const QString GetReferenceModel() {return referenceModel;}
    void SetReferenceRomFile(const QString filename) {referenceRomFile = filename;}
    const QString GetReferenceRomFile() {return referenceRomFile;}
    void SetProbeModel(const QString model) {probeModel = model;}
    const QString GetProbeModel() {return probeModel;}
    void SetProbeCalibrationFile(const QString filename) {probeCalibrationFile = filename;}
    const QString GetProbeCalibrationFile() {return probeCalibrationFile;}
    void SetProbeRomFile(const QString filename) {probeRomFile = filename;}
    const QString GetProbeRomFile() {return probeRomFile;}
    void SetUsDepth(const QString depth) {usDepth = depth;}
    const QString GetUsDepth() {return usDepth;}
    void SetUsScale(const QString scale) {usScale = scale;}
    const QString GetUsScale() {return usScale;}
    void SetApplication(const QString app) {application = app;}
    const QString GetApplication() {return application;}
    void SetApplicationVersion(const QString version) {applicationVersion = version;}
    const QString GetApplicationVersion() {return applicationVersion;}
    void SetCalibrationDate(const QString date) {calibrationDate = date;}
    const QString GetCalibrationDate() {return calibrationDate;}
    void SetCalibrationMethod(const QString method) {calibrationMethod = method;}
    const QString GetCalibrationMethod() {return calibrationMethod;}
    void SetCalibrationVersion(const QString version) {calibrationVersion = version;}
    const QString GetCalibrationVersion() {return calibrationVersion;}
    void SetAcquisitionType(const QString type) {acquisitionType = type;}
    const QString GetAcquisitionType() {return acquisitionType;}
    void SetAcquisitionColor(const QString color) {acquisitionColor = color;}
    const QString GetAcquisitionColor() {return acquisitionColor;}
    
    void SetLastVisitedDirectory(const QString dir) {lastVisitedDirectory = dir;}
    const QString GetLastVisitedDirectory() {return lastVisitedDirectory;}

    void Serialize( Serializer * serializer );
    void PrepareForMINCOutput( );
    STUDY_ATTRIBUTES *GetStudyAttributes() {return &studyAttributes;};
    ACQUISITION_ATTRIBUTES *GetAcquisitionAttributes() {return &acquisitionAttributes;}
    PATIENT_ATTRIBUTES *GetPatientAttributes() {return &patientAttributes;}

protected:
    QString patientIdentification;
    QString patientName;
    QString studentName;
    QString surgeonName;
    QString acquisitionDate;
    QString comment;
    QString ultrasoundModel;
    QString trackerModel;
    QString trackerVersion;
    QString pointerModel;
    QString pointerCalibrationFile;
    QString pointerRomFile;
    QString referenceModel;
    QString referenceRomFile;
    QString probeModel;
    QString probeCalibrationFile;
    QString probeRomFile;
    QString usDepth;
    QString usScale;
    QString application;
    QString applicationVersion;
    QString calibrationDate;
    QString calibrationMethod;
    QString calibrationVersion;
    QString acquisitionType;
    QString acquisitionColor;
    
    QString lastVisitedDirectory;

/*
This is redundant, but as for now 2009/09/03 the fastest way to transfer
data to our vtkMINCWriter, until I find out how to use vtkMINCImageWriter
the one from vtk5.4 is messing up image orientation, spacing, etc.
I should not mix Qt with vtk in the vtkMINCWriter, so I have to change
QStrings to char pointers.
*/
 
    STUDY_ATTRIBUTES studyAttributes;
    ACQUISITION_ATTRIBUTES acquisitionAttributes;
    PATIENT_ATTRIBUTES patientAttributes;
};

ObjectSerializationHeaderMacro( USAcquisitionInfo );

#endif // USACQUISITIONINFO_H

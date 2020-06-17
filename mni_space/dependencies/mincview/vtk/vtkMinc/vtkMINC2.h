/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: vtkMINC2.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:23:21 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

/* 
   This file contains additional variables not existing in "minc.h" that was
   distributed with minc version 1.4 and 2.0
   The choice of the name was unfortunate as it suggests usage with minc 2 while the number 2 was used to distinguish from vtkMINC.h used in vtk 5.4.
*/
#ifndef vtkMINC2_h
#define vtkMINC2_h

#include <map>

/* The study  attributes  used in ibis */
#define MIapplication               "application"
#define MIapplication_version       "application_version"
#define MItracker                   "tracker"
#define MItracker_model             "tracker_model"
#define MItracker_version           "tracker_version"
#define MItransducer_model          "transducer_model"
#define MItransducer_rom            "transducer_rom"
#define MItransducer_calibration    "transducer_calibration"
#define MIpointer_model             "pointer_model"
#define MIpointer_rom               "pointer_rom"
#define MIpointer_calibration       "pointer_calibration"
#define MIreference_model           "reference_model"
#define MIreference_rom             "reference_rom"
#define MIcalibration_method        "calibration_method"
#define MIcalibration_version       "calibration_version"
#define MIcalibration_date          "calibration_date"

/* The acquisition and its attributes */
#define MIultrasound_depth              "ultrasound_depth"
#define MIultrasound_scale              "ultrasound_scale"
#define MIultrasound_acquisition_type   "ultrasound_acquisition_type"
#define MIultrasound_acquisition_color  "ultrasound_acquisition_color"

// The following map is used to assign study attributes to their values
typedef std::map<const char*,const char*> STUDY_ATTRIBUTES;

// The following map is used to assign acquisition attributes to their values
typedef std::map<const char*,const char*> ACQUISITION_ATTRIBUTES;

// The following map is used to assign patient attributes that can be defined in ibis to their values
typedef std::map<const char*,const char*> PATIENT_ATTRIBUTES;

/* End ifndef vtkMINC2_h */
#endif

/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: ignsconfig.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:40:37 $
  Version:   $Revision: 1.11 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.
  
=========================================================================*/
 
#ifndef TAG_CONFIGURATION_H
#define TAG_CONFIGURATION_H

#define IGNS_CONFIGURATION_SUBDIRECTORY  "Igns"
#define IGNS_BMODE_MASK_SUBDIRECTORY     "/Igns/masks/b-mode/"
#define IGNS_DOPPLER_MASK_SUBDIRECTORY   "/Igns/masks/doppler/"
#define IGNS_INSTALLATION_SUBDIRECTORY   "/usr/local/Igns"
#define IGNS_IBIS_ICON                   "ibis.png"
#define IGNS_MINC_VIEW_ICON              "mincview.png"
#define IGNS_TAG_CONFIG_FILE_NAME        "tagfiledir.xml"
#define IGNS_TRANSFER_FUNCTION_SETTINGS  "transfer_function.xml"
#define IGNS_MINC_VIEW_CONFIG            "MincView_config.xml"
#define IGNS_IBIS_CONFIG                 "ibis_config.xml"
#define IGNS_IBIS_CONFIG_BACKUP          "ibis_config_back.xml"
#define IGNS_TRACKER_CONFIG              "tracker_config.xml"
#define IGNS_VIDEO_CONFIG                "video_source_config.xml"
#define IGNS_ULTRASOUND_CONFIG           "ultrasound_probe_config.xml"
#define IGNS_IBIS_WINDOW_SETTINGS        "ibis_window_settings.cfg"
#define IGNS_ACQ_CONFIG_FILE_NAME        "acquisitioninfo.xml"

//Acquisition
#define IGNS_ACQUISITION_BASE_DIR        "acquisitions"
#define IGNS_ACQUISITION_PREFIX          "acq"

//Devices
#define IGNS_ULTRASOUND_MODEL            "ATL Ultrasound Model HDI 5000"
#define IGNS_TRACKER_MODEL               "NDI Polaris"
#define IGNS_POINTER_CALIBRATION_FILE    "pointer_calibration.xfm"
#define IGNS_US_PROBE_CALIBRATION_FILE   "us_probe_calibration.xfm"

#endif //TAG_CONFIGURATION_H
/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: ignsmsg.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:40:37 $
  Version:   $Revision: 1.16 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.
  
=========================================================================*/
#ifndef TAG_IGNSMSG_H
#define TAG_IGNSMSG_H

//miscellaneous, mostly used in tracker settings
#define IGNS_MSG_CFG_FILES               "Configuration file (*.xml)"
#define IGNS_MSG_ERROR                   "Error"
#define IGNS_MSG_INFO                    "Info"
#define IGNS_MSG_LOAD_DEFAULT_CFG        "Loading default hardware configuration."
#define IGNS_MSG_LOAD_USER_CFG           "Loading user hardware configuration."
#define IGNS_MSG_NO_CONFIG_FILE          "Settings file not found. Select User Settings."
#define IGNS_MSG_MAKE_CONFIG             "Settings file not provided. Configure the application manually."
#define IGNS_MSG_OPEN                    "open"
#define IGNS_MSG_OPEN_CFG_FILE           "Open cofiguration file"
#define IGNS_MSG_RESTORE_DEFAULT         ". Restoring default settings."
#define IGNS_MSG_DIR                     "Directory Selection"

// Acquisition settings
#define IGNS_MSG_DIR_CHOOSE              "Choose existing directory."
#define IGNS_MSG_NO_DIR                  "Directory does not exist: "
#define IGNS_MSG_NO_ACQ_SETTINGS         "Video cannot be saved." 
#define IGNS_MSG_SET_PARAMS              "Please, set acquisition parameters before saving."
#define IGNS_MSG_ACQ_EXIST               "Please select different directory.\nAcquisition data already saved in: "
#define IGNS_MSG_ACQ_ALREADY_SAVED       "The last acquisition is already saved in:\n"
#define IGNS_MSG_ACQ_SAVED1              "Saved frames:\n"
#define IGNS_MSG_ACQ_SAVED2              " in:\n"
#define IGNS_MSG_ACQ_SAVED3              "Saved current frame in:\n"
#define IGNS_MSG_VOLUME_SAVED            "Saved US volume as:\n"

// Minc reader
#define IGNS_MSG_MINC_CORRUPTED          "The MINC file may contain unsupported features and cannot be read."
#define IGNS_MSG_MINC_NEGATIVE_STEP      "At least one of the step size is negative in the image.\nThis is not well supported in IBIS."

//user permissions
#define IGNS_MSG_FILE_NOT_EXISTS         "File does not exist:\n"
#define IGNS_MSG_NO_WRITE                "No write permission:\n"
#define IGNS_MSG_NO_READ                 "No read permission:\n"
#define IGNS_MSG_NO_EXECUTE              "No execute permission:\n"
#define IGNS_MSG_CANT_OPEN               "Can't open file:\n"
#define IGNS_MSG_CANT_MAKE_DIR           "Can't create directory:\n"
#define IGNS_MSG_NOTHING_TO_SAVE         "There is nothing to save."
#define IGNS_MSG_CANT_REMOVE             "Can't remove file:\n"

//limits
#define IGNS_MSG_POSITIVE                 "This must be a positive number."
#define IGNS_MSG_POSITIVE_LIMITED         "Setting default. This must be a positive not bigger than "
#define IGNS_MSG_POSITIVE_OR_ZERO         "This value may not be negative "

//transformation management
#define IGNS_TRANSFORM_EXISTS             "Name already in use: "

#endif //TAG_IGNSMSG_H

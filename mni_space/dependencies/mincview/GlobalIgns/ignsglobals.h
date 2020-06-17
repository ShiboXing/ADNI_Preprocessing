/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: ignsglobals.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:40:37 $
  Version:   $Revision: 1.10 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.
  
=========================================================================*/
#ifndef TAG_IGNSGLOBALS_H
#define TAG_IGNSGLOBALS_H

#define IGNS_DEFAULT_REGRID_RADIUS          1.0
#define IGNS_MAX_REGRID_RADIUS             10.0
#define IGNS_DEFAULT_STEP_SIZE              1.0
#define IGNS_MAX_STEP_SIZE                 10.0
#define IGNS_DEFAULT_SATURATION_THRESHOLD   0.0

// interpolation types

enum IGNS_INTERPOLATION_TYPES {
    IGNS_GAUSSIAN_INTERPOLATION,
    IGNS_KAISERBESSEL_INTERPOLATION,
    IGNS_LINEAR_INTERPOLATION
};

#define STR_IGNS_GAUSSIAN_INTERPOLATION "Gaussian"
#define STR_IGNS_KAISERBESSEL_INTERPOLATION "Kaiser-Bessel"
#define STR_IGNS_LINEAR_INTERPOLATION "Linear"

// default masks
#define IGNS_NUMBER_OF_MASKS 5

#define STR_IGNS_MASK_4CM_BMODE "mask_4cm_bmode_resampled.mnc"
#define STR_IGNS_MASK_5CM_BMODE "mask_5cm_bmode_resampled.mnc"
#define STR_IGNS_MASK_6CM_BMODE "mask_6cm_bmode_resampled.mnc"
#define STR_IGNS_MASK_8CM_BMODE "mask_8cm_bmode_resampled.mnc"
#define STR_IGNS_MASK_9CM_BMODE "mask_9cm_bmode_resampled.mnc"
#define STR_IGNS_MASK_4CM_RGB "mask_4cm_rgb.mnc"
#define STR_IGNS_MASK_5CM_RGB "mask_5cm_rgb.mnc"
#define STR_IGNS_MASK_6CM_RGB "mask_6cm_rgb.mnc"
#define STR_IGNS_MASK_8CM_RGB "mask_8cm_rgb.mnc"
#define STR_IGNS_MASK_9CM_RGB "mask_9cm_rgb.mnc"

#endif //TAG_IGNSGLOBALS_H

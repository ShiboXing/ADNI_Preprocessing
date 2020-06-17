/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: volumereconstruction.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:35 $
  Version:   $Revision: 1.2 $

  Copyright (c) 2008-2008 IPL, BIC, MNI, McGill, Sean J S Chen 
  All rights reserved.
  Extracted from mainwindow.cpp and modified by Anka
=========================================================================*/
// .NAME VolumeReconstruction - reconstructing volume from slices
// .SECTION Description
// This class takse as input a list of MINC files from a US acquisition and
// reconstructs the volume using parameters passed from volumixe dialog
#ifndef TAG_VOLUMERECONSTRUCTION_H_
#define TAG_VOLUMERECONSTRUCTION_H_

#include <qstring.h>
#include "volumecreator.h"

class RegridVolume;
class USVolumeReconstructionSettings;
class SceneManager;
class USAcquisitionInfo;

class VolumeReconstruction : public VolumeCreator
{
public:
    static VolumeReconstruction * New() { return new VolumeReconstruction; }

    VolumeReconstruction();
    virtual ~VolumeReconstruction();
    
    // Description:
    // Import frames from US acquisition and build a volume
    virtual void VolumeReconstructionFromImportedVideoFrames(USAcquisitionInfo *info = 0);

};

#endif //TAG_VOLUMERECONSTRUCTION_H_

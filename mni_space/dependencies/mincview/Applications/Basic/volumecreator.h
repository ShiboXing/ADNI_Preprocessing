/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: volumecreator.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:34 $
  Version:   $Revision: 1.2 $

  Copyright (c) 2008-2008 IPL, BIC, MNI, McGill, Sean J S Chen 
  All rights reserved.
  Extracted from mainwindow.cpp and modified by Anka
=========================================================================*/
// .NAME VolumeCreator - reconstructing volume from slices
// .SECTION Description
// This class takse as input a list of MINC files from a US acquisition and
// reconstructs the volume using parameters passed from volumixe dialog
#ifndef TAG_VOLUMECREATOR_H_
#define TAG_VOLUMECREATOR_H_

#include "vtkObject.h"
#include <qstring.h>

class RegridVolume;
class USVolumeReconstructionSettings;
class SceneManager;
class USAcquisitionInfo;

class VolumeCreator : public vtkObject
{
public:
    static VolumeCreator * New() { return new VolumeCreator; }

    VolumeCreator();
    virtual ~VolumeCreator();
    
    // Description:
    // build the reconstructed volume
    virtual void CreateVolumeObject(const RegridVolume* const p_pTheRegridder,
                                    QString p_theName, bool p_doExportFile,
                                    USAcquisitionInfo *info = 0);
    
    // Description:
    // set scene manager
    void SetSceneManager(SceneManager *manager);
    // Description:
    // set the last used directory
    void SetLastVisitedDirectory(const QString & dir);
    // Description:
    // Set acquisition parameters for volume reconstruction
    void SetUSReconstructionParams(USVolumeReconstructionSettings *params)    {m_reconstructionParams = params;}

protected:

    QString             m_lastVisitedDirectory;  
    SceneManager        * m_sceneManager;
    USVolumeReconstructionSettings *m_reconstructionParams;
    
};

#endif //TAG_VOLUMECREATOR_H_

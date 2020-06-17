/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: toolsmenufunctions.h,v $
  Language:  C++
  Date:      $Date: 2010-04-08 14:56:03 $
  Version:   $Revision: 1.7 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
// .NAME ToolsMenuFunctions - all the functions serving the tools menu in MainWindow
// .SECTION Description
// This file is created in order to remove the bulk of the code from MainWindow
// in ibis and MincView
#ifndef TAG_TOOLSMENUFUNCTIONS_H_
#define TAG_TOOLSMENUFUNCTIONS_H_

#include <qobject.h>
#include <qstring.h>
#include "scenemanager.h"

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
// IGSTK headers
#include "igstkObject.h"
#include "igstkMacros.h"
#endif
class RegridVolume;
class USVolumeReconstructionSettings;

class ToolsMenuFunctions : public QObject
{

    Q_OBJECT

public:

    ToolsMenuFunctions();
    virtual ~ToolsMenuFunctions();
    
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()
#endif    
    // Description:
    // Enabling picking
    virtual void EnablePicker(QWidget *parent);
    // Description:
    // Open dialog to enter reconstruction parameters
    virtual void OpenVolumeReconstructionDialog(QWidget *parent);
    
    // Description:
    // Open dialog to view a serie of acquired US images, or any serie of 2D minc files
    virtual void OpenShowMultipleFilesDialog(QWidget *parent);
    
    // Description:
    // Change view mode
    void SetManager(SceneManager *manager);
    // Description:
    // Set starting data directory
    void SetLastVisitedDirectory(const QString & dir);
    // Description:
    // Set acquisition parameters for volume reconstruction
    void SetUSReconstructionParams(USVolumeReconstructionSettings *params)    {m_reconstructionParams = params;}
    
protected:

    QString             m_lastVisitedDirectory;  
    SceneManager        * m_sceneManager;
    USVolumeReconstructionSettings *m_reconstructionParams;
};

#endif //TAG_TOOLSMENUFUNCTIONS_H_

/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: importmenufunctions.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
// .NAME ImportMenuFunctions - all the functions serving the import menu in MainWindow
// .SECTION Description
// This file is created in order to remove the bulk of the code from MainWindow
// in ibis and MincView
#ifndef TAG_IMPORTMENUFUNCTIONS_H_
#define TAG_IMPORTMENUFUNCTIONS_H_

#include <qobject.h>
#include "scenemanager.h"

enum IMPORTERROR {IMP_NO_ERR, IMP_NO_OBJECT, IMP_ONLY_ONE};
class ImportMenuFunctions : public QObject
{

    Q_OBJECT

public:

    ImportMenuFunctions();
    virtual ~ImportMenuFunctions();
    
    // Description:
    // Import data in obj format
    IMPORTERROR ImportObj();
    // Description:
    // Import data in MINC format
    IMPORTERROR ImportMinc();
    // Description:
    // Remove imported data
    void RemoveImportedObjOrMinc();
    
    // Description:
    // Set scene manager
    void SetManager(SceneManager *manager);
    // Description:
    // Set starting data directory
    void SetLastVisitedDirectory(const QString & dir);

protected:

    QString m_lastVisitedDirectory;  
    SceneManager    * m_sceneManager;
    IMPORTERROR ImportObjOrMinc(SceneObject *o);
};

#endif //TAG_IMPORTMENUFUNCTIONS_H_

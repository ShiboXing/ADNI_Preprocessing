/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: viewmenufunctions.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:07 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
// .NAME ViewMenuFunctions - all the functions serving the view menu in MainWindow
// .SECTION Description
// This file is created in order to remove the bulk of the code from MainWindow
// in ibis and MincView
#ifndef TAG_VIEWMENUFUNCTIONS_H_
#define TAG_VIEWMENUFUNCTIONS_H_

#include <qobject.h>
#include "scenemanager.h"

class ViewMenuFunctions : public QObject
{

    Q_OBJECT

public:

    ViewMenuFunctions();
    virtual ~ViewMenuFunctions();
    
    // Description:
    // Hide/show data outline
    void ViewOutline(bool viewOn);
    // Description:
    // Hide/show X or Y or Z Plane
    void ViewPlane(bool viewOn, int index);
    // Hide/show Planes in 3D view
    void View3DPlanes(int viewOn);
    // Description:
    // Show the data in 3D view from front, back, left, right, top or bottom
    void ViewStandardPosition(STANDARDVIEW position);
    void ViewMode(int mode);
   // Description:
    // set opacity of all the objects in one shot
    void SetAllObjectsOpacity();
        
    // Description:
    // Set manager
    void SetManager(SceneManager *manager);
    
protected:

    SceneManager    * m_sceneManager;
};

#endif //TAG_VIEWMENUFUNCTIONS_H_

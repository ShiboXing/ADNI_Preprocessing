/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: viewmenufunctions.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:07 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "viewmenufunctions.h"
#include "imageobject.h"
#include "setopacitydialog.h"

ViewMenuFunctions::ViewMenuFunctions():
    m_sceneManager(0)
{
}

ViewMenuFunctions::~ViewMenuFunctions()
{
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
}

void ViewMenuFunctions::SetManager(SceneManager *manager)
{
    if (m_sceneManager == manager)
        return;
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
    m_sceneManager = manager;
    if (m_sceneManager)
        m_sceneManager->Register(0);
}

void ViewMenuFunctions::ViewOutline(bool viewOn)
{
    SceneObject *tmpObj = m_sceneManager->GetCurrentObject();
    if (tmpObj && (tmpObj->GetObjectType() & MINC_TYPE))
    {
        ImageObject * object = reinterpret_cast<ImageObject *>(tmpObj);
        if (viewOn && object->GetViewOutline() == 0 || !viewOn && object->GetViewOutline() == 1 )
            object->SetViewOutline( viewOn );
    }
}

void ViewMenuFunctions::ViewPlane(bool viewOn, int index)
{
    SceneObject *tmpObj = m_sceneManager->GetCurrentObject();
    if (tmpObj && (tmpObj->GetObjectType() & MINC_TYPE))
    {
        ImageObject * object = reinterpret_cast<ImageObject *>(tmpObj);
        if (viewOn && object->GetViewPlane(index) == 0 || !viewOn && object->GetViewPlane(index) == 1 )
            object->SetViewPlane( index, viewOn );
    }
}

void ViewMenuFunctions::View3DPlanes(int viewOn)
{
    SceneObject *tmpObj = m_sceneManager->GetCurrentObject();
    if (tmpObj && (tmpObj->GetObjectType() & MINC_TYPE))
    {
        ImageObject * object = reinterpret_cast<ImageObject *>(tmpObj);
        if (viewOn && object->GetView3DPlanes() == 0 || !viewOn && object->GetView3DPlanes() == 1 )
            object->SetView3DPlanes( viewOn );
    }
}

void ViewMenuFunctions::ViewStandardPosition(STANDARDVIEW position)
{
    if (m_sceneManager)
        m_sceneManager->SetStandardView(position);
}

void ViewMenuFunctions::ViewMode(int mode)
{
    if (m_sceneManager)
        m_sceneManager->SetViewMode(mode);
}

void ViewMenuFunctions::SetAllObjectsOpacity()
{
    if (m_sceneManager)
    {
        SetOpacityDialog *dlg = new SetOpacityDialog();
        dlg->SetManager(m_sceneManager);
        dlg->show();
    }
}
        
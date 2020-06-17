/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: importmenufunctions.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "importmenufunctions.h"

#include <qfiledialog.h>
#include <qmessagebox.h>
#include "imageobject.h"
#include "polydataobject.h"

ImportMenuFunctions::ImportMenuFunctions()
{
    m_sceneManager = 0;
    m_lastVisitedDirectory = QDir::homeDirPath();
}

ImportMenuFunctions::~ImportMenuFunctions()
{
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
}

void ImportMenuFunctions::SetManager(SceneManager *manager)
{
    if (m_sceneManager == manager)
        return;
    if (m_sceneManager)
        m_sceneManager->UnRegister(0);
    m_sceneManager = manager;
    if (m_sceneManager)
        m_sceneManager->Register(0);
}

void ImportMenuFunctions::SetLastVisitedDirectory(const QString & dir)
{
    m_lastVisitedDirectory = dir;
    if( !QFile::exists( m_lastVisitedDirectory ) )
    {
        m_lastVisitedDirectory = QDir::homeDirPath();
    }
}

IMPORTERROR ImportMenuFunctions::ImportObj()
{
    IMPORTERROR err = IMP_NO_ERR;
    if (m_sceneManager)
    {
        SceneObject *obj = m_sceneManager->GetCurrentObject();
        if (! obj || obj->GetObjectManagedBySystem() ||!(obj->GetObjectType() & MINC_TYPE))
            obj = m_sceneManager->GetUniqueTypedObject(MINC_TYPE);
        if (obj)
            err = ImportObjOrMinc(obj);
        else
            err = IMP_NO_OBJECT;
    }
    return err;
}

IMPORTERROR ImportMenuFunctions::ImportMinc()
{
    IMPORTERROR err = IMP_NO_ERR;
    if (m_sceneManager)
    {
        SceneObject *obj = m_sceneManager->GetCurrentObject();
        if (! obj || obj->GetObjectManagedBySystem() ||!(obj->GetObjectType() & OBJ_TYPE))
            obj = m_sceneManager->GetUniqueTypedObject(OBJ_TYPE);
        if (obj)
            err = ImportObjOrMinc(obj);
        else
            err = IMP_NO_OBJECT;
    }
    return err;
}

void ImportMenuFunctions::RemoveImportedObjOrMinc()
{
    if (m_sceneManager)
    {
        SceneObject *importedObject = m_sceneManager->FindImportedObject();
        if (importedObject)
        {
            m_sceneManager->DisAssociateMincWithObj(importedObject);
            m_sceneManager->RemoveObject(importedObject);
        }
    }
}

IMPORTERROR ImportMenuFunctions::ImportObjOrMinc(SceneObject *objectThatNeedsImport)
{
    bool imported = FALSE;
    int type;
    QString filename, name;
    IMPORTERROR err = IMP_NO_ERR;
    
    if (m_sceneManager )
    {
        if (objectThatNeedsImport)
        {
            if (m_sceneManager->GetMincAndObj() == 0)
            {
                type = objectThatNeedsImport->GetObjectType();
            
                if (type & MINC_TYPE)
                    filename = QFileDialog::getOpenFileName( m_lastVisitedDirectory, "Objfile (*.obj)", 0 , "open", "Open obj file" );
                else
                    filename = QFileDialog::getOpenFileName( m_lastVisitedDirectory, "Minc file (*.mnc)", 0 , "open", "Open minc file" );
                if( !filename.isNull() )
                {
                    SceneObject *rootObj = 0;
                    int ret = 0;
                    if (type & PREOP_TYPE)
                    {
                        rootObj = m_sceneManager->GetObject(PREOP_ROOT_OBJECT_NAME);
                        ret = m_sceneManager->OpenFile( filename, rootObj, PREOP_TYPE );
                    }
                    else
                    {
                        rootObj = m_sceneManager->GetObject(INTRAOP_ROOT_OBJECT_NAME);
                        ret = m_sceneManager->OpenFile( filename, rootObj, INTRAOP_TYPE );
                    }
                    if (ret == 1)
                    {
                        QFileInfo info( filename );
                        imported = TRUE;
                        name = info.fileName();
                    }
                    else
                        QMessageBox::warning( 0, "Error: can't open file: ", filename, 1, 0 );
                }
                if (imported)
                {
                    // get the last added object
                    ImageObject * iObj;
                    PolyDataObject* pObj;
                    
                    if (type & MINC_TYPE)
                    {
                        iObj = reinterpret_cast<ImageObject*>(objectThatNeedsImport);
                        pObj = (PolyDataObject*)m_sceneManager->GetObject( name );
                        pObj->SetObjectIsImported(true);
                    }
                    else
                    {
                        pObj = reinterpret_cast<PolyDataObject*>(objectThatNeedsImport);
                        iObj = (ImageObject*)m_sceneManager->GetObject( name );
                        iObj->SetObjectIsImported(true);
                    }
                    m_sceneManager->AssociateMincWithObj(iObj, pObj);
                }
            }
            else
                err = IMP_ONLY_ONE;
        }
        else
        {
            err = IMP_NO_OBJECT;
        }
    }
    return err;
}
        

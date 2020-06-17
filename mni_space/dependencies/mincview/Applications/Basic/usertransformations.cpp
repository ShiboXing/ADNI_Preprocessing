/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: usertransformations.cpp,v $
  Language:  C++
  Date:      $Date: 2010-04-28 19:06:03 $
  Version:   $Revision: 1.11 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "usertransformations.h"
#include "vtkLinearTransform.h"
#include "vtkmatrix4x4utilities.h"
#include <qstringlist.h>

ObjectSerializationMacro( UserTransformations );
ObjectSerializationMacro( TransformParams );

TransformParams::TransformParams()
{
    predefinedTransform = 1;
    type = ABSOLUTE_TRANSFORM;
    transform = 0;
    referenceCounter = 0;
}   

TransformParams::~TransformParams()
{
    if (transform)
    {
        transform->Delete();
    }
}

void TransformParams::Serialize( Serializer * ser )
{
    vtkMatrix4x4 *mat;
    ::Serialize( ser, "TransformationName", name );
    ::Serialize( ser, "TransformationType", type );
    ::Serialize( ser, "TransformationIsPredefined", predefinedTransform );
    if( ser->IsReader())
    {
        transform = vtkTransform::New();
        transform->Identity();
        mat = vtkMatrix4x4::New();
        mat->Identity();
        ::Serialize( ser, "TransformationMatrix", mat );
        transform->Concatenate(mat);
    }
    else
    {
        mat = 0;
        if (transform)
            mat = transform->GetMatrix(); 
        if (mat)
            ::Serialize( ser, "TransformationMatrix", mat );
    }
}

UserTransformations::UserTransformations()
{
    m_numPredefined = 0;
    // first is identity
    TransformParams *par = new TransformParams;
    par->name = IGNS_IDENTITY_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    m_userTransforms.push_back( par );
    m_numPredefined++;
    // then registration
    par = new TransformParams;
    par->name = IGNS_REGISTRATION;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    // now all the incremental predefined
    par = new TransformParams;
    par->name = IGNS_X_PLUS_1;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(1.0, 0.0, 0.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_Y_PLUS_1;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(0.0, 1.0, 0.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_Z_PLUS_1;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(0.0, 0.0, 1.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_X_MINUS_1;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(-1.0, 0.0, 0.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_Y_MINUS_1;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(0.0, -1.0, 0.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_Z_MINUS_1;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(0.0, 0.0, -1.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_X_PLUS_5;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(5.0, 0.0, 0.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_Y_PLUS_5;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(0.0, 5.0, 0.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_Z_PLUS_5;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(0.0, 0.0, 5.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_X_MINUS_5;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(-5.0, 0.0, 0.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_Y_MINUS_5;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(0.0, -5.0, 0.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
    par = new TransformParams;
    par->name = IGNS_Z_MINUS_5;
    par->type = INCREMENTAL_TRANSFORM;
    par->transform = vtkTransform::New();
    par->transform->Identity();
    par->transform->Translate(0.0, 0.0, -5.0);
    m_userTransforms.push_back( par );
    m_registered = false;
    m_numPredefined++;
}

UserTransformations::~UserTransformations()
{
}

void UserTransformations::Serialize( Serializer * ser )
{
    int n = m_userTransforms.size()-m_numPredefined; 
    ::Serialize( ser, "NumberOfUserTransformations", n );
    TransformParams *par;
    for (int i = 0; i < n; i++)
    {
        if( ser->IsReader())
        {
            par = new TransformParams;
            m_userTransforms.push_back( par );
        }
        else
        {
            par = m_userTransforms[i+m_numPredefined];
        }
        QString transformNo = QString( "Transform_%1" ).arg(i);
        ::Serialize( ser, transformNo, m_userTransforms[i+m_numPredefined] );
    }    
}

void UserTransformations::SetRegistrationTransform(vtkLinearTransform *regTransform)
{
    m_userTransforms[1]->transform->SetInput(regTransform);
}

vtkTransform * UserTransformations::GetRegistrationTransform()
{
    return m_userTransforms[1]->transform;
}

void UserTransformations::GetTransformList(QStringList &names)
{
    names.clear();
    UserTransformList::iterator it = m_userTransforms.begin();
    for( ; it != m_userTransforms.end(); ++it )
    {
        names.append((*it)->name);
    }
}

bool UserTransformations::TransformIsPredefined(int index )
{
    if (index < (int)m_userTransforms.size() && index > -1)
        return m_userTransforms[index]->predefinedTransform == 1;
    return false;
}

int UserTransformations::GetTransformType(int index)
{
    if (index < (int)m_userTransforms.size() && index > -1)
        return m_userTransforms[index]->type;
    return UNKNOWN_TRANSFORM_TYPE;
}

QString UserTransformations::GetTransformName(int index)
{
    if (index < (int)m_userTransforms.size() && index > -1)
        return m_userTransforms[index]->name;
    return "";
}

bool UserTransformations::TransformIsPredefined(QString transformName )
{
    UserTransformList::iterator it = m_userTransforms.begin();
    for( ; it != m_userTransforms.end(); ++it )
    {
        if ((*it)->name == transformName)
            return ((*it)->predefinedTransform==1);
    }
    return false;
}

int UserTransformations::GetTransformType(QString transformName)
{
    UserTransformList::iterator it = m_userTransforms.begin();
    for( ; it != m_userTransforms.end(); ++it )
    {
        if ((*it)->name == transformName)
            return (*it)->type;
    }
    return UNKNOWN_TRANSFORM_TYPE;
}

int UserTransformations::GetTransformIndex(QString transformName)
{
    UserTransformList::iterator it = m_userTransforms.begin();
    for( int i = 0; it != m_userTransforms.end(); ++it, i++ )
    {
        if ((*it)->name == transformName)
            return i;
    }
    return -1;
}

vtkMatrix4x4 * UserTransformations::GetTransformMatrix(int index)
{
    if (index < (int)m_userTransforms.size() && index > -1)
    {
        if (m_userTransforms[index]->transform)
            return m_userTransforms[index]->transform->GetMatrix();
    }
    return 0;
}

vtkMatrix4x4 * UserTransformations::GetTransformMatrix(QString transformName)
{
    UserTransformList::iterator it = m_userTransforms.begin();
    for( ; it != m_userTransforms.end(); ++it )
    {
        if ((*it)->name == transformName)
        {
            if ((*it)->transform)
                return (*it)->transform->GetMatrix();
            return 0;
        }
    }
    return 0;
}

int UserTransformations::GetReferenceCounter(int index)
{
    if (index < (int)m_userTransforms.size() && index > -1)
    {
        return m_userTransforms[index]->referenceCounter;
    }
    return 0;
}

int UserTransformations::GetReferenceCounter(QString name)
{
    UserTransformList::iterator it = m_userTransforms.begin();
    for( ; it != m_userTransforms.end(); ++it )
    {
        if ((*it)->name == name)
            return (*it)->referenceCounter;
    }
    return 0;
}

void UserTransformations::IncrementReferenceCounter(int index)
{
    if (index < (int)m_userTransforms.size() && index > -1)
    {
        m_userTransforms[index]->referenceCounter++;
    }
}

void UserTransformations::IncrementReferenceCounter(QString name)
{
    UserTransformList::iterator it = m_userTransforms.begin();
    for( ; it != m_userTransforms.end(); ++it )
    {
        if ((*it)->name == name)
            (*it)->referenceCounter++;
    }
}

void UserTransformations::DecrementReferenceCounter(int index)
{
    if (index < (int)m_userTransforms.size() && index > -1)
    {
        m_userTransforms[index]->referenceCounter--;
    }
}

void UserTransformations::DecrementReferenceCounter(QString name)
{
    UserTransformList::iterator it = m_userTransforms.begin();
    for( ; it != m_userTransforms.end(); ++it )
    {
        if ((*it)->name == name)
            (*it)->referenceCounter--;
    }
}

void UserTransformations::AddUserTransform(const QString &name, int type, bool postMultiply, vtkMatrix4x4 *mat)
{
    TransformParams *par = new TransformParams;
    par->name = name;
    par->type = type;
    par->predefinedTransform = 0;
    vtkTransform *tmp = vtkTransform::New();
    tmp->Identity();
    tmp->SetMatrix(mat);
    if (postMultiply)
        tmp->PostMultiply();
    par->transform = tmp;
    m_userTransforms.push_back( par );
    emit TransformsChanged();
}

void UserTransformations::DeleteUserTransform(const QString &name)
{
    UserTransformList::iterator it = m_userTransforms.begin();
    TransformParams *params = 0;
    for( ; it != m_userTransforms.end(); ++it )
    {
        if ((*it)->name == name)
        {
            params = (*it);
            m_userTransforms.erase(it);
            break;
        }
    }
    if (params)
        delete params;
}

void UserTransformations::UpdateUserTransform(const QString &oldName,const QString &name, int type, bool postMultiply, vtkMatrix4x4 *mat)
{
    UserTransformList::iterator it = m_userTransforms.begin();
    for( ; it != m_userTransforms.end(); ++it )
    {
        if ((*it)->name == oldName)
        {
            (*it)->name = name;
            (*it)->type = type;
            vtkMatrix4x4 *tmp = (*it)->transform->GetMatrix();
            tmp->DeepCopy(mat);
            if (postMultiply)
                (*it)->transform->PostMultiply();
        }
    }
    emit TransformsChanged();
}

vtkTransform *UserTransformations::GetUserTransform(int index)
{
    if (index < (int)m_userTransforms.size() && index > -1)
        return m_userTransforms[index]->transform;
    return 0;
}

vtkTransform *UserTransformations::GetUserTransform(QString transformName)
{
    UserTransformList::iterator it = m_userTransforms.begin();
    for( ; it != m_userTransforms.end(); ++it )
    {
        if ((*it)->name == transformName)
        {
            return (*it)->transform;
        }
    }
    return 0;
}

int UserTransformations::GetExpertLevel()
{
    return m_expertMode->GetExpertLevel();
}

/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: expertmode.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:33 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/
#include "expertmode.h"

ObjectSerializationMacro( ExpertMode );

ExpertMode::ExpertMode():
    m_expertLevel(0)
{
}

ExpertMode::~ExpertMode()
{
}

void ExpertMode::Serialize(Serializer * serializer)
{
    ::Serialize( serializer, "ExpertLevel", m_expertLevel );
}


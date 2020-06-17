/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: expertmode.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:33 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/
/*
    For now there is only on/off - 1 expert, 0 standard
*/
#ifndef TAG_EXPERTMODE_H_
#define TAG_EXPERTMODE_H_

#include "serializer.h"

class ExpertMode
{
            
public:
    ExpertMode();
    ~ExpertMode();
    void Serialize( Serializer * serializer );
    
    void SetExpertLevel(int mode) {m_expertLevel = mode;}
    int GetExpertLevel() {return m_expertLevel;}
    
     
protected:
    
    int m_expertLevel;
};
ObjectSerializationHeaderMacro( ExpertMode );

#endif //TAG_EXPERTMODE_H_

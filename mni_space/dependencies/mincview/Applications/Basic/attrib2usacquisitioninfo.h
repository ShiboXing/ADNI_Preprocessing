/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: attrib2usacquisitioninfo.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:56:33 $
  Version:   $Revision: 1.2 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef ATTRIB2USACQUISITIONINFO_H
#define ATTRIB2USACQUISITIONINFO_H

#include "usacquisitioninfo.h"
#include <qobject.h>
#include <qstring.h>

class vtkMINCImageAttributes;

class Attrib2USAcquisitionInfo : public QObject
{
    Q_OBJECT

public:
    Attrib2USAcquisitionInfo();
    ~Attrib2USAcquisitionInfo();

    void FillUSAcquisitionInfo( vtkMINCImageAttributes * attributes );
    USAcquisitionInfo * GetUSAcquisitionInfo( ) {return m_acquisitionInfo;}
    void SetUSAcquisitionInfo(USAcquisitionInfo * info );
    
protected:
    USAcquisitionInfo *m_acquisitionInfo;
    
};


#endif // ATTRIB2USACQUISITIONINFO_H

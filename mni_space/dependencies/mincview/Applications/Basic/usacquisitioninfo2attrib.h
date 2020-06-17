/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: usacquisitioninfo2attrib.h,v $
  Language:  C++
  Date:      $Date: 2010-02-22 14:55:28 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef USACQUISITIONINFO2ATTRIB_H
#define USACQUISITIONINFO2ATTRIB_H

#include "usacquisitioninfo.h"
#include <qobject.h>
#include <qstring.h>

class vtkMINCImageAttributes;

class USAcquisitionInfo2Attrib : public QObject
{
    Q_OBJECT

public:
    USAcquisitionInfo2Attrib();
    ~USAcquisitionInfo2Attrib();

    void FillImageAttributes( vtkMINCImageAttributes * attributes );
    USAcquisitionInfo * GetUSAcquisitionInfo( ) {return m_acquisitionInfo;}
    void SetUSAcquisitionInfo(USAcquisitionInfo * info );
    
protected:
    USAcquisitionInfo *m_acquisitionInfo;

};


#endif // USACQUISITIONINFO2ATTRIB_H

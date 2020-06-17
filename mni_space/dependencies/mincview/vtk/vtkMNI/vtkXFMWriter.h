/*=========================================================================

  Program:   Visualization Toolkit Bic Extension
  Module:    $RCSfile: vtkXFMWriter.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:26:57 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.
  
=========================================================================*/
// .NAME vtkXFMWriter - Write matrix in an XFM format
// .SECTION Description
// This is a preliminary version that writes only Single linear transformation.

#ifndef TAG_VTKXFMWRITER_H
#define TAG_VTKXFMWRITER_H

#include "vtkObject.h"

class vtkMatrix4x4;

class vtkXFMWriter : public vtkObject
{

public:

    static vtkXFMWriter * New();
    vtkTypeRevisionMacro(vtkXFMWriter,vtkObject);

    vtkSetStringMacro(FileName);
    
    virtual void Write();   
    virtual void SetMatrix(vtkMatrix4x4 *mat);
    
    void PrintSelf(ostream &os, vtkIndent indent);

protected:

    char * FileName;
    vtkMatrix4x4 *Matrix;

    vtkXFMWriter();
    virtual ~vtkXFMWriter();

private:
    
    vtkXFMWriter(const vtkXFMWriter&);    // Not implemented.
    void operator=(const vtkXFMWriter&);  // Not implemented.

};

#endif //TAG_VTKXFMWRITER_H





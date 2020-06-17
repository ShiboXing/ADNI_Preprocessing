/*=========================================================================

  Program:   Visualization Toolkit Bic Extension
  Module:    $RCSfile: vtkXFMReader.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:26:57 $
  Version:   $Revision: 1.6 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.
  
=========================================================================*/
// .NAME vtkXFMReader - Read XFM files into vtkMatrix4x4
// .SECTION Description
// This is a preliminary version that reads only Single linear transformation.

#ifndef TAG_VTKXFMREADER_H
#define TAG_VTKXFMREADER_H

#include "vtkObject.h"


class vtkMatrix4x4;

class vtkXFMReader : public vtkObject
{

public:

    static vtkXFMReader * New();
    vtkTypeRevisionMacro(vtkXFMReader,vtkObject);

    vtkSetStringMacro(FileName);
    virtual void Update();
    virtual void SetMatrix(vtkMatrix4x4 *mat);
    virtual int CanReadFile( const char * fname );
    
    void PrintSelf(ostream &os, vtkIndent indent);

//BTX
protected:

    char * FileName;
    vtkMatrix4x4 *Matrix;

    vtkXFMReader();
    ~vtkXFMReader();

private:
    vtkXFMReader(const vtkXFMReader&);      // Not implemented.
    void operator=(const vtkXFMReader&);  // Not implemented.
//ETX
};
#endif //TAG_VTKXFMREADER_H





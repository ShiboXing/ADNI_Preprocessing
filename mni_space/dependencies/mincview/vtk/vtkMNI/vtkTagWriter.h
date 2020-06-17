/*=========================================================================

  Program:   Visualization Toolkit Bic Extension
  Module:    $RCSfile: vtkTagWriter.h,v $
  Language:  C++
  Date:      $Date: 2010-03-03 19:23:45 $
  Version:   $Revision: 1.5 $

  Copyright (c) 2002-2008 IPL, BIC, MNI, McGill,Simon Drouin
  All rights reserved.
  
=========================================================================*/
// .NAME vtkTagWriter - Write vtkPoints into MNI tag files
// .SECTION Description

#ifndef __vtkTagWriter_h
#define __vtkTagWriter_h

#include <string>
#include <vector>
#include "vtkObject.h"

class vtkPoints;

class vtkTagWriter : public vtkObject
{

public:

    static vtkTagWriter * New() { return new vtkTagWriter; }

    vtkSetStringMacro(FileName);
    
    void Write();
    
    void AddVolume( vtkPoints * volume, const char * name );

    void SetPointNames( std::vector<std::string> & names );

    void SetReferenceDataFile(const char * ts);
    void SetTimeStamp(const char * ts);
    void SetTransformToSave(const char * ts);

    void PrintSelf(ostream &os, vtkIndent indent);

protected:

    char * FileName;
    typedef std::vector<vtkPoints*> PointsVec;
    PointsVec Volumes;
    char TimeStamp[64];
    char TransformToSave[256];
    char ReferenceDataFile[128];
    std::vector<std::string> VolumeNames;
    std::vector<std::string> PointNames;

    vtkTagWriter();
    ~vtkTagWriter();

private:
    
    vtkTagWriter(const vtkTagWriter&);    // Not implemented.
    void operator=(const vtkTagWriter&);  // Not implemented.

};

#endif





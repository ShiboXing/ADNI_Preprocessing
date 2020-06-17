/*=========================================================================

  Program:   Visualization Toolkit Bic Extension
  Module:    $RCSfile: vtkTagReader.h,v $
  Language:  C++
  Date:      $Date: 2010-03-03 19:23:45 $
  Version:   $Revision: 1.7 $

  Copyright (c) 2007-2008 IPL, BIC, MNI, McGill, Simon Drouin
  All rights reserved.
  
=========================================================================*/
// .NAME vtkTagReader - Read MNI tag files into vtkPoints
// .SECTION Description

#ifndef __vtkTagReader_h
#define __vtkTagReader_h

#include <string>
#include <vector>
#include "vtkObject.h"
#include "vtkMatrix4x4.h"


class vtkPoints;

class vtkTagReader : public vtkObject
{

public:

    static vtkTagReader * New();
    vtkTypeRevisionMacro(vtkTagReader,vtkObject);

    vtkSetStringMacro(FileName);
    int CanReadFile( const char * fname );
    void Update();
    
    char * GetReferenceDataFileName();
    char * GetTimeStamp();
    vtkMatrix4x4 * GetSavedTransform();

    // Get the output of the reader
    int GetNumberOfVolumes();
    vtkPoints * GetVolume( unsigned int volumeIndex );
    std::vector<std::string> & GetPointNames() { return PointNames; }
    std::vector<std::string> & GetVolumeNames() { return VolumeNames; }

    void PrintSelf(ostream &os, vtkIndent indent);

//BTX
protected:

    char * FileName;
    typedef std::vector<vtkPoints*> PointsVec;
    PointsVec Volumes;
    std::vector<std::string> PointNames;
    std::vector<std::string> VolumeNames;
    char TimeStamp[64];
    vtkMatrix4x4 *SavedTransform;
    char ReferenceDataFile[128];
    
    void ClearOutput();

    vtkTagReader();
    ~vtkTagReader();

private:
    vtkTagReader(const vtkTagReader&);      // Not implemented.
    void operator=(const vtkTagReader&);  // Not implemented.
//ETX
};
#endif





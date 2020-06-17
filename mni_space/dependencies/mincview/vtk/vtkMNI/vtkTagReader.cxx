/*=========================================================================

  Program:   Visualization Toolkit Bic Extension
  Module:    $RCSfile: vtkTagReader.cxx,v $
  Language:  C++
  Date:      $Date: 2010-03-03 19:23:45 $
  Version:   $Revision: 1.7 $

  Copyright (c) 2002-2008 IPL, BIC, MNI, McGill, Simon Drouin
  All rights reserved.
  
=========================================================================*/
#include "vtkTagReader.h"
#include "vtkPoints.h"
#include "vtkReal.h"
#include <sstream>
#include "stringtools.h"

vtkCxxRevisionMacro(vtkTagReader, "$Revision: 1.7 $");

vtkTagReader * vtkTagReader::New()
{
    return new vtkTagReader();
}

int vtkTagReader::GetNumberOfVolumes()
{
    return this->Volumes.size();
}

vtkPoints * vtkTagReader::GetVolume( unsigned int volumeIndex )
{
    if( volumeIndex >= 0 && volumeIndex < this->Volumes.size() )
    {
        return this->Volumes[ volumeIndex ];
    }
    return 0;
}

#define MAX_LINE_LENGTH 1024

void vtkTagReader::Update()
{
    // clear previous data
    PointsVec::iterator it = this->Volumes.begin();
    for( ; it != this->Volumes.end(); ++it )
    {
        (*it)->Reset();
    }
    this->PointNames.clear();
    this->VolumeNames.clear();

    // See if a filename has been provided
    if( !this->FileName )
    {
        vtkErrorMacro(<< "No tag file name was provided" << endl);
        return;
    }

    // try to open the file
    ifstream f( this->FileName );
    if( !f.is_open() )
    {
        vtkErrorMacro( << "Couldn't open file " << this->FileName << endl );
        return;
    }
    
    // Get the number of volumes in the file
    int found = 0;
    char tempLine[MAX_LINE_LENGTH];
    unsigned int numberOfVolumes = 0;
    while( !found && !f.eof() )
    {
        f.getline( tempLine, MAX_LINE_LENGTH );
        std::string line( tempLine );
        std::vector<std::string> tokens;
        Tokenize( line, tokens );
        if( tokens.size() == 3 && tokens[0] == "Volumes" )
        {
            std::istringstream stream( tokens[2] );
            stream >> numberOfVolumes;
            found = 1;
        }
    }
    
    if( !found )
    {
        vtkErrorMacro( << "Couldn't determine the number of volumes in the file" << endl );
        return;
    }
    unsigned int i, j;
    for( i = 0; i < numberOfVolumes; i++ )
    {
        vtkPoints * volume = vtkPoints::New();
        this->Volumes.push_back( volume );
    }
    
    // find the definition of the points and maybe the volume's names if present
    found = 0;
    while( !found && !f.eof() )
    {
        f.getline( tempLine, MAX_LINE_LENGTH );
        std::string line( tempLine );
        std::vector<std::string> tokens;
        Tokenize( line, tokens );
        if( tokens.size() )
        {
            if( tokens[0] == "%Volume:" )
            {
                if( tokens.size() > 1 )
                    this->VolumeNames.push_back( tokens[1] );
                else
                    this->VolumeNames.push_back( "" );
            }
            else if( tokens[0] == "%ReferenceDataFile:" )
            {
                strcpy(this->ReferenceDataFile, tokens[1].c_str());
            }
            else if( tokens[0] == "%Transform:" )
            {
                // read matrix elements
                unsigned int tokenIndex = 1;
                this->SavedTransform = vtkMatrix4x4::New();
                this->SavedTransform->Identity();
                for( i = 0; i < 3; i++ )
                {
                    for ( j = 0; j < 4; j ++)
                    {
                        this->SavedTransform->SetElement(i, j, atof(tokens[tokenIndex].c_str()));
                        tokenIndex++;
                    }
                }
            }
            else if( tokens[0] == "%TimeStamp:" )
            {
                strcpy(this->TimeStamp, tokens[1].c_str());
            }
            else if( tokens[0] == "Points" )
            {
                found = 1;
            }
        }
    }
    if( this->VolumeNames.size() < numberOfVolumes )
    {
        for( i = this->VolumeNames.size() - 1; i < numberOfVolumes; i++ )
        {
            this->VolumeNames.push_back( "" );
        }
    }
    
    if( !found )
    {
        vtkErrorMacro( << "Couldn't find the beginning of the definition of points in the file" << endl );
        return;
    }

    // Read each volume's points
    int lastLine = 0;
    while( !lastLine && !f.eof() )
    {
        f.getline( tempLine, MAX_LINE_LENGTH );
        std::string line( tempLine );
        std::vector<std::string> tokens;
        Tokenize( line, tokens );
        
        if( line.rfind( ';' ) != std::string::npos )
        {
            lastLine = 1;
        }
        
        // read the points for each volume
        unsigned int tokenIndex = 0;
        for( i = 0; i < numberOfVolumes; i++ )
        {
            vtkReal newPoint[3] = { 0, 0, 0 };
            for( int j = 0; j < 3; j++ )
            {
                std::istringstream stream( tokens[ tokenIndex ] );
                stream >> newPoint[j];
                tokenIndex++;
            }
            this->Volumes[i]->InsertNextPoint( newPoint );
        }
        
        // lets see if we have <Weight> <Structure> <Patient> data which is optional
        if( tokens.size() > 3 * numberOfVolumes + 1 )
        {
            // TODO: keep that in memory somewhere
            tokenIndex += 3;
        }
        
        // Let's see if we have a tag name
        if( tokens.size() > tokenIndex )
        {
            // strip off the quotes from the name string first
            std::string quotedName(tokens[tokenIndex]);
            if (!lastLine)
            {
                this->PointNames.push_back(quotedName.substr(1,quotedName.length()-2));
            }
            else
            {
                this->PointNames.push_back(quotedName.substr(1,quotedName.length()-3));
            }
            
        }
        else
        {
            this->PointNames.push_back( "" );
        }
    }
    f.close();
}

void vtkTagReader::ClearOutput()
{
    // clear previous data
    PointsVec::iterator it = this->Volumes.begin();
    for( ; it != this->Volumes.end(); ++it )
    {
        (*it)->Delete();
    }
    this->PointNames.clear();
    this->VolumeNames.clear();
}

char * vtkTagReader::GetReferenceDataFileName()
{
    if (this->ReferenceDataFile[0])
        return this->ReferenceDataFile;
    return 0;        
}

char * vtkTagReader::GetTimeStamp()
{
    if (this->TimeStamp[0])
        return this->TimeStamp;
    return 0;        
}

vtkMatrix4x4 * vtkTagReader::GetSavedTransform()
{
    return this->SavedTransform;
}

vtkTagReader::vtkTagReader()
{
    this->TimeStamp[0] = 0;
    this->SavedTransform = 0;
    this->ReferenceDataFile[0] = 0;
    this->FileName = 0;
}

vtkTagReader::~vtkTagReader()
{
    this->ClearOutput();
    
    if( this->FileName )
        delete this->FileName;
    if (this->SavedTransform)        
        this->SavedTransform->Delete();
}


void vtkTagReader::PrintSelf(ostream &os, vtkIndent indent)
{
    os << indent << "FileName: " << this->FileName << endl;
    os << indent << "NumberOfVolumes: " << this->Volumes.size() << endl;
    for( unsigned int i = 0; i < this->Volumes.size(); i++ )
    {
        os << indent << "Volume name: " << this->VolumeNames[i].c_str() << endl;
        os << indent << "Points:" << endl;
        for( int j = 0; j < this->Volumes[i]->GetNumberOfPoints(); j++ )
        {
            vtkReal * point = this->Volumes[i]->GetPoint( j );
            os << indent << "( " << point[0] << ", " << point[1] << ", " << point[2] << " )" << endl;
        }
    }        
}

//-------------------------------------------------------------------------------
int vtkTagReader::CanReadFile( const char * fname )
{
    ifstream f( fname );
    if( !f.is_open() )
    {
        vtkErrorMacro( << "Couldn't open file " << this->FileName << endl );
        return 0;
    }
    
    // read the first line of the file to make sure it is a tag file
    char fileType[30];
    f.getline( fileType, 30 );
    f.close();
    if( strcmp( fileType, "MNI Tag Point File" ) != 0 )
    {
        vtkErrorMacro( << "This is not an MNI Tag file." << endl );
        return 0;
    }
    return 1;
}


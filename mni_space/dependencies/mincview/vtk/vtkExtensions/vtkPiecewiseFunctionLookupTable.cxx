/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPiecewiseFunctionLookupTable.cxx,v $
  Language:  C++
  Date:      $Date: 2004-04-19 15:52:22 $
  Version:   $Revision: 1.1 $
  Thanks:    Thanks to Simon Drouin who developed this class.

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPiecewiseFunctionLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"

vtkCxxRevisionMacro(vtkPiecewiseFunctionLookupTable, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkPiecewiseFunctionLookupTable);


// Construct with range=(0,1); and hsv ranges set up for rainbow color table
// (from red to blue).
vtkPiecewiseFunctionLookupTable::vtkPiecewiseFunctionLookupTable()
{
    this->ColorFunction = vtkColorTransferFunction::New();
    this->AlphaFunction = vtkPiecewiseFunction::New();
}


vtkPiecewiseFunctionLookupTable::~vtkPiecewiseFunctionLookupTable()
{
    this->ColorFunction->Delete();
    this->AlphaFunction->Delete();
}


// Force the lookup table to rebuild
void vtkPiecewiseFunctionLookupTable::ForceBuild()
{
    int number = this->NumberOfColors;
    float step = (float)1.0 / (number - 1);
    float x = 0.0;
    float r, g, b, a;
    for( int i = 0; i < number; i++ )
    {
        r = this->ColorFunction->GetRedValue( x );
        g = this->ColorFunction->GetGreenValue( x );
        b = this->ColorFunction->GetBlueValue( x );
        a = this->AlphaFunction->GetValue( x );
        this->SetTableValue( i, r, g, b, a );
        x += step;
    }

    this->BuildTime.Modified();
}


void vtkPiecewiseFunctionLookupTable::AddColorPoint( float value, float r, float g, float b )
{
    this->ColorFunction->AddRGBPoint( value, r, g, b );
}


void vtkPiecewiseFunctionLookupTable::AddAlphaPoint( float value, float alpha )
{
    this->AlphaFunction->AddPoint( value, alpha );
}


void vtkPiecewiseFunctionLookupTable::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    this->ColorFunction->PrintSelf( os, indent );
    this->AlphaFunction->PrintSelf( os, indent );
}



/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPiecewiseFunctionLookupTable.h,v $
  Language:  C++
  Date:      $Date: 2004-05-11 21:17:29 $
  Version:   $Revision: 1.3 $
  Thanks:    Thanks to Simon Drouin who developed this class.

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPiecewiseFunctionLookupTable - Lookup table built using piecewise function for every rgba component
// .SECTION Description
// vtkPiecewiseFunctionLookupTable is a lookup table that is built using
// 4 different piecewise function (r,g,b,a). It samples the each of the
// functions to build its internal table.
//
// .SECTION See Also
// vtkColorTransferFunction, vtkPiecewiseFunction, vtkLookupTable

#ifndef __vtkPiecewiseFunctionLookupTable_h
#define __vtkPiecewiseFunctionLookupTable_h


#include "vtkReal.h"
#include "vtkLookupTable.h"


class vtkColorTransferFunction;
class vtkPiecewiseFunction;


class vtkPiecewiseFunctionLookupTable : public vtkLookupTable
{

public:

  // Description:
  // Construct with range=[0,1]; and hsv ranges set up for rainbow color table
  // (from red to blue).
  static vtkPiecewiseFunctionLookupTable *New();

  vtkTypeRevisionMacro(vtkPiecewiseFunctionLookupTable,vtkLookupTable);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Force the lookup table to regenerate.
  virtual void ForceBuild();

  void AddColorPoint( float value, float r, float g, float b );
  void AddAlphaPoint( float value, float alpha );

protected:

    vtkColorTransferFunction * ColorFunction;
    vtkPiecewiseFunction * AlphaFunction;

  vtkPiecewiseFunctionLookupTable();
  ~vtkPiecewiseFunctionLookupTable();

private:

  vtkPiecewiseFunctionLookupTable(const vtkPiecewiseFunctionLookupTable&);  // Not implemented.
  void operator=(const vtkPiecewiseFunctionLookupTable&);  // Not implemented.

};


#endif




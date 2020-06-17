#ifndef VTKEXTENTSIONSCOMMONINCLUDE_H
#define VTKEXTENTSIONSCOMMONINCLUDE_H

#include "vtkSystemIncludes.h"

// if vtkSystemIncludes did not define vtkFloatingPointType, it mean we are
// building with a version of vtk prior to 4.4, so we define vtkFloatingPointType as float
#ifndef vtkFloatingPointType
#define vtkFloatingPointType vtkFloatingPointType
typedef float vtkFloatingPointType;
#define VTK_REAL_IS_FLOAT
#endif

// define vtkReal just because it is shorter to write
typedef vtkFloatingPointType vtkReal;

#endif


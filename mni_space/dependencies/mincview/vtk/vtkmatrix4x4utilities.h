#ifndef __vtkMatrix4x4Utilities_h_
#define __vtkMatrix4x4Utilities_h_

inline void Serialize( Serializer * serial, const char * attrName, vtkMatrix4x4 * mat )
{
    double * elem = (double*)mat->Element;
    Serialize( serial, attrName, elem, 16 );
}

#endif
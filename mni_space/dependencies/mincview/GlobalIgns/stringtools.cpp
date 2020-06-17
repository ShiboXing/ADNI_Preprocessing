/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: stringtools.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:40:37 $
  Version:   $Revision: 1.6 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.
  
=========================================================================*/
#include <string>
#include <vector>
#include <sstream>
#include "stringtools.h"

//function Tokenize() is written by Simon Drouin
void Tokenize( const std::string & str, std::vector<std::string> & tokens, const std::string & delimiters )
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while ( std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

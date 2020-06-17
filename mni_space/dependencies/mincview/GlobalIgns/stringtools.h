/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: stringtools.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:40:37 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.

=========================================================================*/
 
#ifndef TAG_STRINGTOOLS_H
#define TAG_STRINGTOOLS_H

void Tokenize( const std::string & str, std::vector<std::string> & tokens, const std::string & delimiters = " " );

#endif //TAG_STRINGTOOLS_H
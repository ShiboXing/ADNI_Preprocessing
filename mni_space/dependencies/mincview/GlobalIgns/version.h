/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: version.h,v $
  Language:  C++
  Date:      $Date: 2010-05-27 15:54:35 $
  Version:   $Revision: 1.52 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
 
#ifndef TAG_VERSION_H
#define TAG_VERSION_H
#ifdef IBIS_DEBUG
#ifdef IGNS_ADMINISTRATOR
#define IGNS_VERSION "v.1.6.4 Admin Debug"
#else
#define IGNS_VERSION "v.1.6.4 Debug"
#endif //IGNS_ADMINISTRATOR
#else
#ifdef IGNS_ADMINISTRATOR
#define IGNS_VERSION "v.1.6.4 Admin"
#else  
#define IGNS_VERSION "v.1.6.4"
#endif //IGNS_ADMINISTRATOR
#endif // IBIS_DEBUG
#define IGNS_BUILD_DATE "May 27th, 2010"

#define IGNS_SCENE_SAVE_VERSION "1.1"

#define IGNS_IPL_HOME_PAGE "<qt><a href=toto>http://noodles.bic.mni.mcgill.ca/ResearchLabsIPL/HomePage</a></qt>"

#endif //TAG_VERSION_H
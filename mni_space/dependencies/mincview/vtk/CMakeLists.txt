PROJECT( IGNS_VTK )

#============================================================
# CHECK FOR VTK
#============================================================
FIND_PATH(VTK_DIR UseVTK.cmake $ENV{VTK_DIR} )    # put env. var VTK_DIR in cmake var VTK_DIR to help FindVTK.cmake module
INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)


#================================
# Define include dir for
# dependent projects
#================================
SET( IGNS_VTK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Include directory vtkReal.h" )
MARK_AS_ADVANCED( IGNS_VTK_INCLUDE_DIR )

#============================================================
# Always include vtkMNI which contains tools to read/write some MNI file formats
#============================================================
SUBDIRS( vtkMNI ) 

#============================================================
# Look for subdirectories if specifies by user
#============================================================

IF( IGNS_BUILD_VTK_EXTENSIONS )
    SUBDIRS( vtkExtensions )
ENDIF( IGNS_BUILD_VTK_EXTENSIONS )

IF( IGNS_BUILD_VTK_MINC )
    SUBDIRS( vtkMinc )
ENDIF( IGNS_BUILD_VTK_MINC )

IF( IGNS_BUILD_VTK_TRACKER )
    SUBDIRS( vtkTracker )
ENDIF( IGNS_BUILD_VTK_TRACKER )

IF( IGNS_BUILD_VTK_VIDEO )
    SUBDIRS( vtkVideo )
ENDIF( IGNS_BUILD_VTK_VIDEO )

IF( IGNS_BUILD_VTK_QT )
    SUBDIRS( vtkQt )
ENDIF( IGNS_BUILD_VTK_QT )

IF( IGNS_BUILD_VTK_ULTRASOUND )
    SUBDIRS( vtkUltrasound )
ENDIF( IGNS_BUILD_VTK_ULTRASOUND )

#================================
# Define include dir for
# dependent projects
#================================
SET( IGNS_VTK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Include directory for vtkMinc .h files" )
MARK_AS_ADVANCED( IGNS_VTK_INCLUDE_DIR )
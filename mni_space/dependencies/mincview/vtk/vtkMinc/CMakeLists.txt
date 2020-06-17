PROJECT( IGNS_VTK_MINC )

#========================================================
# If vtk was found, use its include directories
#========================================================
IF( VTK_FOUND )
  INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
ELSE( VTK_FOUND )
  SET (VTKMINC_CAN_BUILD 0)
ENDIF (VTK_FOUND)

#================================
# Define sources
#================================
SET( IGNS_VTK_MINC_SRC
    vtkMINCWriter.cxx
    vtkMINCReader.cxx
    vtkMINCImageReader2.cxx
    vtkMINCImageAttributes2.cxx
    vtkSimpleMINCReader.cxx
    vtkMINCIcv.cxx
    vtkMNIOBJReader.cxx )

SET( IGNS_VTK_MINC_HDR
    vtkMINCWriter.h
    vtkMINCReader.h
    vtkMINCImageReader2.h
    vtkMINCImageAttributes2.h
    vtkSimpleMINCReader.h
    vtkMINC2.h
    vtkMINCIcv.h
    vtkMNIOBJReader.h )

#================================
# Find minc headers
#================================
IF( MINC_INCLUDE_DIR )
    INCLUDE_DIRECTORIES( ${MINC_INCLUDE_DIR} )
ENDIF( MINC_INCLUDE_DIR )

#================================
# use IGSTK
#================================
IF(IGSTK_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${IGSTK_INCLUDE_DIR} )
ENDIF(IGSTK_INCLUDE_DIR)

#================================
# Define output
#================================
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../ )  # for vtkReal.h
ADD_LIBRARY( vtkMinc ${IGNS_VTK_MINC_SRC} ${IGNS_VTK_MINC_HDR} )
INSTALL_TARGETS(/lib vtkMinc)
INSTALL_FILES(/include FILES ${IGNS_VTK_MINC_HDR} ${CMAKE_CURRENT_SOURCE_DIR}/../vtkReal.h)

#================================
# Define include dir for
# dependent projects
#================================
SET( VTK_MINC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Include directory for vtkMinc .h files" )
MARK_AS_ADVANCED( VTK_MINC_INCLUDE_DIR )

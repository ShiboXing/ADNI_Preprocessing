PROJECT( IGNS_VTK_MNI )

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
SET( IGNS_VTK_MNI_SRC
    vtkTagReader.cxx
    vtkTagWriter.cxx 
    vtkXFMReader.cxx
    vtkXFMWriter.cxx )

SET( IGNS_VTK_MNI_HDR
    vtkTagReader.h
    vtkTagWriter.h
    vtkXFMReader.h
    vtkXFMWriter.h )

#================================
# Define output
#================================
INCLUDE_DIRECTORIES( ${IGNS_GLOBAL_INCLUDE_DIR})
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../ )  # for vtkReal.h
ADD_LIBRARY( vtkMNI ${IGNS_VTK_MNI_SRC} ${IGNS_VTK_MNI_HDR} )
TARGET_LINK_LIBRARIES(vtkMNI GlobalIgns)

INSTALL_TARGETS(/lib vtkMNI)
INSTALL_FILES(/include FILES ${IGNS_VTK_MNI_HDR} ${CMAKE_CURRENT_SOURCE_DIR}/../vtkReal.h)

#================================
# Define include dir for
# dependent projects
#================================
SET( VTK_MNI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Include directory for vtkMNI .h files" )
MARK_AS_ADVANCED( VTK_MNI_INCLUDE_DIR )

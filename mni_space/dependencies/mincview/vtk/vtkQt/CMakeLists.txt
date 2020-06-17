PROJECT( IGNS_VTK_QT )

SET( IGNS_VTK_QT_SRC
           vtkQtImageViewer.cpp
           vtkQtProgressDialog.cpp
           vtkQtRenderWindow.cpp
           vtkQtRenderWindowInteractor.cpp
           vtkQtCallback.cpp
           vtkQtMatrixDialog.cpp
           vtkQtAsyncCallback.cpp )

SET( IGNS_VTK_QT_HDR
           vtkQtRenderWindow.h
           vtkQtImageViewer.h
           vtkQtProgressDialog.h )

SET( IGNS_VTK_QT_HDR_MOC vtkQtRenderWindowInteractor.h vtkQtMatrixDialog.h vtkQtAsyncCallback.h vtkQtCallback.h )

#================================
# Settings for Qt
#================================

INCLUDE (${CMAKE_ROOT}/Modules/FindQt.cmake)
IF( QT_FOUND )
  INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
  ADD_DEFINITIONS(-DQT_THREAD_SUPPORT ${QT_DEFINITIONS} )  # we want to make sure we use multithreaded qt

  IF(QT_WRAP_CPP)
     QT_WRAP_CPP( vtkQt IGNS_VTK_QT_SRC ${IGNS_VTK_QT_HDR_MOC} )
  ENDIF(QT_WRAP_CPP)
ELSE( QT_FOUND )
  MESSAGE( FATAL "Qt was not found. Can't build without it" )
ENDIF( QT_FOUND )


INCLUDE_DIRECTORIES(${QT_EXTENSIONS_INCLUDE_DIR})
#================================
# Settings for VTK
#================================

IF( VTK_FOUND )
  INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
ENDIF( VTK_FOUND )


#================================
# Define sources
#================================
INCLUDE_DIRECTORIES( ${IGNS_GLOBAL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${VTK_MNI_INCLUDE_DIR})
INCLUDE_DIRECTORIES( ${IGNS_VTK_QT_SOURCE_DIR}/../ )  # for vtkReal.h
ADD_LIBRARY( vtkQt ${IGNS_VTK_QT_SRC} ${IGNS_VTK_QT_HDR} ${IGNS_VTK_QT_HDR_MOC}  )
TARGET_LINK_LIBRARIES(vtkQt qtextensions)
INSTALL_TARGETS(/lib vtkQt)
INSTALL_FILES(/include FILES ${IGNS_VTK_QT_HDR} )
#================================
# Define include dir for
# dependent projects
#================================
SET( IGNS_VTK_QT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Include directory for vtkQt .h files" )
MARK_AS_ADVANCED( IGNS_VTK_QT_INCLUDE_DIR )

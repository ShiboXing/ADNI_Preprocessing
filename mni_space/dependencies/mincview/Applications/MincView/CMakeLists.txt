PROJECT ( IGNS_MINC_VIEW )

SET( IGNS_MINC_VIEW_SRC main.cpp mainwindow.cpp application.cpp )
SET( IGNS_MINC_VIEW_HDR application.h )
SET( IGNS_MINC_VIEW_HDR_MOC mainwindow.h )

#================================
# Find vtk
#================================

INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)
IF( VTK_FOUND )
  INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
  LINK_DIRECTORIES(${VTK_LIBRARY_DIRS})
  LINK_LIBRARIES( vtkCommon vtkHybrid vtkVolumeRendering vtkWidgets )
ELSE( VTK_FOUND )
    MESSAGE( FATAL "vtk was not found. Can't build without it" )
ENDIF( VTK_FOUND )

#================================
# Find Qt
#================================

INCLUDE (${CMAKE_ROOT}/Modules/FindQt.cmake)
IF( QT_FOUND )
  INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
  ADD_DEFINITIONS(-DQT_THREAD_SUPPORT ${QT_DEFINITIONS} )  # we want to make sure we use multithreaded qt
  LINK_DIRECTORIES(${QT_QT_LIBRARY})
  LINK_LIBRARIES( ${QT_LIBRARIES} )
ELSE( QT_FOUND )
  MESSAGE( FATAL "Qt was not found. Can't build without it" )
ENDIF( QT_FOUND )

#=======================================
# moc Qt source file without a ui file
#=======================================
IF(QT_WRAP_CPP)
  QT_WRAP_CPP( IgnsPlatform IGNS_MINC_VIEW_SRC ${IGNS_MINC_VIEW_HDR_MOC} )
ENDIF(QT_WRAP_CPP)

#================================
# use vtkQt lib
#================================
IF(IGNS_VTK_QT_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${IGNS_VTK_QT_INCLUDE_DIR} )
ENDIF(IGNS_VTK_QT_INCLUDE_DIR)

#================================
# use qtExtensions
#================================
IF( QT_EXTENSIONS_INCLUDE_DIR )
    INCLUDE_DIRECTORIES( ${QT_EXTENSIONS_INCLUDE_DIR} )
ENDIF( QT_EXTENSIONS_INCLUDE_DIR )

#================================
# use Basic 
#================================
IF(BASIC_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${BASIC_INCLUDE_DIR} )
    INCLUDE_DIRECTORIES( ${BASIC_INCLUDE_DIR}/gui )
ENDIF(BASIC_INCLUDE_DIR)

#================================
# use vtkExtensions
#================================
IF(VTK_EXTENSIONS_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${VTK_EXTENSIONS_INCLUDE_DIR} )
ENDIF(VTK_EXTENSIONS_INCLUDE_DIR)

#================================
# use IGSTK
#================================
IF(IGSTK_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${IGSTK_INCLUDE_DIR} )
ENDIF(IGSTK_INCLUDE_DIR)

#================================
# use GlobalIgns
#================================
IF( IGNS_GLOBAL_INCLUDE_DIR )
    INCLUDE_DIRECTORIES( ${IGNS_GLOBAL_INCLUDE_DIR} )
ENDIF( IGNS_GLOBAL_INCLUDE_DIR )

#================================
# Define sources
#================================
SET( SOURCE ${IGNS_MINC_VIEW_HDR} ${IGNS_MINC_VIEW_HDR_MOC} ${IGNS_MINC_VIEW_SRC} )
INCLUDE_DIRECTORIES( ${IGNS_BASIC_BINARY_DIR} )    # include qt-generated headers in Basic
INCLUDE_DIRECTORIES( ${IGNS_MINC_VIEW_BINARY_DIR} )  # needed to include qt-generated source files
INCLUDE_DIRECTORIES( ${IGNS_MINC_VIEW_SOURCE_DIR}/../../vtk ) # for vtkReal.h
ADD_EXECUTABLE( MincView ${SOURCE} )
TARGET_LINK_LIBRARIES( MincView Basic vtkQt vtkMNI vtkMinc vtkExtensions)
IF (NOT WIN32)
IF( IGNS_USE_IGSTK_LOG )
TARGET_LINK_LIBRARIES( MincView ITKIO IGSTK)
ENDIF(IGNS_USE_IGSTK_LOG )
ENDIF (NOT WIN32)
IF( MINC_LIBRARIES )
    TARGET_LINK_LIBRARIES( MincView ${MINC_LIBRARIES} )
ENDIF( MINC_LIBRARIES )
INSTALL_TARGETS(/bin MincView )
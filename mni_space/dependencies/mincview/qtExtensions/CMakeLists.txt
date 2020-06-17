PROJECT( IGNS_QT_EXTENSIONS )

SET( QT_EXTENSIONS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Include directory for qtExtensions .h files" )
MARK_AS_ADVANCED( QT_EXTENSIONS_INCLUDE_DIR )

#================================
# Define sources
#================================
SET( IGNS_QT_EXT_SRC
    qtextfiledialog.cpp
    qtextlistviewitem.cpp
   )

SET( IGNS_QT_EXT_HDR
    qtextfiledialog.h
    qtextlistviewitem.h
   )

SET( IGNS_QT_EXT_HDR_MOC 
    qtextfiledialog.h
   )

#================================
# Settings for Qt
#================================

INCLUDE (${CMAKE_ROOT}/Modules/FindQt.cmake)
IF( QT_FOUND )
  INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
ELSE( QT_FOUND )
  MESSAGE( FATAL "Qt was not found. Can't build without it" )
ENDIF( QT_FOUND )

#=======================================
# moc Qt source file without a ui file
#=======================================
IF(QT_WRAP_CPP)
  QT_WRAP_CPP( qtextensions IGNS_QT_EXT_SRC ${IGNS_QT_EXT_HDR_MOC} )
ENDIF(QT_WRAP_CPP)

INCLUDE_DIRECTORIES( ${IGNS_GLOBAL_INCLUDE_DIR})
#================================
# Define library
#================================
ADD_LIBRARY( qtextensions ${IGNS_QT_EXT_SRC} ${IGNS_QT_EXT_HDR} ${IGNS_QT_EXT_HDR_MOC}  )
INSTALL_TARGETS(/lib qtextensions)
INSTALL_FILES(/include FILES ${IGNS_QT_EXT_HDR} )

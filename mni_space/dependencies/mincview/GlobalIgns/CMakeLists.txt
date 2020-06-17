PROJECT( IGNS_GLOBAL )

SET( IGNS_GLOBAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Include directory for global .h files" )
MARK_AS_ADVANCED( IGNS_GLOBAL_INCLUDE_DIR )

#================================
# Define sources
#================================
SET( IGNS_GLOBAL_SRC
    stringtools.cpp
   )

SET( IGNS_GLOBAL_HDR
    stringtools.h
    version.h
    ignsconfig.h
    ignsmsg.h
   )

#================================
# Define library
#================================
ADD_LIBRARY( GlobalIgns  ${IGNS_GLOBAL_SRC} ${IGNS_GLOBAL_HDR}  )
INSTALL_TARGETS(/lib GlobalIgns)
INSTALL_FILES(/include FILES ${IGNS_GLOBAL_HDR} )


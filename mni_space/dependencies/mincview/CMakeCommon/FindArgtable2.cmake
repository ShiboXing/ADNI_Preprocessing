# - Find expat
# Find the native EXPAT headers and libraries.
#
#  EXPAT_INCLUDE_DIRS - where to find expat.h, etc.
#  EXPAT_LIBRARIES    - List of libraries when using expat.
#  EXPAT_FOUND        - True if expat found.

# Look for the header file.
FIND_PATH(ARGTABLE2_INCLUDE_DIR NAMES argtable2.h)

# Look for the library.
FIND_LIBRARY(ARGTABLE2_LIBRARY NAMES argtable2)

SET (ARGTABLE2_FOUND 0)
IF (ARGTABLE2_INCLUDE_DIR)
    IF (ARGTABLE2_LIBRARY)
        SET (ARGTABLE2_FOUND 1)
    ENDIF (ARGTABLE2_LIBRARY)
ENDIF (ARGTABLE2_INCLUDE_DIR)

# Copy the results to the output variables.
IF(ARGTABLE2_FOUND)
  SET(ARGTABLE2_LIBRARIES ${ARGTABLE2_LIBRARY})
  SET(ARGTABLE2_INCLUDE_DIRS ${ARGTABLE2_INCLUDE_DIR})
ELSE(ARGTABLE2_FOUND)
  SET(ARGTABLE2_LIBRARIES)
  SET(ARGTABLE2_INCLUDE_DIRS)
ENDIF(ARGTABLE2_FOUND)

MARK_AS_ADVANCED(ARGTABLE2_INCLUDE_DIR ARGTABLE2_LIBRARY)
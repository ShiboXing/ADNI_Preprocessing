# - Searches for all installed versions of MINC.
# This should only be used if your project can work with multiple
# versions of MINC.  If not, you should just directly use FindMinc1 or FindMinc2
# If multiple versions of MINC are found on the machine, then
# The user must set the option DESIRED_MINC_VERSION to the version
# they want to use.  If only one version of MINC is found on the machine,
# then the DESIRED_MINC_VERSION is set to that version and the
# matching FindMinc1 or FindMinc2 module is included.
# Once the user sets DESIRED_MINC_VERSION, then the FindMinc1 or FindMinc2 module
# is included.
#
#  MINC_REQUIRED if this is set to TRUE then if CMake can
#              not find MINC2 or MINC1 an error is raised
#              and a message is sent to the user.
#
#  DESIRED_MINC_VERSION OPTION is created
#  MINC2_INSTALLED is set to TRUE if MINC2 is found.
#  MINC1_INSTALLED is set to TRUE if MINC1 is found.


# look for signs of MINC2 installations
FILE(GLOB GLOB_TEMP_VAR /usr/local/bic/lib/libminc2.a)
IF(GLOB_TEMP_VAR)
  SET(MINC2_INSTALLED TRUE)
ENDIF(GLOB_TEMP_VAR)
SET(GLOB_TEMP_VAR)

# look for MINC1_INSTALLED installations
FILE(GLOB GLOB_TEMP_VAR /usr/local/mni/lib/libminc.a)
IF(GLOB_TEMP_VAR)
  SET(MINC1_INSTALLED TRUE)
ENDIF(GLOB_TEMP_VAR)
SET(GLOB_TEMP_VAR)

FIND_FILE( MINC2_H_FILE minc.h
  /usr/local/bic/include
  /usr/local/include
  /usr/include
  )

FIND_FILE( MINC1_H_FILE minc.h
  /usr/local/mni/include
  /usr/local/include
  /usr/include
  )
  
IF(MINC2_H_FILE)
  SET(MINC2_INSTALLED TRUE)
ENDIF(MINC2_H_FILE)

IF(MINC1_H_FILE)
  SET(MINC1_INSTALLED TRUE)
ENDIF(MINC1_H_FILE)


IF(MINC1_INSTALLED AND MINC2_INSTALLED )
  # force user to pick if we have both
  SET(DESIRED_MINC_VERSION 0 CACHE STRING "Pick a version of MINC to use: 1 or 2")
ELSE(MINC1_INSTALLED AND MINC2_INSTALLED )
  # if only one found then pick that one
  IF(MINC1_INSTALLED)
    SET(DESIRED_MINC_VERSION 1 CACHE STRING "Pick a version of MINC to use: 1 or 2")
  ENDIF(MINC1_INSTALLED)
  IF(MINC2_INSTALLED)
    SET(DESIRED_MINC_VERSION 2 CACHE STRING "Pick a version of MINC to use: 1 or 2")
  ENDIF(MINC2_INSTALLED)
ENDIF(MINC1_INSTALLED AND MINC2_INSTALLED )

IF(DESIRED_MINC_VERSION MATCHES 1)
  SET(MINC1_FIND_REQUIRED ${MINC_FIND_REQUIRED})
  SET(MINC1_FIND_QUIETLY  ${MINC_FIND_QUIETLY})
  INCLUDE(${CMAKE_COMMON_DIR}/FindMinc1.cmake)
ENDIF(DESIRED_MINC_VERSION MATCHES 1)
IF(DESIRED_MINC_VERSION MATCHES 2)
  SET(MINC2_FIND_REQUIRED ${MINC_FIND_REQUIRED})
  SET(MINC2_FIND_QUIETLY  ${MINC_FIND_QUIETLY})
  INCLUDE(${CMAKE_COMMON_DIR}/FindMinc2.cmake)
ENDIF(DESIRED_MINC_VERSION MATCHES 2)

IF(NOT MINC_FOUND)
    MESSAGE(FATAL_ERROR "CMake was unable to find MINC version: ${DESIRED_MINC_VERSION}.")
ENDIF(NOT MINC_FOUND)

IF(NOT MINC1_INSTALLED AND NOT MINC2_INSTALLED)
  IF(MINC_REQUIRED)
    MESSAGE(SEND_ERROR "CMake was unable to find any MINC versions, put minc.h in your path.")
  ENDIF(MINC_REQUIRED)
ELSE(NOT MINC1_INSTALLED AND NOT MINC2_INSTALLED)
  IF(NOT MINC_FOUND AND NOT DESIRED_MINC_VERSION)
    IF(MINC_REQUIRED)
      MESSAGE(SEND_ERROR "Multiple versions of MINC found please set DESIRED_MINC_VERSION")
    ELSE(MINC_REQUIRED)
      MESSAGE("Multiple versions of MINC found please set DESIRED_MINC_VERSION")
    ENDIF(MINC_REQUIRED)
  ENDIF(NOT MINC_FOUND AND NOT DESIRED_MINC_VERSION)
  IF(NOT MINC_FOUND AND DESIRED_MINC_VERSION)
    IF(MINC_REQUIRED)
      MESSAGE(FATAL_ERROR "CMake was unable to find MINC version: ${DESIRED_MINC_VERSION}. Set advanced values MINC_INCLUDE_DIR and MINC_LIBRARIES.")
    ELSE(MINC_REQUIRED)
      MESSAGE( "CMake was unable to find desired MINC version: ${DESIRED_MINC_VERSION}. Set advanced values MINC_INCLUDE_DIR and MINC_LIBRARIES.")
    ENDIF(MINC_REQUIRED)
  ENDIF(NOT MINC_FOUND AND DESIRED_MINC_VERSION)
ENDIF(NOT MINC1_INSTALLED AND NOT MINC2_INSTALLED)

IF( NETCDF_LIBRARIES )
    SET( MINC_LIBRARIES ${NETCDF_LIBRARIES} ${MINC_LIBRARIES} )
ENDIF( NETCDF_LIBRARIES )

#IF( VOLUME_IO_LIBRARIES AND MINC_LIBRARIES )
#    SET( MINC_LIBRARIES ${MINC_LIBRARIES} ${VOLUME_IO_LIBRARIES} )
#ENDIF( VOLUME_IO_LIBRARIES AND MINC_LIBRARIES )

IF( NETCDF_LIBRARIES AND MINC_LIBRARIES )
    SET( VOLUME_IO_LIBRARIES ${MINC_LIBRARIES} ${VOLUME_IO_LIBRARIES} )
ENDIF( NETCDF_LIBRARIES AND MINC_LIBRARIES )



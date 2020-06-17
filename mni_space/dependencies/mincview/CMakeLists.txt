CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

cmake_policy(SET CMP0003 OLD)

SET(CMAKE_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMakeCommon)
#=====================================================
# Find location to install the progs
#
# This MUST come before PROJECT or it does not work
INCLUDE(${CMAKE_COMMON_DIR}/InstallLocation.cmake)
#======================================================



PROJECT( IGNS_MINC_VIEWER )

#==================================================================
# define useful macro stolen from vtk to make an option depend on a set of others
#==================================================================
MACRO(IGNS_DEPENDENT_OPTION option doc default depends force)
  SET(${option}_AVAILABLE 1)
  FOREACH(d ${depends})
    IF(NOT ${d})
      SET(${option}_AVAILABLE 0)
    ENDIF(NOT ${d})
  ENDFOREACH(d)
  IF(${option}_AVAILABLE)
    OPTION(${option} "${doc}" "${default}")
    SET(${option} "${${option}}" CACHE BOOL "${doc}" FORCE)
  ELSE(${option}_AVAILABLE)
    SET(${option} "${${option}}" CACHE INTERNAL "${doc}")
    SET(${option} ${force})
  ENDIF(${option}_AVAILABLE)
ENDMACRO(IGNS_DEPENDENT_OPTION)


SET(MINC_REQUIRED TRUE)

#==================================================================
# Include our cmake macros
#==================================================================
INCLUDE(${CMAKE_COMMON_DIR}/FindMinc.cmake)

#==================================================================
# Include linux include directories, needed for kernel 2.6.x+
#==================================================================
INCLUDE_DIRECTORIES(/usr/src/linux/include)

#==================================================================
# Create options to build or not the different dependent projects.
#==================================================================
#OPTION( IGNS_USE_ITK "Build tools that require ITK?" OFF )
OPTION( IGNS_BUILD_VTK_MINC "Build lib vtkMinc that contains minc-vtk binding classes. Requires netcdf and minc libraries." OFF )
OPTION( IGNS_BUILD_VTK_EXTENSIONS "Build lib vtkExtensions" OFF )
OPTION( IGNS_BUILD_VTK_QT "Build lib vtkQt" OFF )
OPTION( IGNS_BUILD_QT_EXTENSIONS "Build lib qtExtensions" OFF )

IGNS_DEPENDENT_OPTION( IGNS_BUILD_BASIC "Build the library used in all applications" OFF "IGNS_BUILD_VTK_MINC;IGNS_BUILD_VTK_QT;IGNS_BUILD_QT_EXTENSIONS" OFF )
IGNS_DEPENDENT_OPTION( IGNS_BUILD_MINC_VIEW "Build the MincView application" OFF "IGNS_BUILD_BASIC" OFF )

##############################################################
# Always build global library, it contains tool that may be used anywhere
##############################################################
SUBDIRS( GlobalIgns )


IF( IGNS_BUILD_QT_EXTENSIONS )
    SUBDIRS( qtExtensions )
ENDIF(IGNS_BUILD_QT_EXTENSIONS )
SUBDIRS( vtk )
SUBDIRS( Applications )



##############################################################
# This is an uninstall script, which will use the CMake
# install_manifest file to uninstall whatever was installed by
# the install command.
#
# Be warned - if you tend to delete your build directories, you
# will, of course, delete the install_manifest.txt.  If you are
# careless about this you could end up with incomplete uninstalls.
#
#
# Uninstalling
CONFIGURE_FILE(
"${CMAKE_COMMON_DIR}/cmake_uninstall.cmake.in"
"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" @ONLY)
ADD_CUSTOM_TARGET(uninstall ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
###############################################################
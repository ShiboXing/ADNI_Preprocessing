PROJECT( IGNS_VTK_EXTENSIONS )

#========================================================
# If vtk was found, use its include directories
#========================================================
IF( VTK_FOUND )
  INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
ELSE( VTK_FOUND )
  SET (VTKEXTENSIONS_CAN_BUILD 0)
ENDIF (VTK_FOUND)


###############################################################
# This block of code sets up the wrapping for python, tcl and java

IF (USE_VTK_FILE)
  #
  # Build shared libs ?
  #
  # Defaults to the same VTK setting.
  #
  # Standard CMake option for building libraries shared or static by default.
  OPTION(BUILD_SHARED_LIBS
         "Build with shared libraries."
         ${VTK_BUILD_SHARED_LIBS})
  #
  # Output path(s)
  #
  SET (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib CACHE PATH
       "Single output directory for building all libraries.")

  SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin CACHE PATH
       "Single output directory for building all executables.")

  MARK_AS_ADVANCED (
    LIBRARY_OUTPUT_PATH
    EXECUTABLE_OUTPUT_PATH
  )


ENDIF (USE_VTK_FILE)


#
# Wrap Tcl, Java, Python
#
# Rational: even if your VTK was wrapped, it does not mean that you want to
# wrap your own local classes.
# Default value is OFF as the VTK cache might have set them to ON but
# the wrappers might not be present (or yet not found).
#
IGNS_DEPENDENT_OPTION( VTKEXTENSIONS_WRAP_TCL  "Wrap classes into the TCL interpreted language." ON "IGNS_WRAP_TCL" OFF )
IGNS_DEPENDENT_OPTION( VTKEXTENSIONS_WRAP_PYTHON  "Wrap classes into the Python interpreted language." ON "IGNS_WRAP_TCL" OFF )
IGNS_DEPENDENT_OPTION( VTKEXTENSIONS_WRAP_JAVA  "Wrap classes into the Java interpreted language." ON "IGNS_WRAP_TCL" OFF )

IF(IGNS_WRAP_TCL)
#
# Tcl
#

IF (VTK_WRAP_TCL )

  SET (VTKEXTENSIONS_WRAP_TCL ON)

ELSE (VTK_WRAP_TCL)

  IF (VTKEXTENSIONS_WRAP_TCL)
    MESSAGE("Warning. VTKEXTENSIONS_WRAP_TCL is ON but the VTK version you have "
            "chosen has not support for Tcl (VTK_WRAP_TCL is OFF).  "
            "Please set VTKEXTENSIONS_WRAP_TCL to OFF.")
    SET (VTKEXTENSIONS_WRAP_TCL OFF)
  ENDIF (VTKEXTENSIONS_WRAP_TCL)

ENDIF (VTK_WRAP_TCL)
ENDIF (IGNS_WRAP_TCL)


IF(IGNS_WRAP_PYTHON)
#
# Python
#

IF (VTK_WRAP_PYTHON)

  SET(VTKEXTENSIONS_WRAP_PYTHON  ON)

  IF (VTKEXTENSIONS_WRAP_PYTHON)
    IF (WIN32)
      IF (NOT BUILD_SHARED_LIBS)
        MESSAGE("Error. Python support requires BUILD_SHARED_LIBS to be ON.")
        SET (VTKEXTENSIONS_CAN_BUILD 0)
      ENDIF (NOT BUILD_SHARED_LIBS)
    ENDIF (WIN32)
  ENDIF (VTKEXTENSIONS_WRAP_PYTHON)

ELSE (VTK_WRAP_PYTHON)

  IF (VTKEXTENSIONS_WRAP_PYTHON)
    MESSAGE("Warning. VTKEXTENSIONS_WRAP_PYTHON is ON but the VTK version you have "
            "chosen has not support for Python (VTK_WRAP_PYTHON is OFF).  "
            "Please set VTKEXTENSIONS_WRAP_PYTHON to OFF.")
    SET (VTKEXTENSIONS_WRAP_PYTHON OFF)
  ENDIF (VTKEXTENSIONS_WRAP_PYTHON)

ENDIF (VTK_WRAP_PYTHON)
ENDIF (IGNS_WRAP_PYTHON)

IF(IGNS_WRAP_JAVA)
#
# Java
#

IF (VTK_WRAP_JAVA)

  SET(VTKEXTENSIONS_WRAP_JAVA ON)

  IF (VTKEXTENSIONS_WRAP_JAVA)
    IF (WIN32)
      IF (NOT BUILD_SHARED_LIBS)
        MESSAGE("Error. Java support requires BUILD_SHARED_LIBS to be ON.")
        SET (VTKEXTENSIONS_CAN_BUILD 0)
      ENDIF (NOT BUILD_SHARED_LIBS)
    ENDIF (WIN32)

    # Tell the java wrappers where to go.
    SET(VTK_JAVA_HOME ${CMAKE_CURRENT_BINARY_DIR}/java/vtkmy)
    MAKE_DIRECTORY(${VTK_JAVA_HOME})
  ENDIF (VTKEXTENSIONS_WRAP_JAVA)

ELSE (VTK_WRAP_JAVA)

  IF (VTKEXTENSIONS_WRAP_JAVA)
    MESSAGE("Warning. VTKEXTENSIONS_WRAP_JAVA is ON but the VTK version you have "
            "chosen has not support for Java (VTK_WRAP_JAVA is OFF).  "
            "Please set VTKEXTENSIONS_WRAP_JAVA to OFF.")
    SET (VTKEXTENSIONS_WRAP_JAVA OFF)
  ENDIF (VTKEXTENSIONS_WRAP_JAVA)

ENDIF (VTK_WRAP_JAVA)
ENDIF (IGNS_WRAP_JAVA)
# Setup our local hints file in case wrappers need them.
SET(VTK_WRAP_HINTS ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping/hints)

##################################################################



#================================
# Define sources
#================================
SET( IGNS_VTK_EXTENSIONS_SRC
    vtkNShapeCalibrationWidget.cxx
    vtkLineWithHandlesWidget.cxx
    vtkCircleWithCrossSource.cxx
    vtkUltrasoundImageStencilSource.cxx
    vtkImagePlaneActor.cxx
    vtkMultiInteractorObserver.cxx
    vtkMulti3DWidget.cxx
    vtkMultiImagePlaneWidget.cxx
    vtkPlaneAndAxisActor.cxx
    vtkPiecewiseFunctionLookupTable.cxx
    vtkProgressTerminalOutput.cxx
    vtkImageDimensionReorder.cxx
    vtkPiecewiseFunctionActor.cxx
    vtkPiecewiseFunctionWidget.cxx
    vtkColorTransferFunctionActor.cxx
    vtkColorTransferFunctionWidget.cxx
    vtkVolumeTransferFunctionWidget.cxx
    vtkGrid.cxx 
    vtkCrosshairWidget.cxx
    vtkSphereWithTextPointWidget.cxx 
    vtkListOfSphereWithTextPointWidgets.cxx )

SET( IGNS_VTK_EXTENSIONS_HDR
    vtkNShapeCalibrationWidget.h
    vtkLineWithHandlesWidget.h
    vtkCircleWithCrossSource.h
    vtkUltrasoundImageStencilSource.h
    vtkImagePlaneActor.h
    vtkMultiInteractorObserver.h
    vtkMulti3DWidget.h
    vtkMultiImagePlaneWidget.h
    vtkPlaneAndAxisActor.h
    vtkPiecewiseFunctionLookupTable.h
    vtkProgressTerminalOutput.h
    vtkImageDimensionReorder.h
    vtkPiecewiseFunctionActor.h
    vtkPiecewiseFunctionWidget.h
    vtkColorTransferFunctionActor.h
    vtkColorTransferFunctionWidget.h
    vtkVolumeTransferFunctionWidget.h
    vtkGrid.h
    vtkCrosshairWidget.h
    vtkSphereWithTextPointWidget.h
    vtkListOfSphereWithTextPointWidgets.h )


#================================
# Find minc headers
#================================
IF( MINC_INCLUDE_DIR )
    INCLUDE_DIRECTORIES( ${MINC_INCLUDE_DIR} )
ENDIF( MINC_INCLUDE_DIR )


#================================
# Define output
#================================
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../ )  # for vtkReal.h
IF (BUILD_SHARED_LIBS)
ADD_LIBRARY( vtkExtensions SHARED ${IGNS_VTK_EXTENSIONS_SRC} ${IGNS_VTK_EXTENSIONS_HDR} )
ELSE( BUILD_SHARED_LIBS )
ADD_LIBRARY( vtkExtensions ${IGNS_VTK_EXTENSIONS_SRC} ${IGNS_VTK_EXTENSIONS_HDR} )
ENDIF( BUILD_SHARED_LIBS )
INSTALL_TARGETS(/lib vtkExtensions)
INSTALL_FILES(/include FILES ${IGNS_VTK_EXTENSIONS_HDR} ${CMAKE_CURRENT_SOURCE_DIR}/../vtkReal.h ${CMAKE_CURRENT_SOURCE_DIR}/../vtkIgnsEvents.h)

#================================
# Define include dir for
# dependent projects
#================================
SET( VTK_EXTENSIONS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Include directory for vtkExtensions .h files" )
MARK_AS_ADVANCED( VTK_EXTENSIONS_INCLUDE_DIR )

#================================
# Wrapping
#================================

# Create the vtkExtensions Tcl library
IF (IGNS_WRAP_TCL AND VTK_WRAP_TCL AND VTKEXTENSIONS_WRAP_TCL)
  VTK_WRAP_TCL (vtkExtensionsTCL ExtensionsTCL_SRCS ${IGNS_VTK_EXTENSIONS_SRC})
  ADD_LIBRARY (vtkExtensionsTCL SHARED ${ExtensionsTCL_SRCS})
  TARGET_LINK_LIBRARIES (vtkExtensionsTCL vtkExtensions vtkHybridTCL minc netcdf)
  INSTALL_TARGETS(/lib vtkExtensionsTCL)
ENDIF (IGNS_WRAP_TCL AND VTK_WRAP_TCL AND VTKEXTENSIONS_WRAP_TCL)

# Create the vtkExtensions Python library.
IF (IGNS_WRAP_PYTHON AND VTK_WRAP_PYTHON AND VTKEXTENSIONS_WRAP_PYTHON)
  VTK_WRAP_PYTHON (vtkExtensionsPython ExtensionsPython_SRCS ${IGNS_VTK_EXTENSIONS_SRC})
  ADD_LIBRARY (vtkExtensionsPython MODULE ${ExtensionsPython_SRCS})
  TARGET_LINK_LIBRARIES (vtkExtensionsPython vtkCommonPython vtkExtensions vtkHybridTCL minc netcdf)
  INSTALL_TARGETS(/lib vtkExtensionsPython)
ENDIF (IGNS_WRAP_PYTHON AND VTK_WRAP_PYTHON AND VTKEXTENSIONS_WRAP_PYTHON)

# Create the vtkExtensions Java library.
IF (IGNS_WRAP_JAVA AND VTK_WRAP_JAVA AND VTKEXTENSIONS_WRAP_JAVA)
  VTK_WRAP_JAVA (vtkExtensionsJava ExtensionsJava_SRCS ${IGNS_VTK_EXTENSIONS_SRC})
  ADD_LIBRARY (vtkExtensionsJava SHARED ${ExtensionsJava_SRCS})
  TARGET_LINK_LIBRARIES (vtkExtensionsJava vtkExtensions vtkHybridTCL minc netcdf)
  INSTALL_TARGETS(/lib vtkExtensionsJava)
ENDIF (IGNS_WRAP_JAVA AND VTK_WRAP_JAVA AND VTKEXTENSIONS_WRAP_JAVA)
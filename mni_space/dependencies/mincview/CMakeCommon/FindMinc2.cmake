# Locate Minc and NetCDF include paths and libraries

# This module defines:

# MINC_INCLUDE_DIR, where to find minc.h
# MINC_LIBRARIES, the libraries to link against to use minc.
# NETCDF_INCLUDE_DIR, where to find netcdf.h
# NETCDF_LIBRARIES, the libraries to link against to use netcdf
# VOLUME_IO_INCLUDE_DIR, where to find netcdf.h
# VOLUME_IO_LIBRARIES, the libraries to link against to use netcdf

FIND_PATH( MINC_INCLUDE_DIR minc.h
           /usr/local/bic/include
            /usr/local/include
           "C:/Program Files/bic/include"
           /home/rain/igns/tools/bic/include
           "I:/tools/windowslibraries/include" )

FIND_LIBRARY( MINC_LIBRARIES minc2
              /usr/local/bic/lib
              /usr/local/lib
              "C:/Program Files/bic/include"
              /home/rain/igns/tools/bic/lib
              "I:/tools/windowslibraries/lib" )

FIND_PATH( NETCDF_INCLUDE_DIR netcdf.h
           /usr/local/bic/include
           /usr/include
           "C:/Program Files/bic/include"
           /home/rain/igns/tools/bic/include
           "I:/tools/windowslibraries/include" )

FIND_LIBRARY( NETCDF_LIBRARIES netcdf
              /usr/local/bic/lib
              /usr/lib
              "C:/Program Files/bic/include"
              /home/rain/igns/tools/bic/lib
              "I:/tools/windowslibraries/lib" )

FIND_PATH( VOLUME_IO_INCLUDE_DIR volume_io.h
           /usr/local/bic/include
           /usr/include
           "C:/Program Files/bic/include"
           /home/rain/igns/tools/bic/include
           "I:/tools/windowslibraries/include" )

FIND_LIBRARY( VOLUME_IO_LIBRARIES volume_io2
              /usr/local/bic/lib
              /usr/lib
              "C:/Program Files/bic/include"
              /home/rain/igns/tools/bic/lib
              "I:/tools/windowslibraries/lib" )

FIND_LIBRARY( Z_LIBRARY z
              /usr/local/lib
              /usr/lib )
              
FIND_LIBRARY( HDF5_LIBRARY hdf5
              /usr/local/lib
              /usr/lib )

IF( Z_LIBRARY )
    SET( MINC_LIBRARIES ${Z_LIBRARY} ${MINC_LIBRARIES} )
ELSE(Z_LIBRARY)
    MESSAGE(FATAL_ERROR "CMake was unable to find libz. Set advanced value Z_LIBRARY.")
ENDIF( Z_LIBRARY )

IF( NOT HDF5_LIBRARY )
    MESSAGE(FATAL_ERROR "CMake was unable to find libhdf5. Set advanced value HDF5_LIBRARY.")
ENDIF( NOT HDF5_LIBRARY )

IF( HDF5_LIBRARY AND MINC_LIBRARIES )
    SET( MINC_LIBRARIES ${MINC_LIBRARIES} ${HDF5_LIBRARY} )
ENDIF( HDF5_LIBRARY AND MINC_LIBRARIES )

IF(MINC_INCLUDE_DIR AND NETCDF_INCLUDE_DIR AND VOLUME_IO_INCLUDE_DIR AND MINC_LIBRARIES)
  SET( MINC_FOUND "YES" )
ENDIF(MINC_INCLUDE_DIR AND NETCDF_INCLUDE_DIR AND VOLUME_IO_INCLUDE_DIR AND MINC_LIBRARIES)

# Locate Minc and NetCDF include paths and libraries

# This module defines:

# MINC_INCLUDE_DIR, where to find minc.h
# MINC_LIBRARIES, the libraries to link against to use minc.
# NETCDF_INCLUDE_DIR, where to find netcdf.h
# NETCDF_LIBRARIES, the libraries to link against to use netcdf
# VOLUME_IO_INCLUDE_DIR, where to find netcdf.h
# VOLUME_IO_LIBRARIES, the libraries to link against to use netcdf

FIND_PATH( MINC_INCLUDE_DIR minc.h
           /usr/local/mni/include
            /usr/local/include
           "C:/Program Files/mni/include"
           /home/rain/igns/tools/bic/include
           "I:/tools/windowslibraries/include" )

FIND_LIBRARY( MINC_LIBRARIES minc
              /usr/local/mni/lib
              /usr/local/lib
              "C:/Program Files/mni/include"
              /home/rain/igns/tools/bic/lib
              "I:/tools/windowslibraries/lib" )

FIND_PATH( NETCDF_INCLUDE_DIR netcdf.h
           /usr/local/mni/include
           /usr/include
           "C:/Program Files/mni/include"
           /home/rain/igns/tools/bic/include
           "I:/tools/windowslibraries/include" )

FIND_LIBRARY( NETCDF_LIBRARIES netcdf
              /usr/local/mni/lib
              /usr/lib
              "C:/Program Files/mni/include"
              /home/rain/igns/tools/bic/lib
              "I:/tools/windowslibraries/lib" )

FIND_PATH( VOLUME_IO_INCLUDE_DIR volume_io.h
           /usr/local/mni/include
           /usr/include
           "C:/Program Files/mni/include"
           /home/rain/igns/tools/bic/include
           "I:/tools/windowslibraries/include" )

FIND_LIBRARY( VOLUME_IO_LIBRARIES volume_io
              /usr/local/mni/lib
              /usr/lib
              "C:/Program Files/mni/include"
              /home/rain/igns/tools/bic/lib
              "I:/tools/windowslibraries/lib" )


IF(MINC_INCLUDE_DIR AND NETCDF_INCLUDE_DIR AND VOLUME_IO_INCLUDE_DIR AND MINC_LIBRARIES)
  SET( MINC_FOUND "YES" )
ENDIF(MINC_INCLUDE_DIR AND NETCDF_INCLUDE_DIR AND VOLUME_IO_INCLUDE_DIR AND MINC_LIBRARIES)

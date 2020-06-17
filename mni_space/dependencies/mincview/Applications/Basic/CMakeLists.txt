PROJECT( IGNS_BASIC )

#================================
# Define source files
#================================

SET( IGNS_BASIC_SRC
                     scenemanager.cpp
                     view.cpp
                     sceneobject.cpp
                     imageobject.cpp
                     pickrefiner.cpp
                     mincwithobj.cpp
                     generatedsurface.cpp
                     pickcommand.cpp
                     movepointcommand.cpp
                     pickerobject.cpp
                     regridvolume.cpp
                     MINCReader.cpp
                     imagetracer.cpp
                     volume.cpp
                     reslicer.cpp
                     polydataobject.cpp
                     pointsobject.cpp
                     editvolumetransferfunctionwidget.cpp
                     usvolumereconstructionsettings.cpp
                     doublewindowfileviewer.cpp
                     usertransformations.cpp
                     usacquisitioninfo.cpp
                     usacquisitioninfo2attrib.cpp
                     attrib2usacquisitioninfo.cpp
                     volumecreator.cpp
                     volumereconstruction.cpp
                     expertmode.cpp
                     gui/viewmenufunctions.cpp
                     gui/settingsmenufunctions.cpp
                     gui/importmenufunctions.cpp
                     gui/toolsmenufunctions.cpp
                     gui/aboutbicigns.cpp
                     gui/doubleviewwindow.cpp
                     gui/hardwaresettings.cpp
                     gui/quadviewwindow.cpp
                     gui/objectlistdialog.cpp
                     gui/simpleobjectlistdialog.cpp
                     gui/imageobjectsettingsdialog.cpp
                     gui/polydataobjectsettingsdialog.cpp
                     gui/pointsobjectsettingsdialog.cpp
                     gui/clearselectedpointdialog.cpp
                     gui/removepointdialog.cpp
                     gui/pickingdialog.cpp
                     gui/pickerobjectsettingsdialog.cpp
                     gui/yesnodialog.cpp
                     gui/editpointcoordinates.cpp
                     gui/scenemanagersettingsdialog.cpp
                     gui/multiplefileviewerdialog.cpp
                     gui/edittransformdialog.cpp
                     gui/opendatafiledialog.cpp
                     gui/transformlistdialog.cpp
                     gui/simplevolumizedialog.cpp
                     gui/mincinfowidget.cpp
                     gui/passworddialog.cpp
                     gui/setopacitydialog.cpp
                     gui/savepointdialog.cpp
                     gui/pointlistdialog.cpp
                     gui/maskdialog.cpp
                     gui/zoomfactordialog.cpp )

SET( IGNS_BASIC_HDR
                     spectrumicons.h
                     generatedsurface.h
                     pickcommand.h
                     movepointcommand.h
                     mincwithobj.h
                     pickerobject.h
                     regridvolume.h
                     MINCReader.h
                     pickrefiner.h
                     imagetracer.h
                     volume.h
                     reslicer.h
                     volumecreator.h
                     volumereconstruction.h
                     usvolumereconstructionsettings.h
                     doublewindowfileviewer.h
                     usertransformations.h
                     usacquisitioninfo.h
                     usacquisitioninfo2attrib.h
                     attrib2usacquisitioninfo.h
                     expertmode.h
                     gui/viewmenufunctions.h
                     gui/settingsmenufunctions.h
                     gui/importmenufunctions.h
                     gui/toolsmenufunctions.h
                     gui/aboutbicigns.h
                     gui/doubleviewwindow.h
                     gui/hardwaresettings.h
                     gui/quadviewwindow.h
                     gui/objectlistdialog.h
                     gui/simpleobjectlistdialog.h
                     gui/imageobjectsettingsdialog.h
                     gui/polydataobjectsettingsdialog.h
                     gui/pointsobjectsettingsdialog.h
                     gui/clearselectedpointdialog.h
                     gui/editpointcoordinates.h
                     gui/removepointdialog.h
                     gui/pickingdialog.h
                     gui/yesnodialog.h
                     gui/pickerobjectsettingsdialog.h
                     gui/scenemanagersettingsdialog.h
                     gui/multiplefileviewerdialog.h
                     gui/edittransformdialog.h
                     gui/opendatafiledialog.h
                     gui/transformlistdialog.h
                     gui/simplevolumizedialog.h
                     gui/mincinfowidget.h
                     gui/passworddialog.h
                     gui/setopacitydialog.h
                     gui/savepointdialog.h
                     gui/pointlistdialog.h
                     gui/maskdialog.h
                     gui/zoomfactordialog.h )

SET( IGNS_BASIC_HDR_MOC
                         editvolumetransferfunctionwidget.h
                         view.h
                         scenemanager.h
                         sceneobject.h
                         imageobject.h
                         imagetracer.h
                         polydataobject.h
                         pointsobject.h
                         pickcommand.h
                         pickerobject.h
                         usvolumereconstructionsettings.h
                         doublewindowfileviewer.h
                         usertransformations.h
                         usacquisitioninfo.h
                         usacquisitioninfo2attrib.h
                         attrib2usacquisitioninfo.h
                         gui/viewmenufunctions.h
                         gui/settingsmenufunctions.h
                         gui/importmenufunctions.h
                         gui/toolsmenufunctions.h
                         gui/aboutbicigns.h
                         gui/doubleviewwindow.h
                         gui/hardwaresettings.h
                         gui/quadviewwindow.h
                         gui/objectlistdialog.h
                         gui/simpleobjectlistdialog.h
                         gui/imageobjectsettingsdialog.h
                         gui/polydataobjectsettingsdialog.h
                         gui/clearselectedpointdialog.h
                         gui/removepointdialog.h
                         gui/pickingdialog.h
                         gui/pickerobjectsettingsdialog.h
                         gui/pointsobjectsettingsdialog.h
                         gui/yesnodialog.h
                         gui/editpointcoordinates.h
                         gui/scenemanagersettingsdialog.h
                         gui/multiplefileviewerdialog.h
                         gui/edittransformdialog.h
                         gui/opendatafiledialog.h
                         gui/transformlistdialog.h
                         gui/simplevolumizedialog.h
                         gui/mincinfowidget.h
                         gui/passworddialog.h
                         gui/setopacitydialog.h
                         gui/savepointdialog.h
                         gui/pointlistdialog.h
                         gui/maskdialog.h
                         gui/zoomfactordialog.h )

SET( IGNS_BASIC_GUI_SRC
                         gui/hardwaresettingsbase.ui
                         gui/imageobjectsettingsdialogbase.ui
                         gui/doubleviewwindowbase.ui
                         gui/multiplefileviewerdialogbase.ui
                         gui/edittransformdialogbase.ui
                         gui/pickingdialogbase.ui
                         gui/objectlistdialogbase.ui
                         gui/opendatafiledialogbase.ui
                         gui/polydataobjectsettingsdialogbase.ui
                         gui/pointsobjectsettingsdialogbase.ui
                         gui/pointlistdialogbase.ui
                         gui/transformlistdialogbase.ui
                         gui/zoomfactordialogbase.ui
                         gui/volumizedialogbase.ui
                         gui/mincinfowidgetbase.ui
                         gui/passworddialogbase.ui
                         gui/setopacitydialogbase.ui
                         gui/savepointdialogbase.ui
                         gui/clearselectedpointdialogbase.ui
                         gui/removepointdialogbase.ui
                         gui/maskdialogbase.ui
                         gui/aboutbicignsbase.ui )


#================================
# Find vtk
#================================

INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)
IF( VTK_FOUND )
  INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
ENDIF( VTK_FOUND )

#================================
# Find Qt
#================================

INCLUDE (${CMAKE_ROOT}/Modules/FindQt.cmake)
IF( QT_FOUND )
  INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
  ADD_DEFINITIONS(-DQT_THREAD_SUPPORT ${QT_DEFINITIONS} )  # we want to make sure we use multithreaded qt
ELSE( QT_FOUND )
  MESSAGE( FATAL "Qt was not found. Can't build without it" )
ENDIF( QT_FOUND )

#=======================================
# Convert Qt .ui files to source files
#=======================================
#2007-11-28 - in cmake 2.4 patch 4 and higher the IF fails, 2008-08-12 in cmake 2.6 works again
IF(QT_WRAP_UI)
  QT_WRAP_UI( Basic IGNS_BASIC_HDR IGNS_BASIC_SRC ${IGNS_BASIC_GUI_SRC} )
ENDIF(QT_WRAP_UI)

#=======================================
# moc Qt source file without a ui file
#=======================================
IF(QT_WRAP_CPP)
  QT_WRAP_CPP( Basic IGNS_BASIC_SRC ${IGNS_BASIC_HDR_MOC} )
ENDIF(QT_WRAP_CPP)

#================================
# use GlobalIgns
#================================
IF( IGNS_GLOBAL_INCLUDE_DIR )
    INCLUDE_DIRECTORIES( ${IGNS_GLOBAL_INCLUDE_DIR} )
ENDIF( IGNS_GLOBAL_INCLUDE_DIR )

#================================
# use qtExtensions
#================================
IF( QT_EXTENSIONS_INCLUDE_DIR )
    INCLUDE_DIRECTORIES( ${QT_EXTENSIONS_INCLUDE_DIR} )
ENDIF( QT_EXTENSIONS_INCLUDE_DIR )

#================================
# use vtkQt
#================================

IF(IGNS_VTK_QT_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${IGNS_VTK_QT_INCLUDE_DIR} )
ENDIF(IGNS_VTK_QT_INCLUDE_DIR)

#================================
# use vtkTracker
#================================
IF(VTK_TRACKER_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${VTK_TRACKER_INCLUDE_DIR} )
ENDIF(VTK_TRACKER_INCLUDE_DIR)

#================================
# use vtkExtensions
#================================
IF(VTK_EXTENSIONS_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${VTK_EXTENSIONS_INCLUDE_DIR} )
ENDIF(VTK_EXTENSIONS_INCLUDE_DIR)

#================================
# use vtkMNI
#================================
IF(VTK_MNI_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${VTK_MNI_INCLUDE_DIR} )
ENDIF(VTK_MNI_INCLUDE_DIR)

#================================
# use vtkMinc
#================================
IF(VTK_MINC_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${VTK_MINC_INCLUDE_DIR} )
ENDIF(VTK_MINC_INCLUDE_DIR)

#================================
# use IGSTK
#================================
IF(IGSTK_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${IGSTK_INCLUDE_DIR} )
ENDIF(IGSTK_INCLUDE_DIR)

#================================
# use minc
#================================
IF( MINC_INCLUDE_DIR )
    INCLUDE_DIRECTORIES( ${MINC_INCLUDE_DIR} )
ENDIF( MINC_INCLUDE_DIR )


#================================
# Define output
#================================
INCLUDE_DIRECTORIES( ${IGNS_BASIC_SOURCE_DIR} )           # so that non-generated gui files can include main files
INCLUDE_DIRECTORIES( ${IGNS_BASIC_SOURCE_DIR}/gui )       # for non-generated gui .h files
INCLUDE_DIRECTORIES( ${IGNS_BASIC_SOURCE_DIR}/../../vtk ) # for vtkReal.h
INCLUDE_DIRECTORIES( ${IGNS_BASIC_BINARY_DIR} )           # for files generated by Qt
ADD_LIBRARY( Basic ${IGNS_BASIC_SRC} ${IGNS_BASIC_HDR} ${IGNS_BASIC_HDR_MOC} ${IGNS_BASIC_GUI_UI_H})


#================================
# Define include dir for
# dependent projects
#================================
SET( BASIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Include directory for Basic .h files" )
MARK_AS_ADVANCED( BASIC_INCLUDE_DIR )

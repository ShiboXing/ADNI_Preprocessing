/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: multiplefileviewerdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-04-08 14:56:03 $
  Version:   $Revision: 1.12 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef TAG_MULTIPLEFILEVIEWERDIALOG_H
#define TAG_MULTIPLEFILEVIEWERDIALOG_H

#include "multiplefileviewerdialogbase.h"
#include <qstring.h>
#include <vector>
#include "vtkObject.h"

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
// IGSTK headers
#include "igstkObject.h"
#include "igstkMacros.h"
#endif

class DoubleWindowFileViewer;
class vtkTransform;
class vtkMatrix4x4;
class vtkImageData;
class vtkLookupTable;
class SceneManager;
class QTimer;
class vtkMINCImageAttributes2;
class USVolumeReconstructionSettings;

//time in seconds for "slide show"
#define SHORTEST_TIME_INTERVAL 1
#define DEFAULT_TIME_INTERVAL 1
#define LONGEST_TIME_INTERVAL  30
#define DISCARDED_ACQUISITIONS_DIR "deletedXXXXXX"

class AcquisitionData:public vtkObject
{
public:
    static AcquisitionData * New() { return new AcquisitionData; }
    
    void SetImage( vtkImageData * image );
    vtkImageData* GetImage( ) {return m_imageData;}
    void SetTransform( vtkMatrix4x4 * transform );
    vtkTransform * GetTransform() {return m_transform;}
    QString GetFileName() {return m_fileName;}
    void SetFileName(QString name) {m_fileName = name;}
    void SetStartCoordinates(double x, double y, double z);
    void GetStartCoordinates(double *x, double *y, double *z);
    void SetStep(double x, double y, double z);
    void GetStep(double *x, double *y, double *z);
    void SetDirectionCosines(vtkMatrix4x4 *dc);
    vtkMatrix4x4 *GetDirectionCosines();

protected:
    vtkImageData *m_imageData;
    vtkTransform *m_transform;
    QString m_fileName;
    
    double m_startX, m_startY, m_startZ;
    double m_stepX, m_stepY, m_stepZ;
    vtkMatrix4x4 *m_directionCosines;
    
 private:
    AcquisitionData();
    ~AcquisitionData();
};

class MultipleFileViewerDialog : public MultipleFileViewerDialogBase
{
    Q_OBJECT

public:
    MultipleFileViewerDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~MultipleFileViewerDialog();

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()
#endif

    virtual void SetViewer(DoubleWindowFileViewer *viewer);
    virtual void SetLastVisitedDirectory(const QString & dir);
    virtual QString GetLastVisitedDirectory() {return m_lastVisitedDirectory;}
    virtual void SetSceneManager( SceneManager *manager ); 
    virtual void Initialize();
    
public slots:
    virtual void LoadFilesButtonClicked();
    virtual void LoadVolumeButtonClicked();
    virtual void DeleteCurrentButtonClicked();
    virtual void FirstButtonClicked();
    virtual void PreviousButtonClicked();
    virtual void FrameSliderValueChanged(int);
    virtual void TimeComboBoxActivated(int);
    virtual void StartButtonClicked();
    virtual void SetTransformButtonClicked();
    virtual void NextButtonClicked();
    virtual void LastButtonClicked();
    virtual void LeftWindowSliderValueChanged(int);
    virtual void LeftLevelSliderValueChanged(int);
    virtual void LeftColorComboBoxActivated(int);
    virtual void RightWindowSliderValueChanged(int);
    virtual void RightLevelSliderValueChanged(int);
    virtual void RightColorComboBoxActivated(int);
    virtual void SetNextImage();
    virtual void ShowMincInfo();
    virtual void SetMaskButtonClicked();

protected:
    DoubleWindowFileViewer * m_viewer;
    USVolumeReconstructionSettings *m_usParamsFromFile;
    QString m_lastVisitedDirectory; 
    SceneManager *m_manager;
    QString m_discardedFilesDirectory;
    vtkLookupTable *m_rightWindowLut;
    vtkLookupTable *m_leftWindowLut;

    typedef std::vector<AcquisitionData*> AcquisitionDataList;
    AcquisitionDataList m_acquisition;
    
    QTimer *m_timer;
    int m_acquisitionImageCounter; 
    int m_current_index;
    int m_timeInterval;
    bool m_slideShowOn;
    
    void CleanAcquisition();
    void SetButtonsEnabled(bool);
};

#endif // TAG_MULTIPLEFILEVIEWERDIALOG_H

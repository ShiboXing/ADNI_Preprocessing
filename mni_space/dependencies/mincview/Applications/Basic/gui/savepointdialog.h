/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: savepointdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-03-03 19:25:59 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef SAVEPOINTDIALOG_H
#define SAVEPOINTDIALOG_H

#include "savepointdialogbase.h"
#include <vector>
#include <qstring.h>
#include <qdatetime.h>

class vtkPoints;
class SceneManager;
class PointsObject;

class SavePointDialog : public SavePointDialogBase
{
    Q_OBJECT

public:
    SavePointDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~SavePointDialog();
    
    void SetSaveDirectory(const QString &dir);
    void AddPoint(double x, double y, double z);
    void AddPoint(double xyz[3]);
    void SetPointNames(std::vector<std::string> & pointNames);
    void SetVolumeNames(std::vector<std::string> & volumeNames);
    void SetCurrentPointCoords(double xyz[3]);
    void SetManager( SceneManager *manager ); 

public slots:
    virtual void OkButtonClicked();
    virtual void BrowsePushButtonClicked();
    virtual void FileNameEntered();

protected:
    QString m_directory;
    QString m_outputFile;
    QString m_displayPointObjectName;
    SceneManager *m_manager;
    double m_currentPointCoords[3];
    vtkPoints * m_points;
    QDateTime m_saveTime;
    PointsObject *m_pointsObject;
    std::vector<std::string> m_pointNames;
    std::vector<std::string> m_volumeNames;
};

#endif // SAVEPOINTDIALOG_H

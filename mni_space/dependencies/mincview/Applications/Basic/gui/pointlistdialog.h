/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: pointlistdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-04-28 19:05:52 $
  Version:   $Revision: 1.3 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef POINTLISTDIALOG_H
#define POINTLISTDIALOG_H

#include "pointlistdialogbase.h"
#include "serializer.h"

class PointsObject;

// Structure that holds general settings for this dialog.
struct PointListDialogSettings
{
    PointListDialogSettings();
    ~PointListDialogSettings();
    void Serialize( Serializer * serializer );

    QString m_lastVisitedDirectory;
};

ObjectSerializationHeaderMacro(PointListDialogSettings);

class PointListDialog : public PointListDialogBase
{
    Q_OBJECT

public:
    PointListDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PointListDialog();

public slots:
    virtual void ClosePushButtonClicked();
    virtual void SavePushButtonClicked();
    virtual void ClearSelectedPushButtonClicked();
    virtual void ClearAllPushButtonClicked();
    virtual void PointFromListSelected(QListViewItem*);
    virtual void HighlightPoint(int index);

    PointsObject *GetPointList() {return m_points;}
    void SetPoints(PointsObject *pts);
    void UpdateUI();

signals:
    void PointListClosed();
    void NumberOfPointsChanged();
    
protected:

    PointsObject *m_points;
    int m_current_point_index;

private:
    PointListDialogSettings * m_settings;
    void closeEvent(QCloseEvent *event);
};

#endif // POINTLISTDIALOG_H

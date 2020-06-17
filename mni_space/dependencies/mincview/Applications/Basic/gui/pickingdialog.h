/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: pickingdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.13 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#ifndef TAG_PICKINGDIALOG_H
#define TAG_PICKINGDIALOG_H
#include "pickingdialogbase.h"
#include "vtkReal.h"
#include "serializer.h"
#include "pickcommand.h"
#include "pickerobject.h"

// Structure that holds general settings for this dialog.
struct PickingDialogSettings
{
    PickingDialogSettings();
    ~PickingDialogSettings();
    void Serialize( Serializer * serializer );

    QString LastVisitedDirectory;
};

ObjectSerializationHeaderMacro( PickingDialogSettings );

class PickingDialog : public PickingDialogBase
{
    Q_OBJECT

public:
    PickingDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~PickingDialog();
    
    virtual void SetSceneManager( SceneManager * man );
    virtual void Init();
    
public slots:
    virtual void OkPushButtonClicked();
    virtual void CancelPushButtonClicked();
    virtual void SavePushButtonClicked();
    virtual void SelectObjectButtonGroupClicked(int);
    virtual void ClearSelectedPushButtonClicked();
    virtual void ClearAllPushButtonClicked();
    virtual void LoadTagsPushButtonClicked();
    virtual void EditPoint(QListViewItem*);
    virtual void PointFromListSelected(QListViewItem*);
    virtual void TagSizeSpinBoxValueChanged(int);
    
    virtual void UpdatePointList();
    virtual void AddNewPoint(POINT_PROPERTIES* point);
    virtual void MovePlainsToPoint(POINT_PROPERTIES* point);
    virtual void MovePoint(POINT_PROPERTIES* point);
    virtual void SelectPoint(POINT_PROPERTIES* point);
    virtual void SelectPoint(int index);
    virtual void HighlightPoint(int index);

protected:

    SceneManager * manager;
    int currentPointIndex;

private:
    
    void ReadConfig();
    void WriteConfig();
    
    PickerObject *pickObj;
    PickingDialogSettings * settings;
};

#endif // TAG_PICKINGDIALOG_H

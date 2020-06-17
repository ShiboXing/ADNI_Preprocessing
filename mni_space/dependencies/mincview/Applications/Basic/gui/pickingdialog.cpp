/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: pickingdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-05-03 14:25:32 $
  Version:   $Revision: 1.32 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "pickingdialog.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qdir.h>
#include <qfiledialog.h>

#include "vtkMath.h"

#include "scenemanager.h"
#include "polydataobject.h"
#include "imageobject.h" 
#include "mincwithobj.h"
#include "pickerobject.h"
#include "yesnodialog.h"
#include "clearselectedpointdialog.h"
#include "editpointcoordinates.h"
#include "ignsconfig.h"
#include "ignsmsg.h"
#include "view.h"
#include "pointsobject.h"

ObjectSerializationMacro( PickingDialogSettings );

PickingDialogSettings::PickingDialogSettings() 
{
    this->LastVisitedDirectory = (const char*)(QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY);
}

PickingDialogSettings::~PickingDialogSettings() 
{
}

void PickingDialogSettings::Serialize( Serializer * serializer )
{
    ::Serialize( serializer, "LastVisitedDirectory", LastVisitedDirectory );
}

PickingDialog::PickingDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : PickingDialogBase( parent, name, modal, fl )
{
    if ( !name )
    setName( "PickingDialog" );
    
    pointListView->setColumnText( 0, tr( "No." ) );
    pointListView->setColumnWidth( 0, 30 );
    pointListView->addColumn( tr( "Coordinates (x,y,z)" ), 215 );
    pointListView->addColumn( tr( "Distance" ), 65 );
    pointListView->addColumn( tr( "Angle" ), 65 );
    pointListView->setSorting( -1, FALSE );
    pointListView->setAllColumnsShowFocus( TRUE );
    pointListView->clear();
    
    tagSizeSpinBox->blockSignals(TRUE);
    tagSizeSpinBox->setMinValue(1);
    tagSizeSpinBox->setMaxValue(10);
    tagSizeSpinBox->blockSignals(FALSE);
    
    this->manager = 0;
    this->currentPointIndex = -1;
    this->pickObj = 0;
    this->settings = new PickingDialogSettings;
    this->ReadConfig();
}

PickingDialog::~PickingDialog()
{
    delete this->settings;
    if( this->manager )
        this->manager->UnRegister( 0 );
}


void PickingDialog::OkPushButtonClicked()
{
    QMessageBox::warning( this, "Warning: ", "Picking will be disabled.", 1, 0 );
    if (this->pickObj)
    {
        this->pickObj->RestoreMouseActions();
        this->pickObj->DeInit();
        this->pickObj->HidePointCoordinatesIn3DView();
        this->pickObj->EnableInteraction(0);
    }
    accept();
}

void PickingDialog::CancelPushButtonClicked()
{
    if (this->pickObj)
    {
        this->pickObj->UndoPicking();
        this->pickObj->DisplayPoints();
        this->pickObj->RemoveCrosshairFromViews();
        this->pickObj->RestoreMouseActions();
        this->pickObj->HidePointCoordinatesIn3DView();
        this->pickObj->EnableInteraction(0);
    }
    reject();
}

void PickingDialog::SavePushButtonClicked()
{
    int n = 0;
    if (this->pickObj)
    {
        QString tag_directory = settings->LastVisitedDirectory;
        if( !QFile::exists( tag_directory ) )
        {
            tag_directory = QDir::homeDirPath() + IGNS_CONFIGURATION_SUBDIRECTORY;
            QDir configDir( tag_directory );
            if( !configDir.exists( ) )
            {
                configDir.mkdir( tag_directory );
            }
        }
        QString filename = QFileDialog::getSaveFileName( tag_directory, "Tag file (*.tag)", 0, "open", "Open tag file" );
        if (QFile::exists(filename))
        {
            int ret = QMessageBox::warning(this, tr("Picking"),
                                           tr("File %1 already exists.\n"
                                                   "Do you want to overwrite it?")
                                                   .arg(QDir::convertSeparators(filename)),
                                           QMessageBox::Yes | QMessageBox::Default,
                                           QMessageBox::No | QMessageBox::Escape);
            if (ret == QMessageBox::No)
                return;
        }
        if( !filename.isEmpty() )
        {
            QFileInfo info( filename );
            QString dirPath = (const char*)info.dirPath( TRUE );
            QFileInfo info1( dirPath );
            if (!info1.isWritable())
            {
                QString accessError = IGNS_MSG_NO_WRITE + dirPath;
                QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
                return;
            }
            this->settings->LastVisitedDirectory = dirPath;
            this->WriteConfig();
        }
        n = this->pickObj->SavePickedPoints(filename);
        if (n == 0)
           QMessageBox::warning( this, "Error: ", "There are no picked points.", 1, 0 );
    }
}

void PickingDialog::SelectObjectButtonGroupClicked(int index)
{
    SceneObject *obj =  this->manager->GetCurrentObject();
    if (!obj)
    { 
        QMessageBox::warning( 0, "Error: ", "Please select an object to pick on.", 1, 0 );
        surfaceRadioButton->setEnabled(TRUE);
        surfaceRadioButton->setChecked(FALSE);
        objectRadioButton->setEnabled(TRUE);
        objectRadioButton->setChecked(FALSE);
        return;
    }
        
    switch (index)
    {
        default:
        case 0:
            if (obj->GetObjectType() & PICKER_TYPE)
            {
                if (this->pickObj && (this->pickObj->GetPickerType() == SURFACE))
                {
                    obj = this->pickObj->GetImageObj();
                }
                else
                {
                    obj = 0;
                }
            }
            else
            {
                this->manager->DestroyPickerObject();
                this->pickObj = 0;
                this->pointListView->clear();
            }
            if (! obj || obj->GetObjectManagedBySystem() || !(obj->GetObjectType() & MINC_TYPE))
            {
                obj =  this->manager->GetUniqueTypedObject(MINC_TYPE);
            }
            if (obj)
            {    
                ImageObject * iObj = reinterpret_cast<ImageObject *>(obj);
                if (this->pickObj)
                {
                    if (!this->pickObj->Init())
                    {
                        QMessageBox::warning( 0, "Error: ", "Please, generate first a surface to pick on.", 1, 0 );
                        surfaceRadioButton->setEnabled(TRUE);
                        surfaceRadioButton->setChecked(FALSE);
                        objectRadioButton->setEnabled(TRUE);
                        objectRadioButton->setChecked(FALSE);
                        break;
                    }
                    this->UpdatePointList();
                }
                else
                {
                    if (!iObj->GetViewSurface())
                    {
                        QMessageBox::warning( 0, "Error: ", "Please, generate first a surface to pick on.", 1, 0 );
                        surfaceRadioButton->setEnabled(TRUE);
                        surfaceRadioButton->setChecked(FALSE);
                        objectRadioButton->setEnabled(TRUE);
                        objectRadioButton->setChecked(FALSE);
                        return;
                    }
                    this->pickObj = this->manager->MakePickerObject(iObj);
                    if (!this->pickObj)
                        return;
                    connect( this->pickObj, SIGNAL( UpdatePoints() ), this, SLOT( UpdatePointList() ) );
                    connect( this->pickObj, SIGNAL( PointSelected(POINT_PROPERTIES*) ), this, SLOT( SelectPoint(POINT_PROPERTIES*) ) );
                    connect( this->pickObj, SIGNAL( PointAdded(POINT_PROPERTIES*) ), this, SLOT( AddNewPoint(POINT_PROPERTIES*) ) );
                    connect( this->pickObj, SIGNAL( PointMoved(POINT_PROPERTIES*) ), this, SLOT( MovePoint(POINT_PROPERTIES*) ) );
                    surfaceRadioButton->setEnabled(FALSE);
                }
                iObj->SetView3DPlanes(0);
                iObj->EmitHidePlaneSignal(THREED_VIEW_TYPE, 0);
            }
            else 
            { 
                QMessageBox::warning( 0, "Error: ", "Please select an object to pick on.", 1, 0 );
                surfaceRadioButton->setEnabled(TRUE);
                surfaceRadioButton->setChecked(FALSE);
                objectRadioButton->setEnabled(TRUE);
                objectRadioButton->setChecked(FALSE);
            }
            break;
        case 1:
            if (obj->GetObjectType() & PICKER_TYPE)
            {
                if (this->pickObj && this->pickObj->GetPickerType() == OBJ)
                {
                    obj = this->pickObj->GetPolyDataObj();
                }
                else
                {
                    obj = 0;
                }
            }
            else if (this->pickObj)
            {
                this->manager->DestroyPickerObject();
                this->pickObj = 0;
                this->pointListView->clear();
            }
    
            if (! obj || obj->GetObjectManagedBySystem() || !(obj->GetObjectType() & OBJ_TYPE))
            {
                obj =  this->manager->GetUniqueTypedObject(OBJ_TYPE);
            }

            if (obj)
            {    
                PolyDataObject * pObj = reinterpret_cast<PolyDataObject *>(obj); 
                if (this->pickObj)
                {
                    if (!this->pickObj->Init())
                    {
                        QMessageBox::warning( 0, "Error: ", "Please select an object to pick on.", 1, 0 );
                        surfaceRadioButton->setEnabled(TRUE);
                        objectRadioButton->setEnabled(TRUE);
                        objectRadioButton->setChecked(FALSE);
                        break;
                    }
                    this->UpdatePointList();
                }
                else
                {
                    this->pickObj = this->manager->MakePickerObject(pObj);
                    if (!this->pickObj)
                        return;
                    connect( this->pickObj, SIGNAL( UpdatePoints() ), this, SLOT( UpdatePointList() ) );
                    connect( this->pickObj, SIGNAL( PointSelected(POINT_PROPERTIES*) ), this, SLOT( SelectPoint(POINT_PROPERTIES*) ) );
                    connect( this->pickObj, SIGNAL( PointAdded(POINT_PROPERTIES*) ), this, SLOT( AddNewPoint(POINT_PROPERTIES*) ) );
                    connect( this->pickObj, SIGNAL( PointMoved(POINT_PROPERTIES*) ), this, SLOT( MovePoint(POINT_PROPERTIES*) ) );
                    objectRadioButton->setEnabled(FALSE);
                }
                if (this->pickObj)
                {
                    MincWithObj* mObj = this->manager->GetMincAndObj();
                    if (mObj)
                    {
                        ImageObject *iObj = mObj->GetImageObj();
                        if (iObj)
                        {
                            this->pickObj->SetImageObject(iObj);
                            iObj->SetView3DPlanes(0);
                            iObj->EmitHidePlaneSignal(THREED_VIEW_TYPE, 0);
                        }
                    }
                }
            }
            else 
            { 
                QMessageBox::warning( 0, "Error: ", "Please select an object to pick on.", 1, 0 );
                surfaceRadioButton->setEnabled(TRUE);
                objectRadioButton->setEnabled(TRUE);
                objectRadioButton->setChecked(FALSE);
            }
            break;
    }
}

void PickingDialog::TagSizeSpinBoxValueChanged(int newSize)
{
    if (this->pickObj)
        this->pickObj->SetPointRadius((double)newSize);
}

void PickingDialog::ClearSelectedPushButtonClicked()
{
    int numPoints = 0;
    QListViewItem *item = this->pointListView->selectedItem();
    if (this->pickObj)
    {
        numPoints = this->pickObj->GetNumberOfPoints();
        if (numPoints > 0)
        {    
            ClearSelectedPointDialog *dlg = new ClearSelectedPointDialog(this, "ClearSelectedPointDialog", TRUE);
            dlg->SetPointsObject(this->pickObj->GetSelectedPoints());
            if (item)
                dlg->SetSelectedPointNumber(item->text(0));
            dlg->show();
        }
        else
           QMessageBox::warning( this, "Warning", "There are no picked points.", 1, 0 );
    }
}

void PickingDialog::ClearAllPushButtonClicked()
{
    if (this->pickObj)
    {
        if (this->pickObj->GetNumberOfPoints() > 0 )
        {
            YesNoDialog *dlg = new YesNoDialog(0, "removeAllPoints", TRUE, Qt::WDestructiveClose  );
            dlg->SetQuestion("Remove all picked points?");
            if (dlg->exec() == QDialog::Accepted)
            {
                this->pickObj->RemoveAllPoints();
                this->pickObj->HidePointCoordinatesIn3DView();
                this->pickObj->RemoveCrosshairFromViews();
            }
        }
        this->UpdatePointList();
    }
}

void PickingDialog::LoadTagsPushButtonClicked()
{
    if (this->pickObj)
    {
        if (this->pickObj->GetNumberOfPoints() > 0 )
        {
            YesNoDialog *dlg = new YesNoDialog(0, "removeAllPoints", TRUE, Qt::WDestructiveClose );
            dlg->SetQuestion("Remove all picked points?");
            if (dlg->exec() == QDialog::Rejected)
                return;
        }
        this->pointListView->clear();
        this->pickObj->RemoveAllPoints();
        this->pickObj->HidePointCoordinatesIn3DView();
        QString tag_directory = settings->LastVisitedDirectory;
        if( !QFile::exists( tag_directory ) )
        {
            tag_directory = QDir::homeDirPath() + IGNS_CONFIGURATION_SUBDIRECTORY;
        }
        if( !QFile::exists( tag_directory ) )
            tag_directory = QDir::homeDirPath();
        QString filename = QFileDialog::getOpenFileName( tag_directory, "Tag file (*.tag)", 0 , "open", "Open tag file" ); 
        if( !filename.isNull() && this->pickObj->GetPickerType() != UNDEFINED)
        {
            QFileInfo info( filename );
            if (!info.exists()) 
            {
                QString accessError = IGNS_MSG_FILE_NOT_EXISTS + filename;
                QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
                return;
            }
            if (!info.isReadable()) 
            {
                QString accessError = IGNS_MSG_NO_READ + filename;
                QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
                return;
            }
            this->settings->LastVisitedDirectory = (const char*)info.dirPath( TRUE );
            this->WriteConfig();
            if (!this->pickObj->LoadPointsFromTagFile(filename))
            {
                QMessageBox::warning( this, "Warning", "Invalid tag file.", 1, 0 );
                return;
            }
        }
        this->pickObj->DisplayPoints();
    }
}

void PickingDialog::EditPoint(QListViewItem* item)
{
    int num = 0;
    if (item)
    {
        num = item->text(0).toInt();
        if (this->pickObj)
        {
            vtkReal pt[3] = {0.0,0.0,0.0};
            this->pickObj->GetPointCoordinates(num-1, pt);
            EditPointCoordinates *dlg = new EditPointCoordinates(this, "editPoint", TRUE );
            dlg->SetPointCoordinates(pt);
            if (dlg->exec() == QDialog::Accepted)
            {
                dlg->GetPointCoordinates(pt);
                this->pickObj->SetPointCoordinates(num-1, pt);
                this->pickObj->DisplayPoints();
                POINT_PROPERTIES ptprop;
                ptprop.index = num-1;
                ptprop.x = pt[0];
                ptprop.y = pt[1];
                ptprop.z = pt[2];
                this->MovePlainsToPoint(&ptprop);
                this->UpdatePointList();
            }
        }
    }
}

void PickingDialog::PointFromListSelected(QListViewItem* item)
{
    int num = 0;
    if (item)
    {
        num = item->text(0).toInt();
        if (this->pickObj)
        {
            this->pickObj->HighlightPoint(num-1);
            POINT_PROPERTIES pt;
            pt.index = num-1;
            if (this->pickObj->GetPointCoordinates(num-1))
            {
                pt.x = this->pickObj->GetPointCoordinates(num-1)[0];
                pt.y = this->pickObj->GetPointCoordinates(num-1)[1];
                pt.z = this->pickObj->GetPointCoordinates(num-1)[2];
                this->MovePlainsToPoint(&pt);
            }
        }
    }
}

void PickingDialog::SetSceneManager( SceneManager * man )
{
    if( man == this->manager )
        return;
    
    if( this->manager )
        this->manager->UnRegister( 0 );
    
    this->manager = man;
    
    if( this->manager )
    {
        this->manager->Register( 0 );
    }
    this->Init();
}

void PickingDialog::Init()
{
    if( this->manager )
        this->pickObj = this->manager->GetPickObj();
    if (this->pickObj)
    {
        this->pickObj->SavePointsForUndo();
        surfaceRadioButton->setEnabled(FALSE);
        objectRadioButton->setEnabled(FALSE);
        switch (this->pickObj->GetPickerType())
        {
        case SURFACE:
        case OBJ:
            if (!this->pickObj->Init())
            {
                QMessageBox::warning( 0, "Error: ", "Please, generate first a surface or select an object to pick on.", 1, 0 );
                return;
            }
            this->UpdatePointList();
            break;
        default:
            break;
        }
        tagSizeSpinBox->blockSignals(TRUE);
        tagSizeSpinBox->setValue((int)this->pickObj->GetPointRadius());
        tagSizeSpinBox->blockSignals(FALSE);
        connect( this->pickObj, SIGNAL( UpdatePoints() ), this, SLOT( UpdatePointList() ) );
        connect( this->pickObj, SIGNAL( PointSelected(POINT_PROPERTIES*) ), this, SLOT( SelectPoint(POINT_PROPERTIES*) ) );
        connect( this->pickObj, SIGNAL( PointAdded(POINT_PROPERTIES*) ), this, SLOT( AddNewPoint(POINT_PROPERTIES*) ) );
        connect( this->pickObj, SIGNAL( PointMoved(POINT_PROPERTIES*) ), this, SLOT( MovePoint(POINT_PROPERTIES*) ) );
        this->pickObj->EnableInteraction(1);
    }
}

void PickingDialog::UpdatePointList()
{
    vtkReal pt1[3], pt2[3], pt3[3];
    QString tmp = "", tmp1 = "", tmp2 = "";
    this->pointListView->clear();
    if (this->pickObj)
    {
        int numPoints = this->pickObj->GetNumberOfPoints();
        for( int i = numPoints - 1; i >= 0; i-- )
        {
            tmp.setNum(i+1);
            if (i > 0)
            {
                this->pickObj->GetPointCoordinates(i, pt1);
                this->pickObj->GetPointCoordinates(i-1, pt2);
                tmp1.setNum(sqrt(vtkMath::Distance2BetweenPoints(pt1, pt2)));
            }
            else
                tmp1.setNum(0.0);
            if (i < numPoints - 1 && i > 0)
            {  
                vtkReal a, b, c;
                this->pickObj->GetPointCoordinates(i, pt1);
                this->pickObj->GetPointCoordinates(i-1, pt2);
                this->pickObj->GetPointCoordinates(i+1, pt3);
                a = vtkMath::Distance2BetweenPoints(pt3, pt2);
                b = vtkMath::Distance2BetweenPoints(pt1, pt3);
                c = vtkMath::Distance2BetweenPoints(pt1, pt2);
#if ((VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION < 4))
                tmp2.setNum(vtkMath::RadiansToDegrees()*(acos((b+c-a)/(2*sqrt(b)*sqrt(c)))));
#else
                tmp2.setNum(vtkMath::DegreesFromRadians((acos((b+c-a)/(2*sqrt(b)*sqrt(c))))));
#endif
            }
            else
                tmp2.setNum(0.0);
            new QListViewItem( this->pointListView, tmp, 
                            this->pickObj->GetPointCoordsString( i ),
                            tmp1, tmp2);
        }
        if (this->currentPointIndex >= numPoints)
            this->currentPointIndex = 0;
        if (numPoints == 0)
            this->currentPointIndex = -1;
        if (this->currentPointIndex >= 0)
        {
            this->HighlightPoint(this->currentPointIndex);
            this->SelectPoint(this->currentPointIndex);
        }
    }
}

void PickingDialog::MovePlainsToPoint(POINT_PROPERTIES* point)
{
    if (this->pickObj)
    {        
        this->pickObj->SetPlanesPosition(point->index, point->x, point->y, point->z);
        this->pickObj->SetCurrentPointIndex(point->index);
    }
}

void PickingDialog::MovePoint(POINT_PROPERTIES* point)
{
    if (this->pickObj)
    {
        vtkReal *pt = &point->x;
        this->pickObj->SetPointCoordinates(point->index, pt);
        this->pickObj->SetPlanesPosition(point->index, point->x, point->y, point->z);
        this->pickObj->ShowPointCoordinatesIn3DView(point->index+1, point->x, point->y, point->z);
        this->currentPointIndex = point->index;
        this->HighlightPoint(this->currentPointIndex);
        this->UpdatePointList();
    }
}

void PickingDialog::SelectPoint(POINT_PROPERTIES* point)
{
    this->currentPointIndex = point->index;
    this->HighlightPoint(this->currentPointIndex);
    this->MovePlainsToPoint(point);
}
void PickingDialog::SelectPoint(int index)
{
    POINT_PROPERTIES point;
    double coords[3];
    point.index = index;
    this->pickObj->GetSelectedPoints()->GetPointCoordinates(index, coords);
    point.x = coords[0];
    point.y = coords[1];
    point.z = coords[2];
    this->currentPointIndex = index;
    this->HighlightPoint(this->currentPointIndex);
    this->MovePlainsToPoint(&point);
}

void PickingDialog::AddNewPoint(POINT_PROPERTIES* point)
{
    if (this->pickObj)
    {
        this->currentPointIndex = point->index;
        this->MovePlainsToPoint(point);
        this->UpdatePointList();
    }
}

void PickingDialog::HighlightPoint(int index)
{
    if (index >= 0)
    {
        this->pointListView->clearSelection();
        QString num = QString::number((int)(index+1));
        QListViewItem * selected = this->pointListView->findItem(num, 0);
        if( selected )
        {
            this->pointListView->setSelected( selected, true );
        }
    }
}

// configuration, reading writing tags
void PickingDialog::ReadConfig()
{
    QString configFileName( QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY + "/" + IGNS_TAG_CONFIG_FILE_NAME );

    if( QFile::exists( configFileName ) )
    {
        SerializerReader reader;
        reader.SetFilename( configFileName );
        reader.Start();
        Serialize( &reader, "TagFileDirectory", settings );
        reader.Finish();
    }
}

void PickingDialog::WriteConfig()
{
    QString configDir( QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY  );
    if( !QFile::exists( configDir ) )
    {
        QDir dir( configDir );
        if( !dir.exists( ) )
            dir.mkdir( configDir );
    }
    QString configFileName( configDir + "/" + IGNS_TAG_CONFIG_FILE_NAME );
    SerializerWriter writer0;
    writer0.SetFilename( configFileName );
    writer0.Start();
    ::Serialize( &writer0, "TagFileDirectory", settings );
    writer0.Finish();
}


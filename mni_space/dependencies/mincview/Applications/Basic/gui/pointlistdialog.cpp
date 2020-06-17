/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: pointlistdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-04-28 19:05:52 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#include "pointlistdialog.h"

#include <qdir.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qdatetime.h>
#include "ignsconfig.h"
#include "ignsmsg.h"
#include "pointsobject.h"
#include "vtkMath.h"
#include "yesnodialog.h"
#include "clearselectedpointdialog.h"

ObjectSerializationMacro( PointListDialogSettings );

PointListDialogSettings::PointListDialogSettings()
{
    m_lastVisitedDirectory = (const char*)(QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY);
}

PointListDialogSettings::~PointListDialogSettings()
{
}

void PointListDialogSettings::Serialize( Serializer * serializer )
{
    ::Serialize( serializer, "LastVisitedDirectory", m_lastVisitedDirectory );
}

PointListDialog::PointListDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : PointListDialogBase( parent, name, modal, fl )
{
    if ( !name )
	setName( "PointListDialog" );
    m_points = 0;
    m_current_point_index = -1;
    m_settings = new PointListDialogSettings;
    pointListView->setColumnText( 0, tr( "No." ) );
    pointListView->setColumnWidth( 0, 30 );
    pointListView->addColumn( tr( "Label" ), 130 );
    pointListView->addColumn( tr( "Coordinates (x,y,z)" ), 215 );
    pointListView->addColumn( tr( "Distance" ), 65 );
    pointListView->addColumn( tr( "Angle" ), 65 );
    pointListView->setSorting( -1, FALSE );
    pointListView->setAllColumnsShowFocus( TRUE );
    pointListView->clear();
}

PointListDialog::~PointListDialog()
{
    if (m_points)
        m_points->UnRegister(0);
    m_points = 0;
    delete m_settings;        
}


void PointListDialog::ClosePushButtonClicked()
{
    if (m_points)
    {
        m_points->ActivateCursorOnAllPlanes(0);
    }
    emit PointListClosed();    
    accept();
}

void PointListDialog::SavePushButtonClicked()
{
    if (m_points)
    {
        int n = m_points->GetNumberOfPoints();
        if (n > 0)
        {
            QString tag_directory = m_settings->m_lastVisitedDirectory;
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
                int ret = QMessageBox::warning(this, tr("Saving Points"),
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
                m_settings->m_lastVisitedDirectory = dirPath;
            }
            QDateTime saveTime = QDateTime::currentDateTime();
            m_points->WriteTagFile(filename, saveTime.toString(Qt::ISODate));
        }
        else
            QMessageBox::warning( this, "Error: ", "There are no points to save.", 1, 0 );
    }
}

void PointListDialog::ClearSelectedPushButtonClicked()
{
    int numPoints = 0;
    QListViewItem *item = this->pointListView->selectedItem();
    if (m_points)
    {
        numPoints = m_points->GetNumberOfPoints();
        if (numPoints > 0)
        {    
            ClearSelectedPointDialog *dlg = new ClearSelectedPointDialog(this, "ClearSelectedPointDialog", TRUE);
            dlg->SetPointsObject(m_points);
            if (item)
                dlg->SetSelectedPointNumber(item->text(0));
            dlg->show();
        }
        else
           QMessageBox::warning( this, "Warning", "There are no picked points.", 1, 0 );
    }
}

void PointListDialog::ClearAllPushButtonClicked()
{
    if (m_points)
    {
        if (m_points->GetNumberOfPoints() > 0 )
        {
            YesNoDialog *dlg = new YesNoDialog(0, "removeAllPoints", TRUE, Qt::WDestructiveClose  );
            dlg->SetQuestion("Remove all picked points?");
            if (dlg->exec() == QDialog::Accepted)
            {
                m_points->RemoveAllPoints();
            }
        }
        this->UpdateUI();
    }
}

void PointListDialog::PointFromListSelected(QListViewItem* item)
{
    int num = 0;
    if (item)
    {
        num = item->text(0).toInt();
        if (m_points)
        {
            m_points->ShowPointInAllViews(num-1);
            m_current_point_index = num - 1;
        }
    }
}

void PointListDialog::SetPoints(PointsObject *pts)
{
    if (m_points == pts)
        return;
    if (m_points)
        m_points->UnRegister(0);
    m_points = pts;
    if (m_points)
    {
        m_points->Register(0);
        tagFileNameLineEdit->setText(m_points->GetName());
        m_current_point_index = m_points->GetHighlightedPointIndex();
    }
}

void PointListDialog::UpdateUI()
{
    if (m_points)
    {
        QListViewItem* item;
        const char* label = m_points->GetHighlightedPointLabel();
            
        vtkReal pt1[3], pt2[3], pt3[3];
        QString tmp = "", tmp0 = "", tmp1 = "", tmp2 = "";
        pointListView->clear();
        int numPoints = m_points->GetNumberOfPoints();
        int i = numPoints - 1;
        do 
        {
            tmp.setNum(i+1);
            tmp0 = m_points->GetPointLabel(i);
            m_points->GetPointCoordinates(i, pt1);
            if (i > 0)
            {
                m_points->GetPointCoordinates(i-1, pt2);
                tmp1.setNum(sqrt(vtkMath::Distance2BetweenPoints(pt1, pt2)));
            }
            else
                tmp1.setNum(0.0);
            if (i < numPoints - 1 && i > 0)
            {  
                vtkReal a, b, c;
                m_points->GetPointCoordinates(i-1, pt2);
                m_points->GetPointCoordinates(i+1, pt3);
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
            QString tmp3 = QString("(%1, %2, %3)") 
                            .arg(pt1[0])
                            .arg(pt1[1])
                            .arg(pt1[2]);
            new QListViewItem( pointListView, tmp, tmp0,
                            tmp3,
                            tmp1, tmp2);
            i--;                            
        } while (i >= 0);
        
        if (label)
        {
            item = pointListView->findItem(label, 1);
            if (item)
            {
                m_current_point_index = item->text(0).toInt()-1;
            }
        }
        else
            m_current_point_index = -1;
        if (m_current_point_index >= numPoints)
            m_current_point_index = 0;
        if (numPoints == 0)
            m_current_point_index = -1;
        if (m_current_point_index >= 0)
            this->HighlightPoint(m_current_point_index);
    }
}

void PointListDialog::HighlightPoint(int index)
{
    if (index >= 0)
    {
        pointListView->clearSelection();
        QString num = QString::number((int)(index+1));
        QListViewItem * selected = pointListView->findItem(num, 0);
        if( selected )
        {
            pointListView->setSelected( selected, true );
            m_current_point_index = index;
            if (m_points)
                m_points->HighlightPoint(index, 1);
        }
    }
}

void PointListDialog::closeEvent(QCloseEvent *event)
{
    if (m_points)
    {
        m_points->ActivateCursorOnAllPlanes(0);
    }
    emit PointListClosed();    
    event->accept();
}

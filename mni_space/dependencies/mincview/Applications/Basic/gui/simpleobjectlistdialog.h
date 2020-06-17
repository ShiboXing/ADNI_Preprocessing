/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: simpleobjectlistdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.7 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#ifndef TAG_SIMPLEOBJECTLISTDIALOG_H
#define TAG_SIMPLEOBJECTLISTDIALOG_H

#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QListView;
class QListViewItem;
class QPushButton;
class QContextMenuEvent;
class QAction;
class SceneManager;

class SimpleObjectListDialog : public QWidget
{
    Q_OBJECT

public:

    SimpleObjectListDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~SimpleObjectListDialog();

    void SetSceneManager( SceneManager * man );

    QListView * ListView;
    
public slots:
    
    void SceneChanged();
    void ListViewSelectionChanged( QListViewItem * item );
    void ListViewClicked( QListViewItem * item );
    
    void DeleteObjectButtonClicked( );
    void DeleteSelectedObject( );
    void HideShowSelectedObject( );
    void MarkAsReferenceObject( );

protected:
    
    void UpdateListView();
    void UpdateObjectSettingsWidget();
    void contextMenuEvent(QContextMenuEvent *event);
    void CreateActions();

    QListViewItem *currentItem;
    QWidget     * currentObjectSettingsWidget;
    QVBoxLayout * ObjectListDialogLayout;
    QPushButton *deleteObjectButton;
    QAction *deleteSelectedObject;
    QAction *hideSelectedObject;
    QAction *markAsReferenceObject;
    SceneManager * sceneManager;

};


#endif //TAG_SIMPLEOBJECTLISTDIALOG_H

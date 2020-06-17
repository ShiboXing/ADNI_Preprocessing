/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: objectlistdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.14 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  from old Simon's code
  All rights reserved.

=========================================================================*/

#ifndef OBJECTLISTDIALOG_H
#define OBJECTLISTDIALOG_H

#include "objectlistdialogbase.h"
#include <qwidget.h>

class QAction;
class QTabWidget;
class QListViewItem;
class SceneManager;
class SceneObject;
class UserTransformations;

class ObjectListDialog : public ObjectListDialogBase
{
    Q_OBJECT

public:
    ObjectListDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual  ~ObjectListDialog();
    
    void SetSceneManager( SceneManager * man );
    
public slots:
    
    virtual void ListViewSelectionChanged(QListViewItem *);
    virtual void ListViewClicked( QListViewItem *);
    virtual void DeleteObjectPushButtonClicked();
    virtual void ObjectListTabWidgetCurrentChanged(QWidget*);
    virtual void SceneChanged();
    virtual void DeleteSelectedObject( );
    virtual void HideShowSelectedObject( );
    virtual void AppliedTransformsChanged();
    virtual void MarkAsReferenceObject( );
    virtual void ShowMINCHeader( );
    
protected:
    void UpdateListView();
    void RecursiveAddChild( QListViewItem * parentItem, SceneObject * parentObject );
    void UpdateObjectSettingsWidget();
    void contextMenuEvent(QContextMenuEvent *event);
    void CreateActions();
    void HideChildren(SceneObject * object, bool hide);
    
    QWidget *m_currentObjectSettingsWidget;
    QAction *m_deleteSelectedObject;
    QAction *m_hideSelectedObject;
    QAction *m_markAsReferenceObject;
    QAction *m_showMINCHeader;
    QListViewItem *m_currentItem;
    SceneManager * m_sceneManager;
    UserTransformations *m_userTransforms;
    bool m_forceSettingsRefresh;
};

#endif // OBJECTLISTDIALOG_H

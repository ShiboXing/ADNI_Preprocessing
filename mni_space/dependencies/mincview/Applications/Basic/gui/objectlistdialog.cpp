/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: objectlistdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:05 $
  Version:   $Revision: 1.33 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  from old Simon's code
  All rights reserved.

=========================================================================*/
#include "objectlistdialog.h"

#include <qtabwidget.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qaction.h>
#include <qdockwindow.h>
#include <qpopupmenu.h>

#include "scenemanager.h"
#include "imageobject.h"
#include "usertransformations.h"
#include "mincinfowidget.h"
#include "qtutilities.h"

ObjectListDialog::ObjectListDialog( QWidget* parent, const char* name, WFlags fl )
    : ObjectListDialogBase( parent, name, fl ),
    m_currentObjectSettingsWidget(0),
    m_deleteSelectedObject(0),
    m_hideSelectedObject(0),
    m_currentItem(0),
    m_sceneManager(0),
    m_userTransforms(0)
{
    if ( !name )
        setName( "ObjectListDialog" );

    QHeader *header;
    dataListView->addColumn(QObject::tr( "Transform" ));
    header = dataListView->header();
    header->setLabel( 0, QObject::tr( "File Name" ), 175 );
    dataListView->clear();
    dataListView->setRootIsDecorated(TRUE);
    dataListView->setVScrollBarMode(QScrollView::AlwaysOn);
    header = toolListView->header();
    header->setLabel( 0, QObject::tr( "Tool Name" ) );
    toolListView->clear();

    CreateActions();
    m_forceSettingsRefresh = false;
    deleteObjectPushButton->setEnabled(false);
}

ObjectListDialog::~ObjectListDialog()
{
    if( m_sceneManager )
    {
        m_sceneManager->UnRegister( 0 );
    }
}

void ObjectListDialog::ListViewSelectionChanged(QListViewItem* item)
{
    deleteObjectPushButton->setEnabled(false);
    if (!item || item->text(0) == INTRAOP_ROOT_OBJECT_NAME)
    {
        if( m_currentObjectSettingsWidget )
        {
            m_currentObjectSettingsWidget->close();
            m_currentObjectSettingsWidget = 0;
        }
    }
    else if( item )
    {
        QString objectName = item->text( 0 );
        m_sceneManager->SetCurrentObject( objectName );
        if (item != m_currentItem)
            this->UpdateObjectSettingsWidget();
        SceneObject * object = m_sceneManager->GetObject( objectName );
        if ( object && !object->GetObjectManagedBySystem() &&
            !(object->GetObjectType() & REFERENCE_DATA_TYPE) )
            deleteObjectPushButton->setEnabled(true);
    }

    m_currentItem = item;
}

void ObjectListDialog::ListViewClicked(QListViewItem* item)
{
    if (!item || item->text(0) == INTRAOP_ROOT_OBJECT_NAME)
    {
        if( m_currentObjectSettingsWidget )
        {
            m_currentObjectSettingsWidget->close();
            m_currentObjectSettingsWidget = 0;
        }
    }
    if( item )
    {
        bool doRefresh = FALSE;;
        QListView *selectedList = item->listView();
        QString objectName = item->text( 0 );
        SceneObject * object = m_sceneManager->GetObject( objectName );
        if (selectedList == dataListView)
        {
            if ( object && !object->GetObjectManagedBySystem() )
            {
                if (((QCheckListItem*)item )->isOn() )
                {
                    if (object->GetObjectHidden())
                    {
                        object->SetObjectHidden(FALSE);
                        object->HideShow();
                        doRefresh = TRUE;
                    }
                }
                else
                {
                    if (!object->GetObjectHidden())
                    {
                        object->SetObjectHidden(TRUE);
                        object->HideShow();
                    }
                }
            }
            else if (objectName == PREOP_ROOT_OBJECT_NAME || objectName == INTRAOP_ROOT_OBJECT_NAME)
            {
                this->HideChildren(object, !((QCheckListItem*)item )->isOn());
            }
            // Yet another hack to show planes correctly blended after hiding and showing the reference image
            if (doRefresh &&
                m_sceneManager->GetViewMode() == VIEW_SYNCHRONIZED &&
                (object->GetObjectType() & REFERENCE_DATA_TYPE))
            {
                SceneObject *preopRoot = m_sceneManager->GetObject(PREOP_ROOT_OBJECT_NAME);
                if (preopRoot)
                {
                    this->HideChildren(m_sceneManager->GetObject(PREOP_ROOT_OBJECT_NAME), TRUE);
                    this->HideChildren(m_sceneManager->GetObject(PREOP_ROOT_OBJECT_NAME), FALSE);
                }
            }
        }
        m_sceneManager->SetCurrentObject( objectName );
    }
    if (item != m_currentItem)
        this->UpdateObjectSettingsWidget();
    m_currentItem = item;
}

void ObjectListDialog::ObjectListTabWidgetCurrentChanged(QWidget* currentPage)
{
    if( m_currentObjectSettingsWidget )
    {
        m_currentObjectSettingsWidget->close();
        m_currentObjectSettingsWidget = 0;
    }
    int pageIndex = objectListTabWidget->currentPageIndex();
    QListView *selectedList;
    if (pageIndex == 1) 
        selectedList = toolListView;
    else
        selectedList = dataListView;
    QListViewItem* item = selectedList->currentItem(); 
    if (item && item->text(0) != INTRAOP_ROOT_OBJECT_NAME)
    {        
        QString objectName = item->text( 0 );
        m_sceneManager->SetCurrentObject( objectName );
        if (item != m_currentItem)
            this->UpdateObjectSettingsWidget();
    }
    m_currentItem = item;
}

void ObjectListDialog::DeleteObjectPushButtonClicked()
{
    int pageIndex = objectListTabWidget->currentPageIndex();
    QListView *selectedList;
    if (pageIndex == 1) 
        selectedList = toolListView;
    else
        selectedList = dataListView;
    QListViewItem *item = selectedList->selectedItem();
    if (item)
    {
        QListView *selectedList = item->listView();
        if (selectedList == dataListView)
        {
            SceneObject *object = m_sceneManager->GetObject(item->text(0)); 
            if(object  && !object->GetObjectManagedBySystem())
            {
                m_sceneManager->RemoveObject(object);
                this->UpdateListView();
            }
        }
        if  (item == m_currentItem)
        {
            m_currentItem = 0;
            if( m_currentObjectSettingsWidget )
            {
                m_currentObjectSettingsWidget->close();
                m_currentObjectSettingsWidget = 0;
            }
        }
    }
    deleteObjectPushButton->setEnabled(false);
}

void ObjectListDialog::SetSceneManager( SceneManager * man )
{
    if( man == m_sceneManager )
    {
        return;
    }

    if( m_sceneManager )
    {
        m_sceneManager->UnRegister( 0 );
        m_sceneManager->disconnect( this );
    }

    m_sceneManager = man;

    if( m_sceneManager )
    {
        m_sceneManager->Register( 0 );
        connect( m_sceneManager, SIGNAL( ObjectAdded() ), this, SLOT( SceneChanged() ) );
        connect( m_sceneManager, SIGNAL( ObjectRemoved() ), this, SLOT( SceneChanged() ) );
        connect( m_sceneManager, SIGNAL( ObjectNameChanged() ), this, SLOT( SceneChanged() ) );
        connect( m_sceneManager, SIGNAL( ObjectShowHide() ), this, SLOT( SceneChanged() ) );
        connect( m_sceneManager, SIGNAL( ObjectTransformChanged() ), this, SLOT( AppliedTransformsChanged() ) );
        m_userTransforms = m_sceneManager->GetUserTransforms();
        if (m_userTransforms)
            connect( m_userTransforms, SIGNAL( TransformsChanged() ), this, SLOT( SceneChanged() ) );
    }
    this->UpdateListView();
}

void ObjectListDialog::SceneChanged()
{
    this->UpdateListView();
}

void ObjectListDialog::RecursiveAddChild( QListViewItem * parentItem, SceneObject * parentObject )
{
    int numberOfChildren = parentObject->GetNumberOfChildren();
    QCheckListItem *checkedItem = 0;
    QListViewItem *item = 0;
    for( int i = 0; i < numberOfChildren; i++ )
    {
        SceneObject * object = parentObject->GetChild( i );
        // reslice is an object hidden from the user. The user may access it via its associated image object
        if (!(object->GetObjectType() & RESLICED_DATA_TYPE))
        {
            checkedItem = new QCheckListItem( parentItem, object->GetName(), QCheckListItem::CheckBoxController);
            if (!object->GetObjectManagedBySystem())
            {
                if (object->GetObjectHidden() && !object->GetObjectReplacedInSynchronizedView())
                    checkedItem->setState(QCheckListItem::Off);
                else
                    checkedItem->setState(QCheckListItem::On);
                RecursiveAddChild( checkedItem, object );
                if (object->GetObjectType() & INTRAOP_TYPE)
                    checkedItem->setText(1, object->GetTransformName()); 
                if (object->GetObjectType() & REFERENCE_DATA_TYPE )
                    checkedItem->setText(1, "Reference Image"); 
            }
            else
            {
                item = new QListViewItem( parentItem, object->GetName());
                RecursiveAddChild( item, object );
            }
            this->UpdateObjectSettingsWidget();
        }
    }
}

void ObjectListDialog::UpdateListView()
{
    if( m_sceneManager )
    {
        dataListView->clear();
        toolListView->clear();
        QString currentObjectName = m_sceneManager->GetCurrentObjectName();
        int numberOfChildren = m_sceneManager->GetNumberOfObjects();
        QCheckListItem *checkedItem = 0;
        QListViewItem *item = 0;
        QString objName;
        for( int i = 0; i < numberOfChildren; i++ )
        {
            SceneObject * object = m_sceneManager->GetObject( i );
            objName = object->GetName();
            if (!object->GetObjectManagedBySystem())
            {
                checkedItem = new QCheckListItem( dataListView, objName, QCheckListItem::CheckBoxController);
                if (object->GetObjectHidden())
                    checkedItem->setState(QCheckListItem::Off);
                else
                    checkedItem->setState(QCheckListItem::On);
            }
            else
            {
                if (objName == PREOP_ROOT_OBJECT_NAME || objName == INTRAOP_ROOT_OBJECT_NAME)
                {
                    checkedItem = new QCheckListItem( dataListView, objName, QCheckListItem::CheckBoxController);
                    if (objName == PREOP_ROOT_OBJECT_NAME)
                        checkedItem->setText(1, object->GetTransformName()); 
                    RecursiveAddChild( checkedItem, object );
                }
                else
                {
                    //tool objects are not added hierarchically
                    item = new QListViewItem( toolListView, object->GetName()); 
                }
            }
        }
        item = dataListView->findItem(currentObjectName, 0);
        if (item)
        {
            QListViewItem *parent;
            parent = item->parent();
            if (parent)
                parent->setOpen(TRUE);
            dataListView->setCurrentItem(item);  
            if (objectListTabWidget->currentPageIndex() != 0) // current tab is not data tab
            {
                objectListTabWidget->showPage(dataTab);
            }
        }  
        else
        {
            item = toolListView->findItem(currentObjectName, 0);
            if (item)
            {
                toolListView->setCurrentItem(item);  
                if (objectListTabWidget->currentPageIndex() != 1) // current tab is not tool tab
                {
                    objectListTabWidget->showPage(toolTab);
                }
            }  
        }
        if (!item || item->text(0) == INTRAOP_ROOT_OBJECT_NAME)
        {
            if( m_currentObjectSettingsWidget )
            {
                m_currentObjectSettingsWidget->close();
                m_currentObjectSettingsWidget = 0;
            }
        }
        else if (item != m_currentItem || m_forceSettingsRefresh) // always update Preop root settings, as transformation name might be changed
        {
            this->UpdateObjectSettingsWidget();
            m_currentItem = item;
        }
    }
}

void ObjectListDialog::UpdateObjectSettingsWidget()
{
    if( m_currentObjectSettingsWidget )
    {
        m_currentObjectSettingsWidget->close();
        delete m_currentObjectSettingsWidget;
        m_currentObjectSettingsWidget = 0;
    }
    int pageIndex = objectListTabWidget->currentPageIndex();
    QListView *selectedList;
    if (pageIndex == 1) 
        selectedList = toolListView;
    else
        selectedList = dataListView;
    QListViewItem* item = selectedList->currentItem(); 
    if (!item)
        return;
    if (item->text(0) == PREOP_ROOT_OBJECT_NAME)
        item->setSelected(true);
    SceneObject * object = m_sceneManager->GetCurrentObject( );
    if( object )
    {
        m_currentObjectSettingsWidget = object->CreateSettingsDialog(this->parentWidget(true) );
        if( m_currentObjectSettingsWidget )
        {
            m_currentObjectSettingsWidget->show();
        }
        deleteObjectPushButton->setEnabled(false);
        if (!(object->GetObjectManagedBySystem()) && !(object->GetObjectType() & REFERENCE_DATA_TYPE))
            deleteObjectPushButton->setEnabled(true);
    }
    m_forceSettingsRefresh = false;
}

void ObjectListDialog::CreateActions()
{
    m_deleteSelectedObject = new QAction("&Delete selected object", CTRL+Key_D, this, "deleteSelectedObject");
    connect(m_deleteSelectedObject, SIGNAL(activated()), this, SLOT (DeleteSelectedObject()));
    m_hideSelectedObject = new QAction("Hide/show selected object", CTRL+Key_H, this, "hideSelectedObject");
    connect(m_hideSelectedObject, SIGNAL(activated()), this, SLOT (HideShowSelectedObject()));
    m_markAsReferenceObject = new QAction("Mark selected object as Reference Image", CTRL+Key_M, this, "markAsReferenceObject");
    connect(m_markAsReferenceObject, SIGNAL(activated()), this, SLOT (MarkAsReferenceObject()));
    m_showMINCHeader = new QAction("Show MINC Header", CTRL+Key_M, this, "showMINCHeader");
    connect(m_showMINCHeader, SIGNAL(activated()), this, SLOT (ShowMINCHeader()));
}

void ObjectListDialog::contextMenuEvent(QContextMenuEvent *event)
{
    int pageIndex = objectListTabWidget->currentPageIndex();
    QListView *selectedList;
    if (pageIndex == 1) 
        selectedList = toolListView;
    else
        selectedList = dataListView;
    QListViewItem* item = selectedList->currentItem(); 
    if (item)
    {
        SceneObject *obj = m_sceneManager->GetObject(item->text(0));
        if(!obj->GetObjectManagedBySystem())
        {
            QPopupMenu contextMenu(this);
            m_hideSelectedObject->addTo(&contextMenu);
            if (!(obj->GetObjectType() & REFERENCE_DATA_TYPE))
                m_deleteSelectedObject->addTo(&contextMenu);
            if (obj->GetObjectHidden())
            {
                m_hideSelectedObject->setMenuText(tr("Show selected object"));            
            }
            else
            {
                m_hideSelectedObject->setMenuText(tr("Hide selected object"));            
            }
            if ((obj->GetObjectType() & PREOP_TYPE) &&
                !(obj->GetObjectType() & REFERENCE_DATA_TYPE) && 
                (obj->GetObjectType() & MINC_TYPE) && 
                (m_sceneManager->GetViewMode() != VIEW_SYNCHRONIZED))
                m_markAsReferenceObject->addTo(&contextMenu);
            if ((obj->GetObjectType() & PREOP_TYPE) &&
                (obj->GetObjectType() & MINC_TYPE))
                m_showMINCHeader->addTo(&contextMenu);
                
            contextMenu.exec(event->globalPos());
        }
    }
}

void ObjectListDialog::DeleteSelectedObject()
{
    int pageIndex = objectListTabWidget->currentPageIndex();
    QListView *selectedList;
    if (pageIndex == 1) 
        selectedList = toolListView;
    else
        selectedList = dataListView;
    QListViewItem* item = selectedList->currentItem(); 
    if (item)
    {
        SceneObject *obj = m_sceneManager->GetObject(item->text(0));
        if(!obj->GetObjectManagedBySystem())
        {
            m_sceneManager->RemoveObject(obj);
            this->UpdateListView();
        }
    }
    deleteObjectPushButton->setEnabled(false);
    if (!item || item == m_currentItem)
    {
        if( m_currentObjectSettingsWidget )
        {
            m_currentObjectSettingsWidget->close();
            m_currentObjectSettingsWidget = 0;
        }
        m_currentItem = 0;
    }
}

void ObjectListDialog::HideShowSelectedObject()
{
    int pageIndex = objectListTabWidget->currentPageIndex();
    bool doRefresh = FALSE;
    QListView *selectedList;
    if (pageIndex == 1) 
        selectedList = toolListView;
    else
        selectedList = dataListView;
    QListViewItem* item = selectedList->currentItem(); 
    if (item)
    {
        SceneObject *obj = m_sceneManager->GetObject(item->text(0));
        if (!obj->GetObjectManagedBySystem())
        {
            if (obj->GetObjectHidden())
            {
                obj->SetObjectHidden(FALSE);
                doRefresh = TRUE;
            }
            else
            {
                obj->SetObjectHidden(TRUE);
            }
            obj->HideShow();
            // Yet another hack to show planes correctly blended after hiding and showing the reference image
            if (doRefresh &&
                m_sceneManager->GetViewMode() == VIEW_SYNCHRONIZED &&
                (obj->GetObjectType() & REFERENCE_DATA_TYPE))
            {
                SceneObject *preopRoot = m_sceneManager->GetObject(PREOP_ROOT_OBJECT_NAME);
                if (preopRoot)
                {
                    this->HideChildren(m_sceneManager->GetObject(PREOP_ROOT_OBJECT_NAME), TRUE);
                    this->HideChildren(m_sceneManager->GetObject(PREOP_ROOT_OBJECT_NAME), FALSE);
                }
            }
            m_sceneManager->SetCurrentObject(obj->GetName());
            this->UpdateListView();
        }
    } 
}

void ObjectListDialog::HideChildren(SceneObject *rootObject, bool hide)
{
    int numberOfChildren = rootObject->GetNumberOfChildren();
    QCheckListItem *checkedItem = 0;
    QString objName;
    bool alreadyHidden;
    for( int i = 0; i < numberOfChildren; i++ )
    {
        SceneObject * object = rootObject->GetChild( i );
        objName = object->GetName();
        checkedItem = (QCheckListItem *)dataListView->findItem(objName, 0);
        if (!object->GetObjectManagedBySystem())
        {
            alreadyHidden = object->GetObjectHidden(); 
            if (alreadyHidden != hide)
            {
                if (alreadyHidden)
                {
                    object->SetObjectHidden(FALSE);
                    if (checkedItem)
                        checkedItem->setState(QCheckListItem::On);
                }
                else
                {
                    object->SetObjectHidden(TRUE);
                    if (checkedItem)
                        checkedItem->setState(QCheckListItem::Off);
                }
                object->HideShow();
            }
            this->UpdateListView();
        }
        else
            HideChildren(object, hide);
    } 
}

void ObjectListDialog::AppliedTransformsChanged()
{
    m_forceSettingsRefresh = true;
    this->UpdateListView();
}

void ObjectListDialog::MarkAsReferenceObject( )
{
    int pageIndex = objectListTabWidget->currentPageIndex();
    QListView *selectedList;
    if (pageIndex == 1) 
        selectedList = toolListView;
    else
        selectedList = dataListView;
    QListViewItem *item = selectedList->selectedItem();
    if (item)
    {
        SceneObject *obj = m_sceneManager->GetObject(item->text(0));
        if (!obj->GetObjectManagedBySystem()
            && (obj->GetObjectType() & MINC_TYPE)
            && (m_sceneManager->GetViewMode() != VIEW_SYNCHRONIZED))
        {
            ImageObject * iObj = reinterpret_cast<ImageObject *>(obj);
            m_sceneManager->SetReferenceDataObject(iObj);
            this->UpdateListView();
        }
    }
}

void ObjectListDialog::ShowMINCHeader( )
{
    QString info;
    int pageIndex = objectListTabWidget->currentPageIndex();
    QListView *selectedList;
    if (pageIndex == 1) 
        selectedList = toolListView;
    else
        selectedList = dataListView;
    QListViewItem *item = selectedList->selectedItem();
    if (item)
    {
        SceneObject *obj = m_sceneManager->GetObject(item->text(0));
        if (!obj->GetObjectManagedBySystem()
            && (obj->GetObjectType() & MINC_TYPE))
        {
            ImageObject * iObj = reinterpret_cast<ImageObject *>(obj);
            iObj->GetMINCHeaderString(info);
            MincInfoWidget *infoWidget = new MincInfoWidget(0, "MincInfo", Qt::WDestructiveClose);
            infoWidget->SetInfoText(info);
            infoWidget->show();
        }
    }
}

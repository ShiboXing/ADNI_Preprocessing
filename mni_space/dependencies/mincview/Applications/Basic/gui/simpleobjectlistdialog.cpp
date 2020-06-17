/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: simpleobjectlistdialog.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-18 19:59:06 $
  Version:   $Revision: 1.12 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "simpleobjectlistdialog.h"

#include <qvariant.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qkeysequence.h>

#include "scenemanager.h"
#include "imageobject.h"


SimpleObjectListDialog::SimpleObjectListDialog( QWidget* parent, const char* name, WFlags fl ) : QWidget( parent, name, fl )
{
    if ( !name )
        setName( "ObjectListDialog" );
    ObjectListDialogLayout = new QVBoxLayout( this, 4, 6, "ObjectListDialogLayout");

    // The list view
    ListView = new QListView( this, "ListView" );
    ListView->addColumn( tr( "Object Name" ) );
    ListView->setSorting( -1, FALSE );
    ObjectListDialogLayout->addWidget( ListView );
    
    deleteObjectButton = new QPushButton(this, "deleteObjectButton");
    deleteObjectButton->setMaximumSize( QSize( 320, 20 ) );
    deleteObjectButton->setText("Remove Object");
    deleteObjectButton->setEnabled(false);
    ObjectListDialogLayout->addWidget( deleteObjectButton );
    
    resize( QSize(296, 409).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    
    connect( this->ListView, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( ListViewSelectionChanged( QListViewItem *)  ) );
    connect( this->ListView, SIGNAL( clicked(QListViewItem*) ), this, SLOT( ListViewClicked( QListViewItem *)  ) );
    connect( this->deleteObjectButton, SIGNAL( clicked() ), this, SLOT( DeleteObjectButtonClicked( )  ) );
    
    CreateActions();

    this->currentObjectSettingsWidget = 0;
    this->sceneManager = 0;
    this->currentItem = 0;
}


SimpleObjectListDialog::~SimpleObjectListDialog()
{
    if( this->sceneManager )
    {
        this->sceneManager->UnRegister( 0 );
    }
}


void SimpleObjectListDialog::SetSceneManager( SceneManager * man )
{
    if( man == this->sceneManager )
    {
        return;
    }

    if( this->sceneManager )
    {
        this->sceneManager->UnRegister( 0 );
        this->sceneManager->disconnect( this );
    }

    this->sceneManager = man;

    if( this->sceneManager )
    {
        this->sceneManager->Register( 0 );
        connect( this->sceneManager, SIGNAL( ObjectAdded() ), this, SLOT( SceneChanged() ) );
        connect( this->sceneManager, SIGNAL( ObjectRemoved() ), this, SLOT( SceneChanged() ) );
        connect( this->sceneManager, SIGNAL( ObjectNameChanged() ), this, SLOT( SceneChanged() ) );
        connect( this->sceneManager, SIGNAL( ObjectShowHide() ), this, SLOT( SceneChanged() ) );
    }
    
    this->UpdateListView();
}

void SimpleObjectListDialog::SceneChanged()
{
    this->UpdateListView();
}


void SimpleObjectListDialog::ListViewSelectionChanged( QListViewItem * item )
{
    if(!item)
    {
        if (this->currentObjectSettingsWidget )
        {
            this->currentObjectSettingsWidget->close();
            this->currentObjectSettingsWidget = 0;
        }
        this->deleteObjectButton->setEnabled(FALSE);
    }
    else
    {
        QString objectName = item->text( 0 );
        this->sceneManager->SetCurrentObject( objectName );
        if (item != this->currentItem)
            this->UpdateObjectSettingsWidget();
        
        SceneObject * object = this->sceneManager->GetObject( objectName );
        if ( object && !object->GetObjectManagedBySystem() )
            this->deleteObjectButton->setEnabled(TRUE);
        else
            this->deleteObjectButton->setEnabled(FALSE);
    }
    this->currentItem = item;
}

void SimpleObjectListDialog::ListViewClicked( QListViewItem * item )
{
    if (!item)
    {
        if( this->currentObjectSettingsWidget )
        {
            this->currentObjectSettingsWidget->close();
            this->currentObjectSettingsWidget = 0;
        }
        this->deleteObjectButton->setEnabled(FALSE);
    }
    else
    {
        QString objectName = item->text( 0 );
        this->sceneManager->SetCurrentObject( objectName );
        SceneObject * object = this->sceneManager->GetObject( objectName );
        if ( object && !object->GetObjectManagedBySystem() )
        {
            if (((QCheckListItem*)item )->isOn() )
            {
                if (object->GetObjectHidden())
                {
                    object->SetObjectHidden(FALSE);
                    object->HideShow();
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
        if (item != this->currentItem)
            this->UpdateObjectSettingsWidget();
        this->currentItem = item;
    }
}

void RecursiveAddChild( QListViewItem * parentItem, SceneObject * parentObject )
{
    int numberOfChildren = parentObject->GetNumberOfChildren();
    QCheckListItem *checkedItem = 0;
    QListViewItem *item = 0;
    for( int i = 0; i < numberOfChildren; i++ )
    {
        SceneObject * object = parentObject->GetChild( i );
        checkedItem = new QCheckListItem( parentItem, object->GetName(), QCheckListItem::CheckBoxController);
        if (!object->GetObjectManagedBySystem())
        {
            if (object->GetObjectHidden())
                checkedItem->setState(QCheckListItem::Off);
            else
                checkedItem->setState(QCheckListItem::On);
            RecursiveAddChild( checkedItem, object );
        }
        else
        {
            item = new QListViewItem( parentItem, object->GetName());
            RecursiveAddChild( item, object );
        }
    }
}

void SimpleObjectListDialog::UpdateListView()
{
    if( this->sceneManager )
    {
        this->ListView->clear();
        QString currentObjectName = this->sceneManager->GetCurrentObjectName();
        int numberOfChildren = this->sceneManager->GetNumberOfObjects();
        QCheckListItem *checkedItem = 0;
        QListViewItem *item = 0;
        for( int i = 0; i < numberOfChildren; i++ )
        {
            SceneObject * object = this->sceneManager->GetObject( i );
            if (!(object->GetObjectType() & RESLICED_DATA_TYPE))
            {
            if (!object->GetObjectManagedBySystem())
            {
                    checkedItem = new QCheckListItem( this->ListView, object->GetName(), QCheckListItem::CheckBoxController);
                    if (object->GetObjectHidden())
                        checkedItem->setState(QCheckListItem::Off);
                    else
                        checkedItem->setState(QCheckListItem::On);
                    checkedItem->setEnabled(TRUE);
                    RecursiveAddChild( checkedItem, object );
                }
                else
                {
                    item = new QListViewItem( this->ListView, object->GetName());
                    RecursiveAddChild( item, object );
                }
            }
        }
        item = this->ListView->findItem(currentObjectName, 0);
        if (item)
        {
            QListViewItem *p;
            p = item->parent();
            if (p)
                p->setOpen(TRUE);
            this->ListView->setCurrentItem(item);  
        }  
        if (item != this->currentItem)
            this->UpdateObjectSettingsWidget();
        this->currentItem = item;
    }
}

void SimpleObjectListDialog::UpdateObjectSettingsWidget()
{
    if( this->currentObjectSettingsWidget )
    {
        this->currentObjectSettingsWidget->close();
        this->currentObjectSettingsWidget = 0;
    }
    SceneObject * object = this->sceneManager->GetCurrentObject( );
    if( object )
    {
        this->currentObjectSettingsWidget = object->CreateSettingsDialog( this->parentWidget () );
        if( this->currentObjectSettingsWidget )
        {
            this->ObjectListDialogLayout->addWidget( this->currentObjectSettingsWidget );
            this->currentObjectSettingsWidget->show();
        }
    }
}

void SimpleObjectListDialog :: DeleteObjectButtonClicked()
{
    QListViewItem *item = this->ListView->selectedItem();
    if (item)
    {
        SceneObject *obj = this->sceneManager->GetObject(item->text(0));
        if(obj)
        {
            this->sceneManager->RemoveObject(obj);
            this->UpdateListView();
        }
    }
    deleteObjectButton->setEnabled(false);
}

void SimpleObjectListDialog :: CreateActions()
{
    deleteSelectedObject = new QAction("&Delete selected object", CTRL+Key_D, this, "deleteSelectedObject");
    connect(deleteSelectedObject, SIGNAL(activated()), this, SLOT (DeleteSelectedObject()));
    hideSelectedObject = new QAction("Hide/show selected object", CTRL+Key_H, this, "hideSelectedObject");
    connect(hideSelectedObject, SIGNAL(activated()), this, SLOT (HideShowSelectedObject()));
    markAsReferenceObject = new QAction("Mark selected object as Reference Image", CTRL+Key_M, this, "markAsReferenceObject");
    connect(markAsReferenceObject, SIGNAL(activated()), this, SLOT (MarkAsReferenceObject()));
}

void SimpleObjectListDialog :: contextMenuEvent(QContextMenuEvent *event)
{
    QListViewItem *item = this->ListView->selectedItem();
    if (item)
    {
        SceneObject *obj = this->sceneManager->GetObject(item->text(0));
        if(!obj->GetObjectManagedBySystem())
        {
            QPopupMenu contextMenu(this);
            hideSelectedObject->addTo(&contextMenu);
            deleteSelectedObject->addTo(&contextMenu);
            if (obj->GetObjectHidden())
            {
                hideSelectedObject->setMenuText(tr("Show selected object"));            
            }
            else
            {
                hideSelectedObject->setMenuText(tr("Hide selected object"));            
            }
            if (obj->GetObjectType() & MINC_TYPE)
                markAsReferenceObject->addTo(&contextMenu);
            contextMenu.exec(event->globalPos());
        }
    }
}

void SimpleObjectListDialog :: DeleteSelectedObject()
{
    QListViewItem *item = this->ListView->selectedItem();
    if (item)
    {
        SceneObject *obj = this->sceneManager->GetObject(item->text(0));
        if(!obj->GetObjectManagedBySystem())
        {
            this->sceneManager->RemoveObject(obj);
            this->UpdateListView();
        }
    }
    deleteObjectButton->setEnabled(false);
}

void SimpleObjectListDialog :: HideShowSelectedObject()
{
    QListViewItem *item = this->ListView->selectedItem();
    if (item)
    {
        SceneObject *obj = this->sceneManager->GetObject(item->text(0));
        if (!obj->GetObjectManagedBySystem())
        {
            if (obj->GetObjectHidden())
            {
                obj->SetObjectHidden(FALSE);
            }
            else
            {
                obj->SetObjectHidden(TRUE);
            }
            obj->HideShow();
            this->UpdateListView();
        }
    } 
}

void SimpleObjectListDialog::MarkAsReferenceObject( )
{
    QListViewItem *item = this->ListView->selectedItem();
    if (item)
    {
        SceneObject *obj = this->sceneManager->GetObject(item->text(0));
        if (!obj->GetObjectManagedBySystem()
            && (obj->GetObjectType() & MINC_TYPE)
            && (this->sceneManager->GetViewMode() != VIEW_SYNCHRONIZED))
        {
            ImageObject * iObj = reinterpret_cast<ImageObject *>(obj);
            this->sceneManager->SetReferenceDataObject(iObj);
        }
    }
}


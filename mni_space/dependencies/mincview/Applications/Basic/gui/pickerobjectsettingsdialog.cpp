#include "pickerobjectsettingsdialog.h"

#include <qpushbutton.h>
#include <qlayout.h>
#include <qdockwindow.h>
#include "qtutilities.h"

#include "pickerobject.h"
#include "scenemanager.h"
#include "pickingdialog.h"

PickerObjectSettingsDialog::PickerObjectSettingsDialog( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
        setName( "PickerObjectSettingsDialog" );
    PickerObjectSettingsDialogLayout = new QVBoxLayout( this, 11, 6, "PickerObjectSettingsDialogLayout"); 
    
    enablePickingButton = new QPushButton( this );
    connect( enablePickingButton, SIGNAL( clicked(  ) ), this, SLOT( EnablePickingButtonToggled( ) ));
    PickerObjectSettingsDialogLayout->addWidget( enablePickingButton );
    
    languageChange();
    resize( QSize(251, 308).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    
    this->manager = 0;
}

PickerObjectSettingsDialog::~PickerObjectSettingsDialog()
{
    if( this->manager )
    {
        this->manager->UnRegister( 0 );
    }
}

void PickerObjectSettingsDialog::SetSceneManager( SceneManager * man )
{
    if( man == this->manager )
    {
        return;
    }
    
    if( this->manager )
    {
        this->manager->UnRegister( 0 );
    }
    
    this->manager = man;
    
    if( this->manager )
    {
        this->manager->Register( 0 );
    }
}

void PickerObjectSettingsDialog::EnablePickingButtonToggled( )
{
    QWidget *w = GetWidgetByName( "PickingDialog" );
    if( !w )
    {
        PickingDialog *pickDialog = new PickingDialog(0, "PickingDialog", FALSE, Qt::WDestructiveClose | Qt::WStyle_StaysOnTop);
        pickDialog->SetSceneManager( this->manager );
        pickDialog->show();
    }
    else
    {
        w->showNormal();
        w->raise();
        w->setActiveWindow();
    }
}

void PickerObjectSettingsDialog::languageChange()
{
    setCaption( tr( "Picker Object Settings" ) );
    enablePickingButton->setText(tr ("Enable Picking") );
}


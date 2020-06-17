#include "scenemanagersettingsdialog.h"
#include "scenemanager.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qcolordialog.h>

SceneManagerSettingsDialog::SceneManagerSettingsDialog( QWidget* parent, const char* name, WFlags fl ) : QWidget( parent, name, fl )
{
    if ( !name )
        setName( "SceneManagerSettingsDialog" );
    SceneManagerSettingsDialogLayout = new QVBoxLayout( this, 11, 6, "SceneManagerSettingsDialogLayout"); 

    layout2 = new QHBoxLayout( 0, 0, 6, "layout2"); 

    backgroundColorLabel = new QLabel( this, "textLabel2" );
    layout2->addWidget( backgroundColorLabel );

    setBackgroundColorButton = new QPushButton( this, "pushButton2" );
    setBackgroundColorButton->setMaximumSize( QSize( 50, 32767 ) );
    layout2->addWidget( setBackgroundColorButton );
    SceneManagerSettingsDialogLayout->addLayout( layout2 );
    languageChange();
    resize( QSize(256, 50).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( setBackgroundColorButton, SIGNAL( clicked() ), this, SLOT( SetBackgroundColorButtonClicked() ) );
    
    this->SceneMan = 0;
}

SceneManagerSettingsDialog::~SceneManagerSettingsDialog()
{
    if( this->SceneMan )
    {
        this->SceneMan->UnRegister( 0 );
    }
}

void SceneManagerSettingsDialog::SetSceneManager( SceneManager * man )
{
    if( man == this->SceneMan )
    {
        return;
    }
    
    if( this->SceneMan )
    {
        this->SceneMan->UnRegister( 0 );
    }
    
    this->SceneMan = man;
    
    if( this->SceneMan )
    {
        this->SceneMan->Register( 0 );
    }
    
    this->UpdateUI();
}

void SceneManagerSettingsDialog::languageChange()
{
    setCaption( tr( "Display settings" ) );
    backgroundColorLabel->setText( tr( "View background color" ) );
    setBackgroundColorButton->setText( tr( "Set..." ) );
}

void SceneManagerSettingsDialog::SetBackgroundColorButtonClicked()
{
    vtkReal * oldColor = this->SceneMan->GetViewBackgroundColor();
    QColor initial( oldColor[0] * 255, oldColor[1] * 255, oldColor[2] * 255 );
    QColor newColor = QColorDialog::getColor( initial );
    vtkReal newColorfloat[3] = { 1, 1, 1 };
    newColorfloat[0] = vtkReal( newColor.red() ) / 255.0; 
    newColorfloat[1] = vtkReal( newColor.green() ) / 255.0;
    newColorfloat[2] = vtkReal( newColor.blue() ) / 255.0;
    this->SceneMan->SetViewBackgroundColor( newColorfloat );
    this->UpdateUI();
}

void SceneManagerSettingsDialog::UpdateUI()
{
    vtkReal * color = this->SceneMan->GetViewBackgroundColor();
    QColor colorQt( color[0] * 255, color[1] * 255, color[2] * 255 );
    this->backgroundColorLabel->setBackgroundColor( colorQt );
}


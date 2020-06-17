
#include "zoomfactordialog.h"

#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qstring.h>
#include "scenemanager.h"

ZoomFactorDialog::ZoomFactorDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : ZoomFactorDialogBase( parent, name, modal, fl )
{
    if ( !name )
	setName( "ZoomFactorDialogBase" );
    
    m_manager = 0;
    factorComboBox->insertItem("5.0");
    factorComboBox->insertItem("4.0");
    factorComboBox->insertItem("3.0");
    factorComboBox->insertItem("2.0");
    factorComboBox->insertItem("1.9");
    factorComboBox->insertItem("1.8");
    factorComboBox->insertItem("1.7");
    factorComboBox->insertItem("1.6");
    factorComboBox->insertItem("1.5");
    factorComboBox->insertItem("1.4");
    factorComboBox->insertItem("1.3");
    factorComboBox->insertItem("1.2");
    factorComboBox->insertItem("1.1");
    factorComboBox->setCurrentItem(8);
    inRadioButton->setChecked(TRUE);
}

ZoomFactorDialog::~ZoomFactorDialog()
{
    if( m_manager )
        m_manager->UnRegister( 0 );
}

void ZoomFactorDialog::OkButtonClicked()
{
    double factor;
    factor = factorComboBox->currentText().toDouble();
    if (factor != 0.0)
    {
        if (zoomInOutButtonGroup->selectedId() > 0)
            factor = 1.0/factor;
        m_manager->ZoomAllCameras(factor);
    }
    accept();
}

void ZoomFactorDialog::SetSceneManager(SceneManager *man)
{
    if( man == m_manager )
        return;
    
    if( m_manager )
        m_manager->UnRegister( 0 );
    
    m_manager = man;
    
    if( m_manager )
    {
        m_manager->Register( 0 );
    }
}



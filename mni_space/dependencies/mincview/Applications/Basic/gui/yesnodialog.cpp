#include "yesnodialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstring.h>

/*
 *  Constructs a YesNoDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
YesNoDialog::YesNoDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
    setName( "YesNoDialog" );

    cancelButton = new QPushButton( this, "cancelButton" );
    cancelButton->setGeometry( QRect( 160, 100, 90, 30 ) );

    question = new QLabel( this, "question" );
    question->setGeometry( QRect( 10, 10, 270, 70 ) );

    okButton = new QPushButton( this, "okButton" );
    okButton->setGeometry( QRect( 40, 100, 90, 30 ) );
    languageChange();
    resize( QSize(290, 150).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    
    connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
YesNoDialog::~YesNoDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void YesNoDialog::languageChange()
{
    setCaption( tr( "Confirm Action" ) );
    cancelButton->setText( tr( "Cancel" ) );
    question->setText( QString::null );
    okButton->setText( tr( "OK" ) );
}

void YesNoDialog::SetQuestion(QString text)
{
    question->setText(text);
}


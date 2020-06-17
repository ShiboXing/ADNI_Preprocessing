#include "editpointcoordinates.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qstring.h>

EditPointCoordinates::EditPointCoordinates( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
    setName( "EditPointCoordinates" );
    setSizeGripEnabled( TRUE );

    xLabel = new QLabel( this, "xLabel" );
    xLabel->setGeometry( QRect( 20, 20, 120, 30 ) );

    yLabel = new QLabel( this, "yLabel" );
    yLabel->setGeometry( QRect( 20, 80, 120, 30 ) );

    zLabel = new QLabel( this, "zLabel" );
    zLabel->setGeometry( QRect( 20, 140, 120, 30 ) );

    xCoordinate = new QLineEdit( this, "xCoordinate" );
    xCoordinate->setGeometry( QRect( 160, 20, 150, 30 ) );

    yCoordinate = new QLineEdit( this, "yCoordinate" );
    yCoordinate->setGeometry( QRect( 160, 80, 150, 30 ) );

    zCoordinate = new QLineEdit( this, "zCoordinate" );
    zCoordinate->setGeometry( QRect( 160, 140, 150, 30 ) );

    QWidget* privateLayoutWidget = new QWidget( this, "Layout1" );
    privateLayoutWidget->setGeometry( QRect( 10, 190, 300, 33 ) );
    Layout1 = new QHBoxLayout( privateLayoutWidget, 0, 6, "Layout1"); 
    Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing2 );

    buttonOk = new QPushButton( privateLayoutWidget, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    buttonCancel = new QPushButton( privateLayoutWidget, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );
    languageChange();
    resize( QSize(331, 235).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( xCoordinate, SIGNAL( lostFocus( ) ), this, SLOT( SetXCoordinate( ) ) );
    connect( yCoordinate, SIGNAL( lostFocus( ) ), this, SLOT( SetYCoordinate( ) ) );
    connect( zCoordinate, SIGNAL( lostFocus( ) ), this, SLOT( SetZCoordinate( ) ) );
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
 
    for (int i = 0; i < 3; i++)
    {
        pointCoordinates[i] = 0.0;
    }   
}

/*
 *  Destroys the object and frees any allocated resources
 */
EditPointCoordinates::~EditPointCoordinates()
{
    // no need to delete child widgets, Qt does it all for us
}

void EditPointCoordinates::SetPointCoordinates(vtkReal *pt)
{
    if (pt)
    {
        pointCoordinates[0] = pt[0];
        pointCoordinates[1] = pt[1];
        pointCoordinates[2] = pt[2];
        QString tmp =  QString("X (%1)").arg(pt[0]);
        xLabel->setText(tmp);
        tmp =  QString("Y (%1)").arg(pt[1]);
        yLabel->setText(tmp);
        tmp =  QString("Z (%1)").arg(pt[2]);
        zLabel->setText(tmp);
        tmp =  QString("%1").arg(pt[0]);
        xCoordinate->setText(tmp);
        tmp =  QString("%1").arg(pt[1]);
        yCoordinate->setText(tmp);
        tmp =  QString("%1").arg(pt[2]);
        zCoordinate->setText(tmp);
    }
}

void EditPointCoordinates::SetXCoordinate()
{
    QString coordText = xCoordinate->text();
    if (!(coordText.isNull() || coordText.isEmpty()))
        pointCoordinates[0] = xCoordinate->text().toDouble();
}

void EditPointCoordinates::SetYCoordinate()
{
    QString coordText = yCoordinate->text();
    if (!(coordText.isNull() || coordText.isEmpty()))
        pointCoordinates[0] = yCoordinate->text().toDouble();
}

void EditPointCoordinates::SetZCoordinate()
{
    QString coordText = zCoordinate->text();
    if (!(coordText.isNull() || coordText.isEmpty()))
        pointCoordinates[0] = zCoordinate->text().toDouble();
}

void EditPointCoordinates::GetPointCoordinates(vtkReal *pt)
{
    if (pt)
    {
        pt[0] = pointCoordinates[0];
        pt[1] = pointCoordinates[1];
        pt[2] = pointCoordinates[2];
    }
}

vtkReal * EditPointCoordinates::GetPointCoordinates()
{
    return pointCoordinates;
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void EditPointCoordinates::languageChange()
{
    setCaption( tr( "Edit Point Coordinates" ) );
    xLabel->setText( tr( "X (...)" ) );
    yLabel->setText( tr( "Y (...)" ) );
    zLabel->setText( tr( "Z (...)" ) );
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr( "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
}


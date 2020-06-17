/****************************************************************************
**
** Created: Wed Aug 24 12:55:35 2005
**      by: Anka Kochanowska at BIC MNI
**
** The number of RGB points never changes, the image range should not change neither
** 
**
**
****************************************************************************/

#include "editvolumetransferfunctionwidget.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qdir.h>
#include <qmessagebox.h>

#include "spectrumicons.h"
#include "ignsconfig.h"

#include "vtkCommand.h"
#include "vtkActor.h"
#include "vtkImageData.h"
#include "vtkVolumeProperty.h"
#include "vtkQtRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkVolumeTransferFunctionWidget.h"
#include "vtkColorTransferFunctionActor.h"
#include "vtkPiecewiseFunctionActor.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"

ObjectSerializationMacro( TransferFunctionSettings );


TransferFunctionSettings::TransferFunctionSettings()
{
    Range[0] = 0;
    Range[1] = 255;
    AlphaPointsNumber = 2;
    AlphaPoints[0][0] = Range[0];
    AlphaPoints[0][1] = 0.0;
    AlphaPoints[1][0] = Range[1] - Range[0];
    AlphaPoints[1][1] = 1.0;
    RgbPointsNumber = 6;
    RgbPoints[0][0] = Range[0];
    RgbPoints[0][1] = 0.0;
    RgbPoints[0][2] = 0.0;
    RgbPoints[0][3] = 0.0;
    RgbPoints[1][0] = (Range[1]-Range[0])/5.0;
    RgbPoints[1][1] = 0.2;
    RgbPoints[1][2] = 0.2;
    RgbPoints[1][3] = 0.2;
    RgbPoints[2][0] = (Range[1]-Range[0])*2.0/5.0;
    RgbPoints[2][1] = 0.4;
    RgbPoints[2][2] = 0.4;
    RgbPoints[2][3] = 0.4;
    RgbPoints[3][0] = (Range[1]-Range[0])*3.0/5.0;
    RgbPoints[3][1] = 0.6;
    RgbPoints[3][2] = 0.6;
    RgbPoints[3][3] = 0.6;
    RgbPoints[4][0] = (Range[1]-Range[0])*4.0/5.0;
    RgbPoints[4][1] = 0.8;
    RgbPoints[4][2] = 0.8;
    RgbPoints[4][3] = 0.8;
    RgbPoints[5][0] = Range[1]-Range[0];
    RgbPoints[5][1] = 1.0;
    RgbPoints[5][2] = 1.0;
    RgbPoints[5][3] = 1.0;
}

void TransferFunctionSettings::Serialize( Serializer * serializer )
{
    int i;
    char tmp[20];
    double r[2];
    r[0] = Range[0];
    r[1] = Range[1];
        ::Serialize( serializer, "Range", r, 2 ); // older systems complain if I pass Range directly, so this is work around, not elegant
    ::Serialize( serializer, "AlphaPointsNumber", AlphaPointsNumber );
    for (i = 0; i < AlphaPointsNumber; i++)
    {
        sprintf(tmp, "AlphaPoints_%i", i);
        ::Serialize( serializer, tmp, AlphaPoints[i], 2 );
    }
    ::Serialize( serializer, "RgbPointsNumber", AlphaPointsNumber );
    for (i = 0; i < RgbPointsNumber; i++)
    {
        sprintf(tmp, "RgbPoints_%i", i);
        ::Serialize( serializer, tmp, RgbPoints[i], 4 );
    }
}

/*
 *  Constructs a EditVolumeTransferFunctionWidget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
EditVolumeTransferFunctionWidget::EditVolumeTransferFunctionWidget( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl ),
      image0( (const char **) image0_data ),
      image1( (const char **) image1_data ),
      image2( (const char **) image2_data ),
      image3( (const char **) image3_data ),
      image4( (const char **) image4_data ),
      image5( (const char **) image5_data ),
      image6( (const char **) image6_data ),
      image7( (const char **) image7_data ),
      image8( (const char **) image8_data )
{
    if ( !name )
        setName( "EditVolumeTransferFunctionWidget" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 680, 460 ) );
    QFont f( font() );
    f.setFamily( "Helvetica" );
    setFont( f ); 

    QWidget* privateLayoutWidget = new QWidget( this, "mainLayout" );
    privateLayoutWidget->setGeometry( QRect( 0, 0, 680, 460 ) );
    mainLayout = new QVBoxLayout( privateLayoutWidget, 0, 0, "mainLayout"); 

    topLayout = new QHBoxLayout( 0, 0, 0, "topLayout");
     
    this->RenderWindow = new vtkQtRenderWindow( privateLayoutWidget, "EditVolumeTransferFunctionWidget" );
    this->RenderWindow->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, this->RenderWindow->sizePolicy().hasHeightForWidth() ) );
    this->RenderWindow->setMinimumSize( QSize( 600, 400 ) );
    this->RenderWindow->resizeGL(600, 400);
    topLayout->addWidget( this->RenderWindow );
    
    SelectSpectrumButtonGroup = new QButtonGroup( privateLayoutWidget, "SelectSpectrumButtonGroup" );
    SelectSpectrumButtonGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, SelectSpectrumButtonGroup->sizePolicy().hasHeightForWidth() ) );
    SelectSpectrumButtonGroup->setMinimumSize( QSize( 80, 400 ) );
    SelectSpectrumButtonGroup->setMaximumSize( QSize( 80, 400 ) );
    QFont SelectSpectrumButtonGroup_font(  SelectSpectrumButtonGroup->font() );
    SelectSpectrumButtonGroup->setFont( SelectSpectrumButtonGroup_font ); 
    SelectSpectrumButtonGroup->setFrameShape( QButtonGroup::WinPanel );
    SelectSpectrumButtonGroup->setFrameShadow( QButtonGroup::Raised );

    QWidget* privateLayoutWidget_2 = new QWidget( SelectSpectrumButtonGroup, "Colorlayout" );
    privateLayoutWidget_2->setGeometry( QRect( 10, 10, 62, 383 ) );
    Colorlayout = new QVBoxLayout( privateLayoutWidget_2, 0, 4, "Colorlayout"); 

    colorLabel = new QLabel( privateLayoutWidget_2, "colorLabel" );
    colorLabel->setMinimumSize( QSize( 60, 36 ) );
    colorLabel->setMaximumSize( QSize( 60, 36 ) );
    QFont colorLabel_font(  colorLabel->font() );
    colorLabel_font.setPointSize( 10 );
    colorLabel->setFont( colorLabel_font ); 
    colorLabel->setAlignment( int( QLabel::AlignCenter ) );
    Colorlayout->addWidget( colorLabel );

    GrayPushButton = new QPushButton( privateLayoutWidget_2, "GrayPushButton" );
    GrayPushButton->setMinimumSize( QSize( 0, 0 ) );
    GrayPushButton->setMaximumSize( QSize( 60, 30 ) );
    GrayPushButton->setPixmap( image0 );
    Colorlayout->addWidget( GrayPushButton );

    RedPushButton = new QPushButton( privateLayoutWidget_2, "RedPushButton" );
    RedPushButton->setMinimumSize( QSize( 0, 0 ) );
    RedPushButton->setMaximumSize( QSize( 60, 30 ) );
    RedPushButton->setPixmap( image1 );
    Colorlayout->addWidget( RedPushButton );

    OrangePushButton = new QPushButton( privateLayoutWidget_2, "OrangePushButton" );
    OrangePushButton->setMaximumSize( QSize( 60, 30 ) );
    OrangePushButton->setPixmap( image2 );
    Colorlayout->addWidget( OrangePushButton );

    YellowPushButton = new QPushButton( privateLayoutWidget_2, "YellowPushButton" );
    YellowPushButton->setMaximumSize( QSize( 60, 30 ) );
    YellowPushButton->setPixmap( image3 );
    Colorlayout->addWidget( YellowPushButton );

    GreenPushButton = new QPushButton( privateLayoutWidget_2, "GreenPushButton" );
    GreenPushButton->setMaximumSize( QSize( 60, 30 ) );
    GreenPushButton->setPixmap( image4 );
    Colorlayout->addWidget( GreenPushButton );

    BluePushButton = new QPushButton( privateLayoutWidget_2, "BluePushButton" );
    BluePushButton->setMaximumSize( QSize( 60, 30 ) );
    BluePushButton->setPixmap( image5 );
    Colorlayout->addWidget( BluePushButton );

    BlueToRedPushButton = new QPushButton( privateLayoutWidget_2, "BlueToRedPushButton" );
    BlueToRedPushButton->setMaximumSize( QSize( 60, 30 ) );
    BlueToRedPushButton->setPixmap( image6 );
    Colorlayout->addWidget( BlueToRedPushButton );

    FullPushButton = new QPushButton( privateLayoutWidget_2, "FullPushButton" );
    FullPushButton->setMaximumSize( QSize( 60, 30 ) );
    FullPushButton->setPixmap( image7 );
    Colorlayout->addWidget( FullPushButton );

    BlackToYellowPushButton = new QPushButton( privateLayoutWidget_2, "BlackToYellowPushButton" );
    BlackToYellowPushButton->setMaximumSize( QSize( 60, 30 ) );
    BlackToYellowPushButton->setPixmap( image8 );
    Colorlayout->addWidget( BlackToYellowPushButton );
    topLayout->addWidget( SelectSpectrumButtonGroup );
    mainLayout->addLayout( topLayout );

    FunctionButtonGroup = new QButtonGroup( privateLayoutWidget, "FunctionButtonGroup" );
    FunctionButtonGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, FunctionButtonGroup->sizePolicy().hasHeightForWidth() ) );
    FunctionButtonGroup->setMinimumSize( QSize( 680, 60 ) );
    FunctionButtonGroup->setFrameShape( QButtonGroup::WinPanel );
    FunctionButtonGroup->setFrameShadow( QButtonGroup::Raised );
    FunctionButtonGroup->setAlignment( int( QButtonGroup::AlignCenter ) );
    FunctionButtonGroup->setColumnLayout(0, Qt::Vertical );
    FunctionButtonGroup->layout()->setSpacing( 6 );
    FunctionButtonGroup->layout()->setMargin( 11 );
    FunctionButtonGroupLayout = new QHBoxLayout( FunctionButtonGroup->layout() );
    FunctionButtonGroupLayout->setAlignment( Qt::AlignTop );

    FunctionLayout = new QHBoxLayout( 0, 0, 0, "FunctionLayout"); 
    spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    FunctionLayout->addItem( spacer2 );

    SavePushButton = new QPushButton( FunctionButtonGroup, "SavePushButton" );
    SavePushButton->setMinimumSize( QSize( 64, 30 ) );
    QFont SavePushButton_font(  SavePushButton->font() );
    SavePushButton_font.setPointSize( 11 );
    SavePushButton->setFont( SavePushButton_font ); 
    FunctionLayout->addWidget( SavePushButton );
    spacer10 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    FunctionLayout->addItem( spacer10 );

    LoadPushButton = new QPushButton( FunctionButtonGroup, "LoadPushButton" );
    LoadPushButton->setMinimumSize( QSize( 64, 30 ) );
    QFont LoadPushButton_font(  LoadPushButton->font() );
    LoadPushButton_font.setPointSize( 11 );
    LoadPushButton->setFont( LoadPushButton_font ); 
    FunctionLayout->addWidget( LoadPushButton );
    spacer9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    FunctionLayout->addItem( spacer9 );

    ResetPushButton = new QPushButton( FunctionButtonGroup, "ResetPushButton" );
    ResetPushButton->setMinimumSize( QSize( 64, 30 ) );
    QFont ResetPushButton_font(  ResetPushButton->font() );
    ResetPushButton_font.setPointSize( 11 );
    ResetPushButton->setFont( ResetPushButton_font ); 
    FunctionLayout->addWidget( ResetPushButton );
    spacer1 = new QSpacerItem( 140, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    FunctionLayout->addItem( spacer1 );

    CancelPushButton = new QPushButton( FunctionButtonGroup, "CancelPushButton" );
    CancelPushButton->setMinimumSize( QSize( 64, 30 ) );
    QFont CancelPushButton_font(  CancelPushButton->font() );
    CancelPushButton_font.setPointSize( 11 );
    CancelPushButton->setFont( CancelPushButton_font ); 
    FunctionLayout->addWidget( CancelPushButton );
    spacer4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    FunctionLayout->addItem( spacer4 );

    OkPushButton = new QPushButton( FunctionButtonGroup, "OkPushButton" );
    OkPushButton->setMinimumSize( QSize( 64, 30 ) );
    QFont OkPushButton_font(  OkPushButton->font() );
    OkPushButton_font.setPointSize( 11 );
    OkPushButton->setFont( OkPushButton_font ); 
    FunctionLayout->addWidget( OkPushButton );
    spacer3 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    FunctionLayout->addItem( spacer3 );
    FunctionButtonGroupLayout->addLayout( FunctionLayout );
    mainLayout->addWidget( FunctionButtonGroup );
    languageChange();
    resize( QSize(680, 460).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( GrayPushButton, SIGNAL( clicked() ), this, SLOT( GrayPushButtonClicked() ) );
    connect( RedPushButton, SIGNAL( clicked() ), this, SLOT( RedPushButtonClicked() ) );
    connect( OrangePushButton, SIGNAL( clicked() ), this, SLOT( OrangePushButtonClicked() ) );
    connect( YellowPushButton, SIGNAL( clicked() ), this, SLOT( YellowPushButtonClicked() ) );
    connect( GreenPushButton, SIGNAL( clicked() ), this, SLOT( GreenPushButtonClicked() ) );
    connect( BluePushButton, SIGNAL( clicked() ), this, SLOT( BluePushButtonClicked() ) );
    connect( BlueToRedPushButton, SIGNAL( clicked() ), this, SLOT( BlueToRedPushButtonClicked() ) );
    connect( FullPushButton, SIGNAL( clicked() ), this, SLOT( FullPushButtonClicked() ) );
    connect( BlackToYellowPushButton, SIGNAL( clicked() ), this, SLOT( BlackToYellowPushButtonClicked() ) );
    connect( OkPushButton, SIGNAL( clicked() ), this, SLOT( SaveAndClose() ) );
    connect( CancelPushButton, SIGNAL( clicked() ), this, SLOT( ResetAndCancel() ) );
    connect( ResetPushButton, SIGNAL( clicked() ), this, SLOT( ResetSettings() ) );
    connect( LoadPushButton, SIGNAL( clicked() ), this, SLOT( LoadSettings() ) );
    connect( SavePushButton, SIGNAL( clicked() ), this, SLOT( SaveSettings() ) );

    // tab order
    setTabOrder( GrayPushButton, RedPushButton );
    setTabOrder( RedPushButton, OrangePushButton );
    setTabOrder( OrangePushButton, YellowPushButton );
    setTabOrder( YellowPushButton, GreenPushButton );
    setTabOrder( GreenPushButton, BluePushButton );
    setTabOrder( BluePushButton, BlueToRedPushButton );
    setTabOrder( BlueToRedPushButton, FullPushButton );
    setTabOrder( FullPushButton, BlackToYellowPushButton );
    setTabOrder( BlackToYellowPushButton, SavePushButton );
    setTabOrder( SavePushButton, LoadPushButton );
    setTabOrder( LoadPushButton, ResetPushButton );
    setTabOrder( ResetPushButton, CancelPushButton );
    setTabOrder( CancelPushButton, OkPushButton );
    
    this->settings = new TransferFunctionSettings();
    this->VolumeTransferFunctionWidget = vtkVolumeTransferFunctionWidget::New();
}

/*
 *  Destroys the object and frees any allocated resources
 */
EditVolumeTransferFunctionWidget::~EditVolumeTransferFunctionWidget()
{
    this->VolumeTransferFunctionWidget->Delete();
    if (this->SaveAlphaFunc)
        this->SaveAlphaFunc->Delete( );
    if (this->SaveRgbFunc)
        this->SaveRgbFunc->Delete( );
    delete this->settings;
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void EditVolumeTransferFunctionWidget::languageChange()
{
    setCaption( tr( "Edit Volume Transfer Function Widget" ) );
    SelectSpectrumButtonGroup->setTitle( QString::null );
    colorLabel->setText( tr( "Select\n"
"Colors" ) );
    GrayPushButton->setText( QString::null );
    RedPushButton->setText( QString::null );
    OrangePushButton->setText( QString::null );
    YellowPushButton->setText( QString::null );
    GreenPushButton->setText( QString::null );
    BluePushButton->setText( QString::null );
    BlueToRedPushButton->setText( QString::null );
    FullPushButton->setText( QString::null );
    BlackToYellowPushButton->setText( QString::null );
    FunctionButtonGroup->setTitle( QString::null );
    SavePushButton->setText( tr( "Save" ) );
    LoadPushButton->setText( tr( "Load" ) );
    ResetPushButton->setText( tr( "Reset" ) );
    CancelPushButton->setText( tr( "Cancel" ) );
    OkPushButton->setText( tr( "OK" ) );
}

void EditVolumeTransferFunctionWidget::SaveSettings()
{
    QString configDir( QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY );
    if( !QFile::exists( configDir ) )
    {
        QDir dir( configDir );
        if( !dir.exists( ) )
            dir.mkdir( configDir );
    }
    QString name( configDir + "/" + IGNS_TRANSFER_FUNCTION_SETTINGS );
    this->UpdateSettings();
    // write a new config
    SerializerWriter writer;
    writer.SetFilename( name );
    writer.Start();
    ::Serialize( &writer, "TransferFunctionSettings", settings );
    writer.Finish();
}

void EditVolumeTransferFunctionWidget::SaveAndClose()
{
    this->SaveSettings();
    close(TRUE);
}

void EditVolumeTransferFunctionWidget::LoadSettings()
{
    int i;
    vtkPiecewiseFunction * alphaFunc = this->VolumeTransferFunctionWidget->GetAlphaActor()->GetFunction();
    vtkColorTransferFunction * rgbFunc = this->VolumeTransferFunctionWidget->GetColorActor()->GetFunction();
    QString name( QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY + "/" + IGNS_TRANSFER_FUNCTION_SETTINGS );
    if( QFile::exists( name ) )
    {
        SerializerReader reader;
        reader.SetFilename( name );
        reader.Start();
        Serialize( &reader, "TransferFunctionSettings", settings );
        reader.Finish();
    }
    if (this->settings->Range[0] != this->ImageRange[0] || this->settings->Range[1] != this->ImageRange[1])
    {
        QMessageBox::warning( this, "Error", "Image range may not be changed.", 1, 0 );
        this->UpdateSettings();
        return;
    }
    alphaFunc->RemoveAllPoints();
    rgbFunc->RemoveAllPoints();
    for (i = 0; i < settings->AlphaPointsNumber; i++)
            alphaFunc->AddPoint(settings->AlphaPoints[i][0], settings->AlphaPoints[i][1]);
    for (i = 0; i < settings->RgbPointsNumber; i++)
            rgbFunc->AddRGBPoint(settings->RgbPoints[i][0], settings->RgbPoints[i][1], settings->RgbPoints[i][2], settings->RgbPoints[i][3]);
    this->RenderWindow->Render();
    this->ImageRange[0] = this->settings->Range[0];
    this->ImageRange[1] = this->settings->Range[1];
}

void EditVolumeTransferFunctionWidget::InitialLoadSettings()
{
    int i;
    vtkPiecewiseFunction * alphaFunc = this->VolumeTransferFunctionWidget->GetAlphaActor()->GetFunction();
    vtkColorTransferFunction * rgbFunc = this->VolumeTransferFunctionWidget->GetColorActor()->GetFunction();
    QString name( QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY + "/" + IGNS_TRANSFER_FUNCTION_SETTINGS );
    if( QFile::exists( name ) )
    {
        SerializerReader reader;
        reader.SetFilename( name );
        reader.Start();
        Serialize( &reader, "TransferFunctionSettings", settings );
        reader.Finish();
    }
    for (i = 0; i < settings->AlphaPointsNumber; i++)
            alphaFunc->AddPoint(settings->AlphaPoints[i][0], settings->AlphaPoints[i][1]);
    for (i = 0; i < settings->RgbPointsNumber; i++)
            rgbFunc->AddRGBPoint(settings->RgbPoints[i][0], settings->RgbPoints[i][1], settings->RgbPoints[i][2], settings->RgbPoints[i][3]);
    this->ImageRange[0] = this->settings->Range[0];
    this->ImageRange[1] = this->settings->Range[1];
}

void EditVolumeTransferFunctionWidget::ResetSettings()
{
    vtkPiecewiseFunction * alphaFunc = this->VolumeTransferFunctionWidget->GetAlphaActor()->GetFunction();
    vtkColorTransferFunction * rgbFunc = this->VolumeTransferFunctionWidget->GetColorActor()->GetFunction();
    alphaFunc->DeepCopy( this->SaveAlphaFunc );
    rgbFunc->DeepCopy( this->SaveRgbFunc );
    this->RenderWindow->Render();
    this->UpdateSettings();
}

void EditVolumeTransferFunctionWidget::ResetAndCancel()
{
    this->ResetSettings();
    this->SaveSettings();
    close(TRUE);
}

void EditVolumeTransferFunctionWidget::GrayPushButtonClicked()
{
    this->SetSelectedSpectrum(Gray);
}

void EditVolumeTransferFunctionWidget::RedPushButtonClicked()
{
    this->SetSelectedSpectrum(Red);
}

void EditVolumeTransferFunctionWidget::OrangePushButtonClicked()
{
    this->SetSelectedSpectrum(Orange);
}

void EditVolumeTransferFunctionWidget::YellowPushButtonClicked()
{
    this->SetSelectedSpectrum(Yellow);
}

void EditVolumeTransferFunctionWidget::GreenPushButtonClicked()
{
    this->SetSelectedSpectrum(Green);
}

void EditVolumeTransferFunctionWidget::BlackToYellowPushButtonClicked()
{
    this->SetSelectedSpectrum(BlackToYellow);
}

void EditVolumeTransferFunctionWidget::BlueToRedPushButtonClicked()
{
    this->SetSelectedSpectrum(BlueToRed);
}

void EditVolumeTransferFunctionWidget::FullPushButtonClicked()
{
    this->SetSelectedSpectrum(Full);
}

void EditVolumeTransferFunctionWidget::BluePushButtonClicked()
{
    this->SetSelectedSpectrum(Blue);
}

void EditVolumeTransferFunctionWidget::Init()
{
    vtkRenderer *renderer= vtkRenderer::New();
    this->RenderWindow->AddRenderer( renderer );
    vtkRenderWindowInteractor * interactor = this->RenderWindow->MakeRenderWindowInteractor();
    this->VolumeTransferFunctionWidget->SetInteractor(interactor);
    this->VolumeTransferFunctionWidget->EnabledOn();
    interactor->SetRenderWindow(this->RenderWindow);
    interactor->Initialize();
    renderer->Delete();
    // set SaveAlphaFunc and SaveRgbFunc for restore of the initial state
    this->SaveAlphaFunc= vtkPiecewiseFunction::New();
    this->SaveRgbFunc = vtkColorTransferFunction::New();
    this->SaveAlphaFunc->DeepCopy( this->VolumeTransferFunctionWidget->GetAlphaActor()->GetFunction() ); 
    this->SaveRgbFunc->DeepCopy( this->VolumeTransferFunctionWidget->GetColorActor()->GetFunction() ); 
}

void EditVolumeTransferFunctionWidget::InitVolumeTransferFunction()
{
    this->InitialLoadSettings();
    this->Init();
}

void EditVolumeTransferFunctionWidget::InitVolumeTransferFunction(vtkVolumeProperty * volumeProperty)
{
    this->VolumeTransferFunctionWidget->GetAlphaActor()->SetFunction(volumeProperty->GetScalarOpacity());
    this->VolumeTransferFunctionWidget->GetColorActor()->SetFunction(volumeProperty->GetRGBTransferFunction());
    volumeProperty->GetRGBTransferFunction()->GetRange(this->ImageRange);
    this->UpdateSettings();
    this->Init();
}

void EditVolumeTransferFunctionWidget::SetSelectedSpectrum(ColorSpectrum spectrum)
{
    vtkColorTransferFunction * rgbFunc = this->VolumeTransferFunctionWidget->GetColorActor()->GetFunction();
    rgbFunc->RemoveAllPoints();
    switch(spectrum)
    {
        default:
        case Gray:
            rgbFunc->AddRGBPoint(ImageRange[0], 0.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])/5.0, 0.2, 0.2, 0.2);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*2.0/5.0, 0.4, 0.4, 0.4);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*3.0/5.0, 0.6, 0.6, 0.6);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*4.0/5.0, 0.8, 0.8, 0.8);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0]), 1.0, 1.0, 1.0);
            break;
        case Red:
            rgbFunc->AddRGBPoint(ImageRange[0], 0.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*1.0/5.0, 0.3, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*2.0/5.0, 0.4, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*3.0/5.0, 0.6, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*4.0/5.0, 0.8, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0]), 1.0, 0.0, 0.0);
            break;
        case Orange:
            rgbFunc->AddRGBPoint(ImageRange[0], 0.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*1.0/5.0, 0.950757, 0.481702, 0.0571550);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*2.0/5.0, 1.0, 0.534703, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*3.0/5.0, 1.0, 0.534703, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*4.0/5.0, 1.0, 0.713648, 0.05);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0]), 1.0, 0.892593, 0.01);
            break;
        case Yellow:
            rgbFunc->AddRGBPoint(ImageRange[0], 0.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*1.0/5.0, 0.797558, 0.633333, 0.08129);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*2.0/5.0, 0.707558, 0.733333, 0.18129);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*3.0/5.0, 0.697558, 0.833333, 0.28129);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*4.0/5.0, 0.598779, 0.896667, 0.356399);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0]), 0.000000, 1.000000, 0.531509);
            break;
        case Green:
            rgbFunc->AddRGBPoint(ImageRange[0], 0.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*1.0/5.0, 0.0, 1.0, 0.3);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*2.0/5.0, 0.0, 1.0, 0.4);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*3.0/5.0, 0.0, 1.0, 0.534703);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*4.0/5.0, 0.0, 1.0, 0.713648);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0]), 0.0, 1.0, 0.892593);
            break;
        case Blue:
            rgbFunc->AddRGBPoint(ImageRange[0], 0, 0, 0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*1.0/5.0, 0.05, 0.0, 0.95075);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*2.0/5.0, 0.1, 0.0, 1.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*3.0/5.0, 0.2, 0.0, 1.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*4.0/5.0, 0.4, 0.0, 1.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0]), 0.0, 0.892593, 1.0);
            break;
        case BlueToRed:
            rgbFunc->AddRGBPoint(ImageRange[0], 0.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*1.0/5.0, 0.25, 0.25, 1.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*2.0/5.0, 0.5, 0.5, 0.5);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*3.0/5.0, 1.0, 1.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*4.0/5.0, 1.0, 0.5, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0]), 1.0, 0.0, 0.0);
            break;
        case Full:
            rgbFunc->AddRGBPoint(ImageRange[0], 0.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*1.0/5.0, 1.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*2.0/5.0, 0.5, 0.0, 0.75);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*3.0/5.0, 0.0, 0.75, 0.55);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*4.0/5.0, 1.0, 1.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0]), 1.0, 0.0, 0.0);
            break;
        case BlackToYellow:
            rgbFunc->AddRGBPoint(ImageRange[0], 0.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*1.0/5.0, 0.5, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*2.0/5.0, 1.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*3.0/5.0, 1.0, 0.0, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0])*4.0/5.0, 1.0, 0.52, 0.0);
            rgbFunc->AddRGBPoint((ImageRange[1]-ImageRange[0]), 1.0, 1.0, 0.0);
            break;
    }
    this->UpdateSettings();
    this->RenderWindow->Render();
}

void EditVolumeTransferFunctionWidget::UpdateSettings()
{
    int i;
    vtkPiecewiseFunction * alphaFunc = this->VolumeTransferFunctionWidget->GetAlphaActor()->GetFunction();
    vtkColorTransferFunction * rgbFunc = this->VolumeTransferFunctionWidget->GetColorActor()->GetFunction();
    vtkIndent indent;
    vtkReal *funcData = alphaFunc->GetDataPointer();
    
    this->settings->Range[0] = this->ImageRange[0];
    this->settings->Range[1] = this->ImageRange[1];
    this->settings->AlphaPointsNumber = alphaFunc->GetSize();
    if (this->settings->AlphaPointsNumber > 20)
        this->settings->AlphaPointsNumber = 20;
    this->settings->RgbPointsNumber = rgbFunc->GetSize();
    if (this->settings->RgbPointsNumber > 20)
        this->settings->RgbPointsNumber = 20;
    for( i = 0; i < settings->AlphaPointsNumber; i++ )
    {
        this->settings->AlphaPoints[i][0] = funcData[2*i];
        this->settings->AlphaPoints[i][1] = funcData[2*i+1];
    }
    funcData = rgbFunc->GetDataPointer();
    for ( i = 0; i < this->settings->RgbPointsNumber; i++ )
    {
        this->settings->RgbPoints[i][0] = funcData[i*4];
        this->settings->RgbPoints[i][1] = funcData[i*4+1];
        this->settings->RgbPoints[i][2] = funcData[i*4+2];
        this->settings->RgbPoints[i][3] = funcData[i*4+3];
    }
}

#include "vtkQtMatrixDialog.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qdir.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstring.h>
#include <qmessagebox.h>
#include "vtkObject.h"
#include "vtkMatrix4x4.h"
#include "vtkQtAsyncCallback.h"
#include "vtkXFMReader.h"
#include "vtkXFMWriter.h"

#include "qtextfiledialog.h"
#include "ignsconfig.h"
#include "ignsmsg.h"

vtkQtMatrixDialog::vtkQtMatrixDialog( vtkMatrix4x4 * mat, bool readOnly, QWidget* parent,  const char* name )
    : QDialog( parent, name, Qt::WDestructiveClose | Qt::WStyle_StaysOnTop )
{
    m_matrixChangedCallback = vtkQtAsyncCallback::New();
    connect( m_matrixChangedCallback, SIGNAL( EventTriggered() ), this, SLOT( MatrixChanged() ) );
    
    m_matrix = mat;
    m_copy_matrix = vtkMatrix4x4::New();
    m_copy_matrix->Identity();
    if( m_matrix )
    {
        m_matrix->Register( NULL );
        m_matrix->AddObserver( vtkCommand::ModifiedEvent, m_matrixChangedCallback );
    }
    
    if ( !name )
        setName( "matrix" );
    resize( 511, 210 ); 

    if( !name )
        setCaption( "Matrix" );
    else
        setCaption( name );
    
    MatrixDialogLayout = new QVBoxLayout( this ); 
    MatrixDialogLayout->setSpacing( 6 );
    MatrixDialogLayout->setMargin( 11 );

    gridBox = new QGridLayout; 
    gridBox->setSpacing( 12 );
    gridBox->setMargin( 2 );

    // Create the text boxes for every matrix entry
    for( int i = 0; i < 4; i++ )
    {
        for( int j = 0; j < 4; j++ )
        {
            m_matEdit[i][j] = new QLineEdit( this );
            m_matEdit[i][j]->setReadOnly( readOnly );
            m_matEdit[i][j]->setAlignment( Qt::AlignRight );
            m_matEdit[i][j]->setMaxLength( 12 );
            gridBox->addWidget( m_matEdit[i][j], i, j );
        }
    }

    MatrixDialogLayout->addLayout( gridBox );
    
    // create the box that contains the ok and cancel buttons
    QSpacerItem * spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    MatrixDialogLayout->addItem( spacer );

    Layout3 = new QHBoxLayout; 
    Layout3->setSpacing( 6 );
    Layout3->setMargin( 0 );
    
    m_identityButton = new QPushButton( this, "m_identityButton" );
    m_identityButton->setText( tr( "Identity" ) );
    m_identityButton->setMinimumSize( QSize( 70, 30 ) );
    m_identityButton->setMaximumSize( QSize( 70, 30 ) );
    m_identityButton->setEnabled(!readOnly);
    Layout3->addWidget( m_identityButton );
    
    m_invertButton = new QPushButton( this, "m_invertButton" );
    m_invertButton->setText( tr( "Invert" ) );
    m_invertButton->setMinimumSize( QSize( 70, 30 ) );
    m_invertButton->setMaximumSize( QSize( 70, 30 ) );
    m_invertButton->setEnabled(!readOnly);
    Layout3->addWidget( m_invertButton );
    
    m_loadButton = new QPushButton( this, "m_loadButton" );
    m_loadButton->setText( tr( "Load" ) );
    m_loadButton->setMinimumSize( QSize( 70, 30 ) );
    m_loadButton->setMaximumSize( QSize( 70, 30 ) );
    m_loadButton->setEnabled(!readOnly);
    Layout3->addWidget( m_loadButton );
    
    m_saveButton = new QPushButton( this, "m_loadButton" );
    m_saveButton->setText( tr( "Save" ) );
    m_saveButton->setMinimumSize( QSize( 70, 30 ) );
    m_saveButton->setMaximumSize( QSize( 70, 30 ) );
    Layout3->addWidget( m_saveButton );
    
    m_revertButton = new QPushButton( this, "m_revertButton" );
    m_revertButton->setText( tr( "Restore" ) );
    m_revertButton->setMinimumSize( QSize( 70, 30 ) );
    m_revertButton->setMaximumSize( QSize( 70, 30 ) );
    m_revertButton->setEnabled(!readOnly);
    Layout3->addWidget( m_revertButton );
    
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout3->addItem( spacer_2 );
    
    m_okButton = new QPushButton( this, "m_okButton" );
    m_okButton->setText( tr( "OK" ) );
    m_okButton->setMinimumSize( QSize( 70, 30 ) );
    m_okButton->setMaximumSize( QSize( 70, 30 ) );
    Layout3->addWidget( m_okButton );
    
    MatrixDialogLayout->addLayout( Layout3 );
    
    // signals and slots connections
    connect( m_invertButton, SIGNAL( clicked() ), this, SLOT( InvertButtonClicked() ) );
    connect( m_identityButton, SIGNAL( clicked() ), this, SLOT( IdentityButtonClicked() ) );
    connect( m_revertButton, SIGNAL( clicked() ), this, SLOT( RevertButtonClicked() ) );
    connect( m_okButton, SIGNAL( clicked() ), this, SLOT( OKButtonClicked() ) );
    connect( m_loadButton, SIGNAL( clicked() ), this, SLOT( LoadButtonClicked() ) );
    connect( m_saveButton, SIGNAL( clicked() ), this, SLOT( SaveButtonClicked() ) );
    UpdateUI();
    m_directory = QDir::homeDirPath();
}

vtkQtMatrixDialog::~vtkQtMatrixDialog()
{
    if( m_matrix )
    {
        m_matrix->UnRegister( NULL );
        m_matrix->RemoveObserver( m_matrixChangedCallback );
    }
    if( m_copy_matrix )
    {
        m_copy_matrix->Delete();
    }
    m_matrixChangedCallback->Delete();
}

void vtkQtMatrixDialog::SetMatrixElements( )
{
    if( m_matrix )
    {
        for( int i = 0; i < 4; i++ )
        {
            for( int j = 0; j < 4; j++ )
            {
                m_matrix->SetElement( i, j, m_matEdit[i][j]->text().toDouble() );
            }
        }
    }
}

void vtkQtMatrixDialog::InvertButtonClicked()
{
    m_copy_matrix->DeepCopy( m_matrix );
    SetMatrixElements( );
    m_matrix->Invert();
    UpdateUI();
}


void vtkQtMatrixDialog::IdentityButtonClicked()
{
    QString one = QString::number( (double)1.0 );
    QString zero = QString::number( (double)0.0 );
    m_copy_matrix->DeepCopy( m_matrix );
    for( int i = 0; i < 4; i++ )
    {
        for( int j = 0; j < 4; j++ )
        {
            if( i == j )
            {
                m_matEdit[i][j]->setText( one );
            }
            else
            {
                m_matEdit[i][j]->setText( zero );
            }
        }
    }
}


void vtkQtMatrixDialog::RevertButtonClicked()
{
    m_matrix->DeepCopy( m_copy_matrix );
    UpdateUI();
}


void vtkQtMatrixDialog::MatrixChanged()
{
    UpdateUI();
}


void vtkQtMatrixDialog::SetMatrix( vtkMatrix4x4 * mat )
{
    if( m_matrix )
    {
        m_copy_matrix->DeepCopy( m_matrix );
        m_matrix->UnRegister( NULL );
        m_matrix->RemoveObserver( m_matrixChangedCallback );
    }
    m_matrix = mat;
    if( m_matrix )
    {
        m_matrix->Register( NULL );
        m_matrix->AddObserver( vtkCommand::ModifiedEvent, m_matrixChangedCallback );
    }
    UpdateUI();   
}


vtkMatrix4x4 * vtkQtMatrixDialog::GetMatrix( )
{
    return m_matrix;
}


void vtkQtMatrixDialog::UpdateUI()
{
    if( m_matrix )
    {
        for( int i = 0; i < 4; i++ )
        {
            for( int j = 0; j < 4; j++ )
            {
                m_matEdit[i][j]->setText( QString::number( m_matrix->Element[i][j], 'g', 15 ) );
            }
        }
    }
    else
    {
        for( int i = 0; i < 4; i++ )
        {
            for( int j = 0; j < 4; j++ )
            {
                m_matEdit[i][j]->setText( "----" );
            }
        }
    }
}

void vtkQtMatrixDialog::OKButtonClicked()
{
    SetMatrixElements( );
    accept();
}

void vtkQtMatrixDialog::LoadButtonClicked()
{
    QFile *OpenFile;
    if( !QFile::exists( m_directory ) )
    {
        m_directory = QDir::homeDirPath();
    }
    QtExtFileDialog *fdialog = new QtExtFileDialog( this,  "Open File", m_directory, TRUE);
    OpenFile = fdialog->GetOpenFile();
    if (OpenFile)
    {
        int type = GetFileType(OpenFile);
        if (type == TEXT_FILE)
        {
            LoadFromTextFile(OpenFile);
        }
        else if (type == XFM_FILE)
        {
            LoadFromXFMFile(OpenFile); // I'm not checking the return because it would be redundant. GetType()  would fail first.
        }
        else
        {
            QMessageBox::warning( this, "Error: ", "Unknown file format.", 1, 0 );
        }
        QFileInfo info( OpenFile->name() );
        m_directory = info.dirPath( true );
    }
    else
    {
        int err = fdialog->GetOpenError();
        if (err != FA_NO_ERROR)
        {
            QString accessError = IGNS_MSG_CANT_OPEN + fdialog->GetOpenFileName();
            QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
        }
    }
    delete fdialog;
 }

void vtkQtMatrixDialog::SaveButtonClicked()
{
    QFile *OpenFile;
    if( !QFile::exists( m_directory ) )
    {
        m_directory = QDir::homeDirPath();
    }
    QtExtFileDialog *fdialog = new QtExtFileDialog( this, "Save File", m_directory, FALSE);
    OpenFile = fdialog->GetOpenFile();
    if (OpenFile)
    {
        vtkXFMWriter *writer = vtkXFMWriter::New();
        writer->SetFileName(OpenFile->name());
        writer->SetMatrix(m_matrix);
        writer->Write();
        writer->Delete();
        QFileInfo info( OpenFile->name() );
        m_directory = info.dirPath( true );
    }
    else
    {
        int err = fdialog->GetOpenError();
        if (err != FA_NO_ERROR)
        {
            QString accessError = IGNS_MSG_CANT_OPEN + fdialog->GetOpenFileName();
            QMessageBox::warning( 0, "Error: ", accessError, 1, 0 );
        }
    }
    delete fdialog;
}

int vtkQtMatrixDialog::GetFileType(QFile *f)
{
    char line[256];
    if (!f->open(IO_ReadOnly))
    {
        QMessageBox::warning( this, "Read error: can't open file" , f->name(), 1, 0 );
        return UNKNOWN_FILE_TYPE;
    }
    line[0] = 0;
    f->readLine( line, 256 );
    f->close();
    if ( isdigit(line[0]) || (line[0] == '-') )
        return TEXT_FILE;
    else
    {
        if( strncmp( line, "MNI Transform File", 18 ) == 0 )
            return XFM_FILE;
    }
    return UNKNOWN_FILE_TYPE;
}

bool vtkQtMatrixDialog::LoadFromXFMFile( QFile *f )
{
    m_copy_matrix->DeepCopy( m_matrix );
    vtkXFMReader *reader = vtkXFMReader::New();
    if( reader->CanReadFile( f->name() ) )
    {    
        reader->SetFileName(f->name());
        reader->SetMatrix(m_copy_matrix);
        reader->Update();
        reader->Delete();    
        for( int i = 0; i < 4; i++ )
        {
            for( int j = 0; j < 4; j++ )
            {
                m_matEdit[i][j]->setText( QString::number( m_copy_matrix->Element[i][j], 'g', 15 ) );
            }
        }
        return true;
    }
    reader->Delete();    
    return false;
}

void vtkQtMatrixDialog::LoadFromTextFile(QFile *f)
{
    char line[256], *tok;
    int ok = 1;
    if (!f->open(IO_ReadOnly))
    {
        QMessageBox::warning( this, "Read error: can't open file" , f->name(), 1, 0 );
        return;
    }
    line[0] = 0;
    f->readLine( line, 256 );
    f->close();
    
    if( m_matrix )
    {
        m_copy_matrix->DeepCopy( m_matrix );
        vtkMatrix4x4 * tmp_matrix = vtkMatrix4x4::New();
        tok = strtok( line, " \t" );
        
        for( int i = 0; i < 4 && ok; i++ )
        {
            for( int j = 0; j < 4 && ok; j++ )
            {
                if ( tok )
                {
                    tmp_matrix->SetElement( i, j, atof(tok) );
                    tok = strtok( NULL, " \t" );
                }
                else 
                    ok = 0;
            }
        }
        if ( ok )
        {
            for( int i = 0; i < 4; i++ )
            {
                for( int j = 0; j < 4; j++ )
                {
                    m_matEdit[i][j]->setText( QString::number( tmp_matrix->Element[i][j], 'g', 15 ) );
                }
            }
        }
        tmp_matrix->Delete();
    }
}

void vtkQtMatrixDialog::SetDirectory(const QString &dir)
{
    m_directory = dir;
    if( !QFile::exists( m_directory ) )
    {
        m_directory = QDir::homeDirPath();
    }
}

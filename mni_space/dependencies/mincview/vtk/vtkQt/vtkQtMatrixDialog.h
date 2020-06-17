#ifndef MATRIXDIALOG_H
#define MATRIXDIALOG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLineEdit;
class QPushButton;
class QFile;
class vtkMatrix4x4;
class vtkQtAsyncCallback;

enum FILETYPE {UNKNOWN_FILE_TYPE, TEXT_FILE, XFM_FILE, XML_FILE};

class vtkQtMatrixDialog : public QDialog
{ 
    Q_OBJECT

public:

    vtkQtMatrixDialog( vtkMatrix4x4 * mat, bool readOnly, QWidget * parent = 0, const char * name = 0 );
    ~vtkQtMatrixDialog();

    void SetMatrix( vtkMatrix4x4 * mat );
    vtkMatrix4x4 * GetMatrix( );
    void SetDirectory(const QString &dir);
    void UpdateUI();

    QLineEdit   * m_matEdit[4][4];
    QPushButton * m_invertButton;
    QPushButton * m_identityButton;
    QPushButton * m_revertButton;
    QPushButton * m_okButton;
    QPushButton * m_loadButton;
    QPushButton * m_saveButton;
        
public slots:
    
    void InvertButtonClicked();
    void IdentityButtonClicked();
    void RevertButtonClicked();
    void MatrixChanged();
    void OKButtonClicked();
    void LoadButtonClicked();
    void SaveButtonClicked();

protected:

    vtkMatrix4x4 * m_matrix;
    vtkMatrix4x4 * m_copy_matrix;
    QString m_directory;
    
    vtkQtAsyncCallback * m_matrixChangedCallback;
    
    QVBoxLayout * MatrixDialogLayout;
    QGridLayout * gridBox;
    QHBoxLayout * Layout3;
    

    void  SetMatrixElements( );
    int   GetFileType(QFile *f); 
    bool  LoadFromXFMFile(QFile *f);
    void  LoadFromTextFile(QFile *f);
};


#endif

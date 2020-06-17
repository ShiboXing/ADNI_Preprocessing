#ifndef QtExtFileDialog_h_
#define QtExtFileDialog_h_

#include <qwidget.h>
#include <qfile.h>

class QString;
class QFile;

enum FILE_ACCESS_ERROR {FA_NO_ERROR, FA_NO_FILE, FA_NO_READ, FA_NO_WRITE, FA_NO_EXEC, FA_EXISTS};

class QtExtFileDialog : public QWidget
{
    Q_OBJECT

public:
    
    QtExtFileDialog( QWidget* parent = 0, const char* dialog_name = 0, QString directory = 0, bool open_file = TRUE);
    ~QtExtFileDialog();
    
    QFile *GetOpenFile();
    int GetOpenError() {return OpenError;}
    QString GetOpenFileName() {return OpenFileName;}
    
public slots:
    

protected:
    QFile *OpenFile;
    int OpenError;
    QString OpenFileName;
};

#endif //QtExtFileDialog_h_

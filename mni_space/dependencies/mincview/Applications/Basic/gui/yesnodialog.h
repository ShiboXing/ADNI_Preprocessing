#ifndef TAG_YESNODIALOGDIALOG_H
#define TAG_YESNODIALOGDIALOG_H

#include <qvariant.h>
#include <qdialog.h>

class QPushButton;
class QLabel;

class YesNoDialog : public QDialog
{
    Q_OBJECT

public:
    YesNoDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    virtual ~YesNoDialog();

    QPushButton* cancelButton;
    QLabel* question;
    QPushButton* okButton;

    void SetQuestion(QString text);
    
protected:

protected slots:
    virtual void languageChange();

};

#endif // TAG_YESNODIALOGDIALOG_H

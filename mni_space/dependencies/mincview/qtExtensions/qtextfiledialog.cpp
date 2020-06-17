#include "qtextfiledialog.h"

#include <qstring.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

QtExtFileDialog::QtExtFileDialog( QWidget* parent, const char* dialog_name, QString directory, bool open_file)
{
    this->OpenFile = 0;
    this->OpenError = FA_NO_ERROR;
    if (open_file)
    {
        OpenFileName = QFileDialog::getOpenFileName(directory, "All valid files(*.txt;*.xml);All files(*)", parent, dialog_name, "Open files" );
        if (!OpenFileName.isEmpty())
        {
            this->OpenFile = new QFile;
            this->OpenFile->setName(OpenFileName);
            if (!this->OpenFile->open(IO_ReadOnly))
            {
                delete this->OpenFile;
                this->OpenFile = 0;
                this->OpenError = FA_NO_READ;
            }
            else
                this->OpenFile->close();
        }
        else
            this->OpenError = FA_NO_FILE;
    }
    else
    {
        OpenFileName = QFileDialog::getSaveFileName(directory, "All valid files(*.txt;*.xml);All files(*)", parent, dialog_name, "Save file" );
        if (QFile::exists(OpenFileName))
        {
            int ret = QMessageBox::warning(this, tr("SaveFile"),
                                           tr("File %1 already exists.\n"
                                                   "Do you want to overwrite it?")
                                                   .arg(QDir::convertSeparators(OpenFileName)),
                                           QMessageBox::Yes | QMessageBox::Default,
                                           QMessageBox::No | QMessageBox::Escape);
            if (ret == QMessageBox::No)
            {
                this->OpenError = FA_EXISTS;
                return;
            }
        }
        if (!OpenFileName.isEmpty())
        {
            this->OpenFile = new QFile;
            this->OpenFile->setName(OpenFileName);
            if (!this->OpenFile->open(IO_WriteOnly))
            {
                delete this->OpenFile;
                this->OpenFile = 0;
                this->OpenError = FA_NO_WRITE;
            }
            else
                this->OpenFile->close();
        }
    }
}

QtExtFileDialog::~QtExtFileDialog()
{
}

QFile * QtExtFileDialog::GetOpenFile()
{
    return this->OpenFile;
}



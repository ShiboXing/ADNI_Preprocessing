#ifndef EDITPOINTCOORDINATES_H
#define EDITPOINTCOORDINATES_H

#include <qvariant.h>
#include <qdialog.h>

#include "vtkReal.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QLineEdit;
class QPushButton;

class EditPointCoordinates : public QDialog
{
    Q_OBJECT

public:
    EditPointCoordinates( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~EditPointCoordinates();

    QLabel* xLabel;
    QLabel* yLabel;
    QLabel* zLabel;
    QLineEdit* xCoordinate;
    QLineEdit* yCoordinate;
    QLineEdit* zCoordinate;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

    void SetPointCoordinates(vtkReal *);
    void GetPointCoordinates(vtkReal *);
    vtkReal * GetPointCoordinates();

public slots:
    
    void SetXCoordinate();
    void SetYCoordinate();
    void SetZCoordinate();
                
protected:
    QHBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;
    
    vtkReal pointCoordinates[3];

protected slots:
    virtual void languageChange();

};

#endif // EDITPOINTCOORDINATES_H

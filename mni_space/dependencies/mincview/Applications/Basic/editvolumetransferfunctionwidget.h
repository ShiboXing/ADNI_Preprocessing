/****************************************************************************
** Form interface generated from reading ui file 'editvolumetransferfunctionwidget.ui'
**
** Created: Wed Aug 24 12:55:33 2005
**      by: The User Interface Compiler ($Id: editvolumetransferfunctionwidget.h,v 1.1 2006-03-28 19:53:58 anka Exp $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef EDITVOLUMETRANSFERFUNCTIONWIDGET_H
#define EDITVOLUMETRANSFERFUNCTIONWIDGET_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

#include "vtkReal.h"
#include "serializer.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QFrame;
class QButtonGroup;
class QLabel;
class QPushButton;

class vtkQtRenderWindow;
class vtkVolumeTransferFunctionWidget;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkVolumeProperty;

// Structure that holds transfer function settings settings
struct TransferFunctionSettings
{
    TransferFunctionSettings();
    void Serialize( Serializer * serializer );
    
    vtkReal Range[2];
    int AlphaPointsNumber;
    double AlphaPoints[20][2];
    int RgbPointsNumber;
    double RgbPoints[20][4];
};

ObjectSerializationHeaderMacro( TransferFunctionSettings );

enum ColorSpectrum {
    Gray, 
    Red,
    Orange,
    Yellow,
    Green,
    Blue,
    BlueToRed,
    Full,
    BlackToYellow,
};

class EditVolumeTransferFunctionWidget : public QWidget
{
    Q_OBJECT

public:
    EditVolumeTransferFunctionWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~EditVolumeTransferFunctionWidget();

    void InitVolumeTransferFunction();
    void InitVolumeTransferFunction(vtkVolumeProperty * volumeProperty);
    void SetSelectedSpectrum(ColorSpectrum spectrum);
    
    QButtonGroup* SelectSpectrumButtonGroup;
    QLabel* colorLabel;
    QPushButton* GrayPushButton;
    QPushButton* RedPushButton;
    QPushButton* OrangePushButton;
    QPushButton* YellowPushButton;
    QPushButton* GreenPushButton;
    QPushButton* BluePushButton;
    QPushButton* BlueToRedPushButton;
    QPushButton* FullPushButton;
    QPushButton* BlackToYellowPushButton;
    QButtonGroup* FunctionButtonGroup;
    QPushButton* SavePushButton;
    QPushButton* LoadPushButton;
    QPushButton* ResetPushButton;
    QPushButton* CancelPushButton;
    QPushButton* OkPushButton;

public slots:
    virtual void SaveSettings();
    virtual void SaveAndClose();
    virtual void LoadSettings();
    virtual void InitialLoadSettings();
    virtual void ResetSettings();
    virtual void ResetAndCancel();
    virtual void GrayPushButtonClicked();
    virtual void RedPushButtonClicked();
    virtual void OrangePushButtonClicked();
    virtual void YellowPushButtonClicked();
    virtual void GreenPushButtonClicked();
    virtual void BlackToYellowPushButtonClicked();
    virtual void BlueToRedPushButtonClicked();
    virtual void FullPushButtonClicked();
    virtual void BluePushButtonClicked();

protected:
    
    void Init();
    
    QVBoxLayout* mainLayout;
    QHBoxLayout* topLayout;
    QVBoxLayout* Colorlayout;
    QHBoxLayout* FunctionButtonGroupLayout;
    QHBoxLayout* FunctionLayout;
    QSpacerItem* spacer2;
    QSpacerItem* spacer10;
    QSpacerItem* spacer9;
    QSpacerItem* spacer1;
    QSpacerItem* spacer4;
    QSpacerItem* spacer3;

    vtkQtRenderWindow  *RenderWindow;
    vtkReal ImageRange[2];
    vtkVolumeTransferFunctionWidget * VolumeTransferFunctionWidget;
    vtkPiecewiseFunction * SaveAlphaFunc;
    vtkColorTransferFunction * SaveRgbFunc;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;
    QPixmap image3;
    QPixmap image4;
    QPixmap image5;
    QPixmap image6;
    QPixmap image7;
    QPixmap image8;

    void UpdateSettings();
    TransferFunctionSettings *settings;
};

#endif // EDITVOLUMETRANSFERFUNCTIONWIDGET_H

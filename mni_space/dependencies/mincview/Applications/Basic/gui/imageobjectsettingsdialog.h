/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: imageobjectsettingsdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-03-03 19:25:59 $
  Version:   $Revision: 1.23 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/

#ifndef IMAGEOBJECTSETTINGSDIALOG_H
#define IMAGEOBJECTSETTINGSDIALOG_H

#include <vector>
#include "imageobjectsettingsdialogbase.h"

class ImageObject;
class UserTransformations;

class ImageObjectSettingsDialog : public ImageObjectSettingsDialogBase
{
    Q_OBJECT

public:
    ImageObjectSettingsDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~ImageObjectSettingsDialog();

    virtual void SetImageObject( ImageObject * obj );
    virtual void SetWindowAndLevel(int, int);
    
    void SetUserTransforms();
    
public slots:
    virtual void SelectColorTableComboBoxActivated(int);
    virtual void ViewVolumeCheckBoxToggled(bool);
    virtual void DecreaseWindowPushButtonClicked( );
    virtual void WindowSliderValueChanged(int);
    virtual void IncreaseWindowPushButtonClicked();
    virtual void DecreaseLevelPushButtonClicked();
    virtual void LevelSilderValueChanged(int);
    virtual void IncreaseLevelPushButtonClicked();
    virtual void ResetPlanesPushButtonClicked();
    virtual void OutlineCheckBoxToggled(bool);
    virtual void XPlaneCheckBoxToggled(bool);
    virtual void YPlaneCheckBoxToggled(bool);
    virtual void ZPlaneCheckBoxToggled(bool);
    virtual void Planes3DCheckBoxToggled(bool);
    virtual void FrontPushButtonClicked();
    virtual void BackPushButtonClicked();
    virtual void LeftPushButtonClicked();
    virtual void RightPushButtonClicked();
    virtual void TopPushButtonClicked();
    virtual void BottomPushButtonClicked();
    virtual void SurfaceCheckBoxToggled(bool);
    virtual void SmoothingCheckBoxToggled(bool);
    virtual void ContourLineEditTextChanged();
    virtual void OpacityLineEditTextChanged();
    virtual void StandardDeviationLineEditTextChanged();
    virtual void RadiusFactorLineEditTextChanged();
    virtual void TransferFunctionPushButtonClicked();
    virtual void ImageSettingsWidgetTabChanged(QWidget *);
    virtual void SameOpacityCheckBoxToggled(bool);
    virtual void DecreaseXPlaneOpacityPushButtonClicked();
    virtual void DecreaseYPlaneOpacityPushButtonClicked();
    virtual void DecreaseZPlaneOpacityPushButtonClicked();
    virtual void IncreaseXPlaneOpacityPushButtonClicked();
    virtual void IncreaseYPlaneOpacityPushButtonClicked();
    virtual void IncreaseZPlaneOpacityPushButtonClicked();
    virtual void XPlaneOpacitySliderValueChanged(int);
    virtual void YPlaneOpacitySliderValueChanged(int);
    virtual void ZPlaneOpacitySliderValueChanged(int);
    virtual void ShowAllOpacitySliders(bool);
    virtual void TransformFromListSelected(const QString &);
    virtual void ShowTransformationMatrix();
    virtual void UpdatePlaneSettings(int, int);
    virtual void UndoLastTransformation();
    virtual void XPlus1PushButtonClicked();
    virtual void YPlus1PushButtonClicked();
    virtual void ZPlus1PushButtonClicked();
    virtual void XPlus5PushButtonClicked();
    virtual void YPlus5PushButtonClicked();
    virtual void ZPlus5PushButtonClicked();
    virtual void XMinus5PushButtonClicked();
    virtual void YMinus5PushButtonClicked();
    virtual void ZMinus5PushButtonClicked();
    virtual void XMinus1PushButtonClicked();
    virtual void YMinus1PushButtonClicked();
    virtual void ZMinus1PushButtonClicked();
        
    virtual void UpdateUI();
    void ShowLastUsedWidget(int index);
    void SetAppliedTransforms(std::vector<int> &transformIds);
    
signals:
    void TransformationChanged(QString, int);
    void UndoTransformation(int);
    
protected:

    void ComputeWindowLevelSlidersStepsAndSetting();
    QWidget * CreateEditVolumeTransferFunctionDialog( );
    
    ImageObject * m_imageObject;
    int m_windowSliderStep;
    int m_levelSliderStep;
    int m_planeOpacitySliderStep;
    bool m_allPlanesHaveSameOpacity;
    double m_maxRange[2];
    UserTransformations *m_userTransforms ;
private:
    void AllControlsBlockSignals(bool yes);
    
};

#endif // IMAGEOBJECTSETTINGSDIALOG_H

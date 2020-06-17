/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: pointsobjectsettingsdialog.h,v $
  Language:  C++
  Date:      $Date: 2010-04-28 19:05:52 $
  Version:   $Revision: 1.4 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/

#ifndef POINTSOBJECTSETTINGSDIALOG_H
#define POINTSOBJECTSETTINGSDIALOG_H

#include "pointsobjectsettingsdialogbase.h"
#include "pointsobject.h"
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
// IGSTK headers
#include "igstkObject.h"
#include "igstkMacros.h"
#endif

class UserTransformations;

class PointsObjectSettingsDialog : public PointsObjectSettingsDialogBase
{
    Q_OBJECT

public:
    PointsObjectSettingsDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~PointsObjectSettingsDialog();

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()
#endif

public slots:
    virtual void TransformFromListSelected(const QString &);
    virtual void ShowTransformationMatrix();
    virtual void UndoLastTransformation();
    virtual void XMinus1PushButtonClicked();
    virtual void YMinus1PushButtonClicked();
    virtual void ZMinus1PushButtonClicked();
    virtual void XMinus5PushButtonClicked();
    virtual void YMinus5PushButtonClicked();
    virtual void ZMinus5PushButtonClicked();
    virtual void XPlus1PushButtonClicked();
    virtual void YPlus1PushButtonClicked();
    virtual void ZPlus1PushButtonClicked();
    virtual void XPlus5PushButtonClicked();
    virtual void YPlus5PushButtonClicked();
    virtual void ZPlus5PushButtonClicked();
    
    virtual void LabelSizeChanged(int);
    virtual void PointRadiusChanged(int);
    virtual void HideLabelsPushButtonClicked();
    virtual void SelectionModePushButtonClicked();
    virtual void ShowListOfPointsCheckBoxToggled(bool);
    
//color tab
    virtual void OpacitySliderValueChanged(int);
    virtual void OpacityTextEditChanged(const QString &);
    virtual void DisabledColorSetButtonClicked();
    virtual void EnabledColorSetButtonClicked();
    virtual void SelectedColorSetButtonClicked();
    
    virtual void SettingsTabWidgetCurrentChanged(QWidget*);
    
    void UpdateSettings();
    void ShowLastUsedWidget(int index);
    void SetPointsObject(PointsObject *obj);
    void ResetPointListCheckBox();
    void SetAppliedTransforms(std::vector<int> &transformIds);

signals:
    void UpdateList();
    void TransformationChanged(QString, int);
    void UndoTransformation(int);
    
protected:
    PointsObject *m_points;
    UserTransformations *m_userTransforms ;
    void UpdateUI();
    void UpdateOpacityUI();
    void SetUserTransforms();
};

#endif // POINTSOBJECTSETTINGSDIALOG_H

#ifndef POLYDATAOBJECTSETTINGSDIALOG_H
#define POLYDATAOBJECTSETTINGSDIALOG_H

#include <vector>
#include "polydataobjectsettingsdialogbase.h"

class PolyDataObject;
class UserTransformations;

class PolyDataObjectSettingsDialog : public PolyDataObjectSettingsDialogBase
{
    Q_OBJECT

public:
    
    PolyDataObjectSettingsDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~PolyDataObjectSettingsDialog();

    void SetPolyDataObject( PolyDataObject * object );
    void SetAppliedTransforms(std::vector<int> &transformIds);
    
public slots:
    
    virtual void ScalarVisibilityCheckBoxToggled(bool);
    virtual void ColorSwatchClicked();
    virtual void DisplayModeButtonPressed( int );
    virtual void OpacitySliderValueChanged(int);
    virtual void OpacityEditTextChanged( const QString & );
    virtual void TransformFromListSelected(const QString &);
    virtual void ShowTransformationMatrix();
    virtual void SettingsTabWidgetCurrentChanged(QWidget*);
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
    virtual void RadiusLineEditChanged();
    virtual void NewResolutionSelected(int);
    
    void ShowLastUsedWidget(int index);
    void UpdateSettings();

signals:
    void TransformationChanged(QString, int);
    void UndoTransformation(int);
        
protected:
        
    PolyDataObject * m_object;
    UserTransformations *m_userTransforms ;
       
    void UpdateUI();
    void UpdateOpacityUI();
    void SetUserTransforms();
};

#endif //POLYDATAOBJECTSETTINGSDIALOG_H

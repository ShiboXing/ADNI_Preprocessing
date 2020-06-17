#ifndef TAG_PICKEROBJECTSETTINGSDIALOG_H_
#define TAG_PICKEROBJECTSETTINGSDIALOG_H_

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class SceneManager;
class QPushButton;

class PickerObjectSettingsDialog : public QWidget
{
    Q_OBJECT

public:
    
    PickerObjectSettingsDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~PickerObjectSettingsDialog();
    
    // only functionality will be enabling picking which will display picking dialog
    QPushButton * enablePickingButton;

    void SetSceneManager( SceneManager * man );
    
public slots:
    
    void EnablePickingButtonToggled( );

protected:
    
    QVBoxLayout* PickerObjectSettingsDialogLayout;
    
    SceneManager * manager;
    
protected slots:
    
    virtual void languageChange();

};

#endif // TAG_PICKEROBJECTSETTINGSDIALOG_H_

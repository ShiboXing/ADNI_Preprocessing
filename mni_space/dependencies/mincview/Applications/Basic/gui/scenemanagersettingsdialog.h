#ifndef __SceneManagerSettingsDialog_h_
#define __SceneManagerSettingsDialog_h_

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;
class SceneManager;

class SceneManagerSettingsDialog : public QWidget
{
    Q_OBJECT

public:
    
    SceneManagerSettingsDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~SceneManagerSettingsDialog();
    
    void SetSceneManager( SceneManager * man );

    QLabel      * backgroundColorLabel;
    QPushButton * setBackgroundColorButton;

public slots:
    
    virtual void SetBackgroundColorButtonClicked();

protected:
    
    void UpdateUI();
    
    QVBoxLayout* SceneManagerSettingsDialogLayout;
    QHBoxLayout* layout2;
    
    SceneManager * SceneMan;

protected slots:
    
    virtual void languageChange();

};

#endif

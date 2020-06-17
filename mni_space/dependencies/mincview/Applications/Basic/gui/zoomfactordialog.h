#ifndef ZOOMFACTORDIALOG_H
#define ZOOMFACTORDIALOG_H

#include "zoomfactordialogbase.h"

class SceneManager;

class ZoomFactorDialog : public ZoomFactorDialogBase
{
    Q_OBJECT

public:
    ZoomFactorDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ZoomFactorDialog();

    void SetSceneManager(SceneManager *m);
    
public slots:
    virtual void OkButtonClicked();

protected:
    SceneManager *m_manager;
};

#endif // ZOOMFACTORDIALOG_H

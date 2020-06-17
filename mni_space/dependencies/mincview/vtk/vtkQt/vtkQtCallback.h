#ifndef __vtkQtCallback_h_
#define __vtkQtCallback_h_

#include "vtkObject.h"
#include "vtkCommand.h"
#include <qwidget.h>
#include <qevent.h>

class vtkQtCallback : public QObject, public vtkCommand
{
    
Q_OBJECT

public:

    vtkQtCallback() { }
    ~vtkQtCallback() {}
    static vtkQtCallback * New() { return new vtkQtCallback; }
    virtual void Execute( vtkObject * caller, unsigned long eventId, void * callData );

signals:

    void EventTriggered( vtkObject * caller, unsigned long eventId, void * callData );
};


#endif

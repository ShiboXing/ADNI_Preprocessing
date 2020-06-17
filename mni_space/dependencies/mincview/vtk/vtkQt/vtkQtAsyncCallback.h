#ifndef __vtkQtAsyncCallback_h_
#define __vtkQtAsyncCallback_h_

#include "vtkCommand.h"
#include <qobject.h>
#include <qevent.h>


class vtkQtCustomEvent : public QCustomEvent
{
    
public:
    
    vtkQtCustomEvent();
    ~vtkQtCustomEvent() {}
    
    vtkObject * caller;
    unsigned long eventId;
    void * callData;
};


// Description:
// This class is used to connect event emited by vtk classes to
// slots defined on Qt classes. Furthermore, these events are
// posted asynchroneously.
class vtkQtAsyncCallback : public QObject, public vtkCommand
{
    
Q_OBJECT
    
public:
    
    static vtkQtAsyncCallback * New() { return new vtkQtAsyncCallback; }
    
    vtkQtAsyncCallback();
    ~vtkQtAsyncCallback();
    
    // Description:
    // This function will catch the vtk event and post a Qt custom event
    virtual void Execute( vtkObject * caller, unsigned long eventId, void * callData );
    
    // Description:
    // This function will catch the Qt custom event posted by Execute and
    // emit a Qt signal that can be caught by other QObjects connected to the signal
    virtual void customEvent ( QCustomEvent * event );
    
    // Description:
    // The DoNotTriggerEventIfPending property of this class is used to control 
    // if an event received by this class from a vtk class will be posted to
    // the Qt application if other event are pending in the Qt Application class's
    // event queue. This can be used to make sure the queue doesn't get too big.
    void DoNotTriggerEventIfPendingOn() { this->DoNotTriggerEventIfPending = 1; }
    void DoNotTriggerEventIfPendingOff() { this->DoNotTriggerEventIfPending = 0; }
    int  IsDoNotTriggerEventIfPendingOn() { return this->DoNotTriggerEventIfPending; }
    
signals:

    void EventTriggered();
    
protected:
    
    int DoNotTriggerEventIfPending;
    vtkQtCustomEvent * LastEvent;
        
};

#endif

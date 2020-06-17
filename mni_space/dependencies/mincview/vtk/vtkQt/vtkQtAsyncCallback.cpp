#include "vtkQtAsyncCallback.h"
#include "qapplication.h"

vtkQtCustomEvent::vtkQtCustomEvent() : QCustomEvent( QEvent::User )
{
    caller = NULL;
    eventId = vtkCommand::NoEvent;
    callData = NULL;
}


vtkQtAsyncCallback::vtkQtAsyncCallback() 
{
    this->DoNotTriggerEventIfPending = 0;
    this->LastEvent = 0;
}

    
vtkQtAsyncCallback::~vtkQtAsyncCallback()
{
}

    
void vtkQtAsyncCallback::Execute( vtkObject * caller, unsigned long eventId, void * callData )
{
    if( this->LastEvent && this->DoNotTriggerEventIfPending )
        return;
    
    this->LastEvent = new vtkQtCustomEvent();
    this->LastEvent->caller = caller;
    this->LastEvent->eventId = eventId;
    this->LastEvent->callData = callData;
    QApplication::postEvent( this, this->LastEvent );
}


void vtkQtAsyncCallback::customEvent( QCustomEvent * event )
{
    vtkQtCustomEvent * subEvent = (vtkQtCustomEvent*)event;
    if( this->LastEvent && this->LastEvent == subEvent )
    {
        // mark last event has processed
        this->LastEvent = 0;
    }
    
    emit EventTriggered();
}
#include "vtkQtCallback.h"
#include "qapplication.h"


void vtkQtCallback::Execute( vtkObject * caller, unsigned long eventId, void * callData )
{
    emit EventTriggered( caller, eventId, callData );
}

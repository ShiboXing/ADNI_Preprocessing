#include "vtkQtProgressDialog.h"
#include "vtkProcessObject.h"
#include <qprogressdialog.h>
#include <qapplication.h>
#include "vtkReal.h"

void vtkQtProgressDialog::Execute( vtkObject * caller, unsigned long eventId, void * callData )
{
    if( eventId == vtkCommand::StartEvent )
    {
        m_dlg->reset();
        m_dlg->showNormal();
    }
    else if( eventId == vtkCommand::EndEvent )
    {
        m_dlg->close();
    }
    else if ( eventId == vtkCommand::ProgressEvent )
    {
        vtkProcessObject * process = vtkProcessObject::SafeDownCast( caller );
        vtkReal percent = process->GetProgress();
        m_dlg->setProgress( (int)(percent * 100) );
    }
}


vtkQtProgressDialog::vtkQtProgressDialog()
{
    m_dlg = new QProgressDialog( qApp->mainWidget(), 0, FALSE, 0 );
    m_dlg->setTotalSteps( 100 );
}


vtkQtProgressDialog::~vtkQtProgressDialog()
{
    delete m_dlg;
}

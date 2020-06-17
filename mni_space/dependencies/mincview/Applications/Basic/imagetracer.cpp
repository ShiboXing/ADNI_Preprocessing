#include "imagetracer.h"
#include <qdir.h>
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorObserver.h"
#include "vtkCommand.h"
#include "vtkCoordinate.h"
#include "vtkViewport.h"
#include "vtkMultiImagePlaneWidget.h"
#include "vtkImageData.h"
#include "vtkLinearTransform.h"
#include "vtkTransform.h"
#include "vtkIgnsEvents.h"
#include "vtkCallbackCommand.h"
#include "view.h"
#include "imageobject.h"
#include "scenemanager.h"
#include "savepointdialog.h"

void SavePointCallbackFunction ( vtkObject* caller, long unsigned int eventId, void* clientData, void* callData )
{
    ImageTracer *self = reinterpret_cast<ImageTracer*>(clientData);
    self->SavePoint();
}

ImageTracer::ImageTracer()
{
    m_imageObj = 0;
    m_manager = 0;
    for (int i = 0; i < 3; i++)
    {
        m_initialPlanesPosition[i] = 0.0;
        m_currentPlanesPosition[i] = 0.0;
    }
    m_interactor = 0;
    m_savePointCallbackCommand = vtkCallbackCommand:: New();
    m_savePointCallbackCommand->SetClientData((void*)this);
    m_savePointCallbackCommand->SetCallback(SavePointCallbackFunction);
    m_savePointCallbackCommand->SetAbortFlagOnExecute(1);
}

ImageTracer::~ImageTracer()
{
    if( m_imageObj )
        m_imageObj->UnRegister( this );
    if (m_manager != NULL)
        m_manager->UnRegister(this);
    if( m_interactor )
    {
        m_interactor->RemoveObserver(m_savePointCallbackCommand);
        m_interactor->UnRegister( this );
    }
    m_savePointCallbackCommand->Delete();
}

void ImageTracer::SetImageObject( ImageObject * obj )
{
    if( m_imageObj == obj )
    {
        return;
    }
    
    if( m_imageObj )
    {
        m_imageObj->UnRegister( this );
    }
    m_imageObj = obj;
    if( m_imageObj )
    {
        m_imageObj->Register( this );
    }
    for (int i = 0; i < 3; i++)
    {
        this->viewPlanes[i] = m_imageObj->GetPlane(i);
        m_initialPlanesPosition[i] = this->viewPlanes[i]->GetSlicePosition();
        m_currentPlanesPosition[i] = m_initialPlanesPosition[i];
        this->viewPlanes[i]->ExternalActivateCursor(1);
    }
    if (m_manager != NULL) 
    { 
        m_manager->UnRegister(this);
    }   
    m_manager = m_imageObj->GetManager();
    if (m_manager != NULL) 
    { 
        m_manager->Register(this);
    }
    View * v3D = m_manager->GetView(THREED_VIEW_TYPE);
    if (v3D)
    {
        this->SetInteractor(v3D->GetInteractor());
    }
}
   
void ImageTracer::SetTracerPosition( vtkReal *pt)
{
    if (pt)
    {
        vtkReal *pos;
        vtkTransform *t = m_imageObj->GetCurrentTransform();
        if (!t)
        {
            pos = pt;
        }
        else
        {
            vtkLinearTransform *transform = t->GetLinearInverse();
            pos = transform->TransformDoublePoint(pt[0], pt[1], pt[2]);
        }
        this->viewPlanes[0]->SetSlicePosition(pos[0]);
        this->viewPlanes[1]->SetSlicePosition(pos[1]);
        this->viewPlanes[2]->SetSlicePosition(pos[2]);
        for (int j = 0; j < 3; j++)
        {
            m_currentPlanesPosition[j] = pos[j];
            View * v = m_manager->GetView(j);
            vtkCoordinate* coord = vtkCoordinate::New();
            coord->SetCoordinateSystemToWorld();
            coord->SetValue(pt);
            int *dispPos = coord->GetComputedDisplayValue(reinterpret_cast<vtkViewport*>(v->GetRenderer()));
            this->viewPlanes[j]->ExternalUpdateCursor(dispPos, v->GetRenderer());
            m_manager->GetView(j)->GetRenderer()->ResetCameraClippingRange();
            m_manager->GetView(j)->NotifyNeedRender();
            this->viewPlanes[j]->ExternalUpdateCursor(dispPos, v->GetRenderer());//TODO this is a patch which forces showing cursor
        }
    }
}

void ImageTracer::DisableImageTracer()
{
    for (int i = 0; i < 3; i++)
    {
        this->viewPlanes[i]->ExternalActivateCursor(0);
        this->viewPlanes[i]->SetSlicePosition(m_initialPlanesPosition[i]);
        m_manager->GetView(i)->GetRenderer()->ResetCameraClippingRange();
        m_manager->GetView(i)->NotifyNeedRender();
    }
}

void ImageTracer::SetInteractor( vtkRenderWindowInteractor * interactor )
{
    if( interactor == m_interactor )
    {
        return;
    }
    
    if( m_interactor )
    {
        m_interactor->RemoveObserver(m_savePointCallbackCommand);
        m_interactor->UnRegister( this );
    }
    
    interactor->Register( this );
    m_interactor = interactor;
    m_interactor->AddObserver( vtkCommand::UserEvent+IGNS_SAVE_POINTER_POSITION, m_savePointCallbackCommand );
}

void ImageTracer::SetSaveDirectory(const QString & dir)
{
    m_saveDirectory = dir;
    if( !QFile::exists( m_saveDirectory ) )
    {
        m_saveDirectory = QDir::homeDirPath();
    }
}

void ImageTracer::SavePoint()
{
    SavePointDialog *dialog = new SavePointDialog;
    dialog->SetSaveDirectory(m_saveDirectory);
    dialog->SetManager(m_manager);
    dialog->SetCurrentPointCoords(m_currentPlanesPosition);
    dialog->show();
}

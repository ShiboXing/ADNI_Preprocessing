#ifndef TAG_IMAGE_TRACER_H_
#define TAG_IMAGE_TRACER_H_

#include "vtkObject.h"
#include <qobject.h>
#include <qstring.h>
#include "vtkReal.h"

class vtkMultiImagePlaneWidget;
class ImageObject;
class SceneManager;
class vtkSavePointCallback;
class vtkRenderWindowInteractor;
class vtkCallbackCommand;

class ImageTracer : public QObject, public vtkObject
{
    Q_OBJECT

public:

    static ImageTracer * New() { return new ImageTracer; }

    ImageTracer();
    virtual ~ImageTracer();

    void DisableImageTracer();
    void SetImageObject(ImageObject *obj);
    ImageObject *GetImageObject() {return m_imageObj;}
    void SetTracerPosition( vtkReal *pos);
    void SetInteractor( vtkRenderWindowInteractor * interactor );
    void SavePoint();
    void SetSaveDirectory(const QString &dir);
        
protected:

    ImageObject *m_imageObj;
    SceneManager *m_manager;
    vtkMultiImagePlaneWidget *viewPlanes[3];
    vtkReal m_initialPlanesPosition[3];
    vtkReal m_currentPlanesPosition[3];
    vtkRenderWindowInteractor *m_interactor;
    QString m_saveDirectory;
    
    vtkCallbackCommand *m_savePointCallbackCommand;
};

#endif //TAG_IMAGE_TRACER_H_

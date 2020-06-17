#ifndef VTKQTIMAGEVIEWER_H
#define VTKQTIMAGEVIEWER_H

#include "vtkQtRenderWindow.h"

class vtkImageActor;
class vtkRenderer;
class vtkQtRenderWindowInteractor;
class vtkInteractorStyle;
class vtkImageData;
class QWidget;


class vtkQtImageViewer : public vtkQtRenderWindow
{

public:

    vtkQtImageViewer( QWidget * parent = 0, const char * name = 0 );
    ~vtkQtImageViewer();
    
    static vtkQtImageViewer * New();
    void PrintSelf(ostream& os, vtkIndent indent);

    vtkTypeMacro(vtkQtImageViewer, vtkQtRenderWindow );

    void SetInput( vtkImageData * input );
    void RenderFirst();
    
private:
    
    vtkImageActor                  * m_actor;
    vtkRenderer                    * m_renderer;

};


#endif


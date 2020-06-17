#ifndef TAG_VTKCROSSHAIRWIDGET_H_
#define TAG_VTKCROSSHAIRWIDGET_H_

#include "vtkReal.h"
#include "vtkImageTracerWidget.h"

class vtkCrosshairWidget : public vtkImageTracerWidget
{
public:
    // Description:
    // Instantiate the object.
    static vtkCrosshairWidget *New();
    
    vtkTypeRevisionMacro(vtkCrosshairWidget,vtkImageTracerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);
    
    virtual void SetEnabled(int a)
        {this->Superclass::SetEnabled(a);}
    
    void SetColor(vtkReal col[3]);
    void SetColor(vtkReal r, vtkReal g, vtkReal b);
    void SetScale(vtkReal s);
    void SetLineWidth(vtkReal w);
        
protected:
    vtkCrosshairWidget();
    virtual ~vtkCrosshairWidget();

private:
    vtkCrosshairWidget(const vtkCrosshairWidget&);  //Not implemented
    void operator=(const vtkCrosshairWidget&);  //Not implemented
};

#endif //TAG_VTKCROSSHAIRWIDGET_H_

#include "vtkCrosshairWidget.h"
#include "vtkObjectFactory.h"
#include "vtkGlyphSource2D.h"
#include "vtkProperty.h"

vtkCxxRevisionMacro(vtkCrosshairWidget, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkCrosshairWidget);

vtkCrosshairWidget::vtkCrosshairWidget()
{
  this->CaptureRadius = 1.0;
}

vtkCrosshairWidget::~vtkCrosshairWidget()
{  
}

void vtkCrosshairWidget::SetColor(vtkReal r, vtkReal g, vtkReal b)
{
    vtkProperty *handleProperty = this->GetHandleProperty();
    if (handleProperty)
        handleProperty->SetColor(r, g, b);
}

void vtkCrosshairWidget::SetColor(vtkReal col[3])
{
    vtkProperty *handleProperty = this->GetHandleProperty();
    if (handleProperty)
        handleProperty->SetColor(col);
}

void vtkCrosshairWidget::SetScale(vtkReal s)
{
    this->GetGlyphSource()->SetScale(s);
    this->GetGlyphSource()->Modified();
}

void vtkCrosshairWidget::SetLineWidth(vtkReal w)
{
    vtkProperty *handleProperty = this->GetHandleProperty();
    if (handleProperty)
        handleProperty->SetLineWidth( w );
}

void vtkCrosshairWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


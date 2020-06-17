#ifndef TAG_GENERATEDSURFACE_H_
#define TAG_GENERATEDSURFACE_H_

#include <qstring.h>
#include "vtkReal.h"
#include "serializer.h"

class vtkRenderWindow;
class vtkActor;
class vtkImageData;
class vtkRenderer;
class vtkRenderWindowInteractor;
class ImageObject;
class vtkLODActor;
class vtkTransform;
class vtkContourFilter;
class vtkPolyDataMapper;

#define DEFAULT_RADIUS 1.0
#define DEFAULT_STANDARD_DEVIATION 1.2

class GeneratedSurface : public vtkObject
{
public:
    
    static GeneratedSurface *New() { return new GeneratedSurface; }
    void Delete() { delete this; }
    
    void SetRenderer(vtkRenderer *ren);
    void SetInteractor(vtkRenderWindowInteractor *inter);
    void SetImageObject(ImageObject *obj);
    void SetContourLut(vtkLookupTable *imageLut);
    void SetSurfaceObjectName( QString name );
    vtkGetObjectMacro( Transform, vtkTransform );
    void SetTransform( vtkTransform * transform );
    vtkReal GetContourValue();
    double GetOpacityValue();
    void SetContourValue(vtkReal cv);
    void SetOpacityValue(double opacity);
    vtkLODActor *GetSurfaceActor() {return contourActor;}
    
    void DisplaySurface();
    void RemoveSurfaceView();
    void ShowSurfaceView();
    
    //Gaussian smoothing of the generated surface
    int GetGaussianSmoothingFlag() {return gaussianSmoothing;}
    double GetRadiusFactor() {return radius;}
    double GetStandardDeviation() {return standardDeviation;}
    void SetGaussianSmoothingFlag(int flag);
    void SetRadiusFactor(double radiusFactor);
    void SetStandardDeviation(double deviation);
    
protected:
    ImageObject *object; 
    vtkLODActor * contourActor;
    vtkContourFilter *contourExtractor;
    vtkPolyDataMapper *contourMapper;    
    vtkLookupTable * contourLut;
    vtkRenderer * renderer;
    vtkRenderWindowInteractor * interactor;
    QString objectName;
    double minScalar, maxScalar;
    double opacity;
    vtkTransform * Transform;
    //Gaussian smoothing of the surface
    int gaussianSmoothing;
    double radius;
    double standardDeviation;
    
    GeneratedSurface();
    ~GeneratedSurface();
    
private:            
    vtkReal maxContourVal;
  
    void GetImageMinMax( vtkImageData * image, double *minval, double *maxval );
};
    
#endif //TAG_GENERATEDSURFACE_H_

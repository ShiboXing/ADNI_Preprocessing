/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSphereWithTextPointWidget.h,v $

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

      adapted from vtkPointWidget by Anka at BIC, 2007
  
  original vtkPointWidget copyright:    
  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

=========================================================================*/
// .NAME vtkSphereWithTextPointWidget a sphere in 3D space
// .SECTION Description
// I made the class vtkSphereWithTextPointWidget  in order to have a movable point with label.
// The point is represented by a small sphere, the label moves with the sphere and is positioned relatively
// to the sphere center. The size of the sphere may be set by SetRadius()

// .SECTION Caveats
// middle and right button are for now disabled, but the sphere highlights whenever selected
// with any button. The label attributes cannot be set, only text. 

// .SECTION See Also
// vtkPointWidget vtk3DWidget vtkLineWidget vtkBoxWidget vtkPlaneWidget

#ifndef TAG_VTKSPHEREWITHTEXTPOINTWIDGET_H
#define TAG_VTKSPHEREWITHTEXTPOINTWIDGET_H

#include <string>
#include "vtk3DWidget.h"
#include "vtkSphereSource.h"     
#include "vtkVectorText.h"

class vtkActor;
class vtkPolyDataMapper;
class vtkCellPicker;
class vtkProperty;
class vtkTextProperty;
class vtkTransform;
class vtkFollower;

#define INITIAL_TAG_LABEL_SCALE 8.0

class vtkSphereWithTextPointWidget : public vtk3DWidget
{
public:
    // Description:
    // Instantiate this widget
    static vtkSphereWithTextPointWidget *New();

    vtkTypeRevisionMacro(vtkSphereWithTextPointWidget,vtk3DWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Set the point ID
    vtkSetMacro(PointIndex,int);
    vtkGetMacro(PointIndex,int);
    
    // Description:
    // update points.
    virtual void Update();
    
    // Description:
    // Methods that satisfy the superclass' API.
    virtual void SetEnabled(int);
    virtual void PlaceWidget(double bounds[6]);
    void PlaceWidget()
        {this->Superclass::PlaceWidget();}
    void PlaceWidget(double xmin, double xmax, double ymin, double ymax, 
                     double zmin, double zmax)
        {this->Superclass::PlaceWidget(xmin,xmax,ymin,ymax,zmin,zmax);}
  
    // Description:
    // Deep copy copies all vtk3DWidget data and, sphere params, label and properties.
    virtual void DeepCopy(vtkSphereWithTextPointWidget *pt);
  
    // Description:
    // Set/Get the position of the point - center of the sphere representing the point
    void SetPosition(double x, double y, double z)
        {this->Sphere->SetCenter(x,y,z);}
    void SetPosition(double x[3])
        {this->Sphere->SetCenter(x[0],x[1],x[2]);}
    double* GetPosition() 
        {return this->Sphere->GetCenter();}
    void GetPosition(double xyz[3]) 
        {this->Sphere->GetCenter(xyz);}

    // Description:
    // Set/Get the size of the point - radius of the sphere representing the point
    void SetRadius(double r)
        {this->Sphere->SetRadius(r);}
    double GetRadius()
        {return this->Sphere->GetRadius();}

    // Description:
    // Set/Get the label of the point 
    void SetLabel( const std::string & text );
    void SeNumericLabel(int n);
    const char* GetLabel()   { return this->Label->GetText();  }
    // Description:
    // Show/hide the label of the point 
    void ShowLabel( bool show);
    void UpdateLabelPosition();
  
    // Description:
    // Set the Label size;
    void SetLabelScale(double s);
    double *GetLabelScale();

    void SetPickable(int yes);
    int GetPickable();
    
    // Description:
    // Get the sphere properties - Property is normal, SelectedProperty is for highlighted 
    vtkGetObjectMacro(Property,vtkProperty);
    vtkGetObjectMacro(SelectedProperty,vtkProperty);
    vtkGetObjectMacro(DisabledProperty,vtkProperty);
    
    // Description:
    // set respective properties colors
    void SetPropertyColor(double color[3]);
    void SetSelectedPropertyColor(double color[3]);
    void SetDisabledPropertyColor(double color[3]);
    // Description:
    // set opacity
    void SetOpacity(double op);
    
    // Description:
    // Set the "hot spot" size; i.e., the region around the focus, in which the
    // motion vector is used to control the constrained sliding action. Note the
    // size is specified as a fraction of the length of the diagonal of the 
    // point widget's bounding box.
//    vtkSetClampMacro(HotSpotSize,double,0.0,1.0);
    vtkSetClampMacro(HotSpotSize,double,0.0,5.0);
    vtkGetMacro(HotSpotSize,double);
  
    // Description:
    // Set/Get current transform
    void SetTransform(vtkTransform * trans);
    vtkGetObjectMacro(Transform, vtkTransform);
    // Description:
    // highlight the point from outside
    void ExternalHighlight(int highlight);
    
    // Description:
    // find if HighlightedFromOutside flag is set
    vtkGetMacro(HighlightedFromOutside,int);
    vtkSetMacro(HighlightedFromOutside,int);
    
    // Description:
    // enable/disable interaction
    void EnableInteraction(int yes);
    
protected:
    vtkSphereWithTextPointWidget();
    virtual ~vtkSphereWithTextPointWidget();
    
    int PointIndex;
//BTX - manage the state of the widget
  
    int State;
    enum WidgetState
    {
        Start=0,
        Moving,
        Outside
    };
//ETX
    //hihglights the picked point
    void Highlight(int highlight);
    // Handles the events
    static void ProcessEvents(vtkObject* object, 
                              unsigned long event,
                              void* clientdata, 
                              void* calldata);

    // ProcessEvents() dispatches to these methods.
    virtual void OnMouseMove();
    virtual void OnLeftButtonDown();
    virtual void OnLeftButtonUp();
  
    // the sphere
    vtkActor          *Actor;
    vtkPolyDataMapper *Mapper;
    vtkSphereSource   *Sphere;
    int HighlightedFromOutside;

    // Do the picking
    vtkCellPicker *CursorPicker;
  
    // Methods to manipulate the sphere
    int ConstraintAxis;
    void MoveFocus(double *p1, double *p2);

    // Properties used to control the appearance of selected objects and
    // the manipulator in general.
    vtkProperty *Property;
    vtkProperty *SelectedProperty;
    vtkProperty *DisabledProperty;
    void CreateDefaultProperties();
  
    // The size of the hot spot.
    double HotSpotSize;
    int DetermineConstraintAxis(int constraint, double *x);
    int WaitingForMotion;
    int WaitCount;

    // The label
    vtkVectorText *Label;
    vtkFollower *LabelActor;
    vtkPolyDataMapper *LabelMapper;
    
    // user transformation
    vtkTransform *Transform;
  
private:
    
    vtkSphereWithTextPointWidget(const vtkSphereWithTextPointWidget&);  //Not implemented
    void operator=(const vtkSphereWithTextPointWidget&);  //Not implemented
};

#endif //TAG_VTKSPHEREWITHTEXTPOINTWIDGET_H

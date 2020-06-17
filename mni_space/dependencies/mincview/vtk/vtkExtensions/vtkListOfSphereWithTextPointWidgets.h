/*=========================================================================

  Program:   IGNS Visualization Toolkit
  Module:    $RCSfile: vtkListOfSphereWithTextPointWidgets.h,v $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/
// .NAME vtkListOfSphereWithTextPointWidgets - list of spherical points with labels
// .SECTION Description
// The list is in a vector form
// We have to be able to clear the list, add a point, remove a point, insert point,
// copy the list, get point at a given index, set point at a given index
// get number of points
// .SECTION See Also
// vtkphereWithTextPointWidget

#ifndef TAG_VTKLISTOFSPHEREWITHTEXTPOINTWIDGETS_H
#define TAG_VTKLISTOFSPHEREWITHTEXTPOINTWIDGETS_H

#include "vtkSphereWithTextPointWidget.h"
#include "vtkReal.h"
#include <vector>

class vtkListOfSphereWithTextPointWidgets : public vtkObject
{

public:

    static vtkListOfSphereWithTextPointWidgets * New();
    vtkTypeRevisionMacro(vtkListOfSphereWithTextPointWidgets,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // set/get number of points
    int GetNumberOfPoints() {return SpherePointList.size();}
    void SetNumberOfPoints(int n) {SpherePointList.resize(n);}

    // Description:
    // Reset - empty the list, but reserve memory
    void Reset( ); 
  
    // Description:
    // update positions and labels of points
    void Update();
    
    // Description:
    // insert point before the index
    void InsertPoint( int index, vtkSphereWithTextPointWidget * pt );
  
    // Description:
    // insert point at the end of the list
    void InsertNextPoint( vtkSphereWithTextPointWidget * pt );

    // Description:
    // remove point at index
    void RemovePoint( int index );
  
    // Description:
    // show/hide point
    void DisableEnablePoint( int index, bool enable );
    
    // Description:
    // get point at index
    vtkSphereWithTextPointWidget *GetPoint( unsigned int index ); 

    // Description:
    // set point at index, here we really want to change only the coordinates
    void SetPoint( int index, vtkSphereWithTextPointWidget * pt );
  
    // Description:
    // get/set point coordinates at index
    double *GetPointCoordinates( int index ); 
    void GetPointCoordinates( int index, double xyz[3] ); 
    void SetPointCoordinates( int index, double xyz[3] );
    void SetPointCoordinates( int index, double x, double y, double z );
  
    // Description:
    // get/set point label at index
    const char* GetPointLabel(int index);
    void  SetPointLabel(int index, const char* label);
    
    // Description:
    // set all points labels to sequential number 
    void  SeNumericLabels();
    
    // Description:
    // finds the highlighted point
    int GetHighlightedPointIndex();
    
    // Description:
    // highlight point
    void HighlightPoint(int index, int on);
    
    // Description:
    // enable/disable interaction
    void EnableInteraction(int yes);
    
    // Description:
    // enable/disable points - set individual point EnableOn/Off
    void EnableAllPoints(int yes);
    void EnableAllPointsOn();
    void EnableAllPointsOff();
    void EnablePoint(int index,int yes);
    void EnablePointOn(int index);
    void EnablePointOff(int index);
    // Description:
    // enable/disable picking on all points
    void EnablePickingOnAllPointsOn();
    void EnablePickingOnAllPointsOff();
    void EnablePickingOnAllPoints(int yes);
    // Description:
    // set point opacity
    void SetPointOpacity(int index, double op);
    // Description:
    // set all points opacity
    void SetAllPointsOpacity(double op);
    // Description:
    // transform all points on the list 
    void SetUserTransform( vtkTransform * tr );
    vtkTransform *GetUserTransform();
    // Description:
    // copy the list, 
    virtual void DeepCopy(vtkListOfSphereWithTextPointWidgets *list);
  
protected:

    vtkListOfSphereWithTextPointWidgets();
    virtual ~vtkListOfSphereWithTextPointWidgets();
    
    // Description:
    // List of points
    typedef std::vector<vtkSphereWithTextPointWidget *> SpherePointsVec;
    SpherePointsVec SpherePointList;
  
private:

    vtkListOfSphereWithTextPointWidgets(const vtkListOfSphereWithTextPointWidgets&);  // Not implemented.
    void operator=(const vtkListOfSphereWithTextPointWidgets&);  // Not implemented.
};

#endif   // TAG_VTKLISTOFSPHEREWITHTEXTPOINTWIDGETS_H

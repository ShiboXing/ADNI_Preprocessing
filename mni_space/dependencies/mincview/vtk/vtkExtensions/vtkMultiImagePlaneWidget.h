/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMultiImagePlaneWidget.h,v $
  Language:  C++
  Date:      $Date: 2009-02-21 01:18:31 $
  Version:   $Revision: 1.11 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkMultiImagePlaneWidget - 3D widget for reslicing image data
// .SECTION Description
// This 3D widget defines a plane that can be interactively placed in an
// image volume. A nice feature of the object is that the
// vtkMultiImagePlaneWidget, like any 3D widget, will work with the current
// interactor style. That is, if vtkMultiImagePlaneWidget does not handle an
// event, then all other registered observers (including the interactor
// style) have an opportunity to process the event. Otherwise, the
// vtkMultiImagePlaneWidget will terminate the processing of the event that it
// handles.
//
// The core functionality of the widget is provided by a vtkImageReslice
// object which passes its output onto a texture mapping pipeline for fast
// slicing through volumetric data. See the key methods: GenerateTexturePlane(),
// UpdateOrigin() and UpdateNormal() for implementation details.
//
// To use this object, just invoke SetInteractor() with the argument of the
// method a vtkRenderWindowInteractor.  You may also wish to invoke
// "PlaceWidget()" to initially position the widget. If the "i" key (for
// "interactor") is pressed, the vtkMultiImagePlaneWidget will appear. (See
// superclass documentation for information about changing this behavior.)
//
// Selecting the widget with the middle mouse button with and without holding
// the shift or control keys enables complex reslicing capablilites.
// To facilitate use, a set of 'margins' (left, right, top, bottom) are shown as
// a set of plane-axes aligned lines, the properties of which can be changed
// as a group.
// Without keyboard modifiers: selecting in the middle of the margins
// enables translation of the plane along its normal. Selecting one of the
// corners within the margins enables spinning around the plane's normal at its
// center.  Selecting within a margin allows rotating about the center of the
// plane around an axis aligned with the margin (i.e., selecting left margin
// enables rotating around the plane's local y-prime axis).
// With control key modifier: margin selection enables edge translation (i.e., a
// constrained form of scaling). Selecting within the margins enables
// translation of the entire plane.
// With shift key modifier: uniform plane scaling is enabled.  Moving the mouse
// up enlarges the plane while downward movement shrinks it.
//
// Window-level is achieved by using the right mouse button.
// The left mouse button can be used to query the underlying image data
// with a snap-to cross-hair cursor.  Currently, the nearest point in the input
// image data to the mouse cursor generates the cross-hairs.  With oblique
// slicing, this behaviour may appear unsatisfactory. Text display of
// window-level and image coordinates/data values are provided by a text
// actor/mapper pair.
// Events that occur outside of the widget (i.e., no part of the widget is
// picked) are propagated to any other registered obsevers (such as the
// interaction style). Turn off the widget by pressing the "i" key again
// (or invoke the Off() method).
//
// The vtkMultiImagePlaneWidget has several methods that can be used in
// conjunction with other VTK objects. The GetPolyData() method can be used
// to get the polygonal representation of the plane and can be used as input
// for other VTK objects. Typical usage of the widget is to make use of the
// StartInteractionEvent, InteractionEvent, and EndInteractionEvent
// events. The InteractionEvent is called on mouse motion; the other two
// events are called on button down and button up (either left or right
// button).
//
// Some additional features of this class include the ability to control the
// properties of the widget. You can set the properties of: the selected and
// unselected representations of the plane's outline; the text actor via its
// vtkTextProperty; the cross-hair cursor. In addition there are methods to
// constrain the plane so that it is aligned along the x-y-z axes.  Finally,
// one can specify the degree of interpolation (vtkImageReslice): nearest
// neighbour, linear, and cubic.

// .SECTION Thanks
// Thanks to Dean Inglis for developing and contributing this class.
// Based on the Python SlicePlaneFactory from Atamai, Inc.

// .SECTION Caveats
// Note that handles and plane can be picked even when they are "behind" other
// actors.  This is an intended feature and not a bug.

// .SECTION See Also
// vtk3DWidget vtkBoxWidget vtkLineWidget  vtkPlaneWidget vtkPointWidget
// vtkPolyDataSourceWidget vtkSphereWidget vtkImplicitPlaneWidget


#ifndef __vtkMultiImagePlaneWidget_h
#define __vtkMultiImagePlaneWidget_h

#include "vtkMulti3DWidget.h"
#include <vector>
#include "vtkReal.h"
#include "vtkMatrix4x4.h"

class vtkActor;
class vtkCellPicker;
class vtkDataSetMapper;
class vtkImageData;
class vtkImageMapToColors;
class vtkImageReslice;
class vtkLookupTable;
class vtkMatrix4x4;
class vtkPlaneSource;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkProperty;
class vtkTextActor;
class vtkTextProperty;
class vtkTexture;
class vtkTextureMapToPlane;
class vtkTransform;

#define VTK_NEAREST_RESLICE 0
#define VTK_LINEAR_RESLICE  1
#define VTK_CUBIC_RESLICE   2

class vtkMultiImagePlaneWidget : public vtkMulti3DWidget
{
public:
    // Description:
    // Instantiate the object.
    static vtkMultiImagePlaneWidget *New();

    vtkTypeRevisionMacro(vtkMultiImagePlaneWidget,vtkMulti3DWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Methods that satisfy the superclass' API.
    virtual void PlaceWidget(vtkReal bounds[6]);
    void PlaceWidget()
    {
        this->Superclass::PlaceWidget();
    }
    void PlaceWidget(vtkReal xmin, vtkReal xmax, vtkReal ymin, vtkReal ymax,vtkReal zmin, vtkReal zmax)
    {
        this->Superclass::PlaceWidget(xmin,xmax,ymin,ymax,zmin,zmax);
    }

    // Description:
    // Set the vtkImageData* input for the vtkImageReslice.
    void SetInput(vtkDataSet* input);

    // Description:
    // Set/Get the origin of the plane.
    void SetOrigin(vtkReal x, vtkReal y, vtkReal z);
    void SetOrigin(vtkReal xyz[3]);
    vtkReal* GetOrigin();
    void GetOrigin(vtkReal xyz[3]);

    // Description:
    // Set/Get the position of the point defining the first axis of the plane.
    void SetPoint1(vtkReal x, vtkReal y, vtkReal z);
    void SetPoint1(vtkReal xyz[3]);
    vtkReal* GetPoint1();
    void GetPoint1(vtkReal xyz[3]);

    // Description:
    // Set/Get the position of the point defining the second axis of the plane.
    void SetPoint2(vtkReal x, vtkReal y, vtkReal z);
    void SetPoint2(vtkReal xyz[3]);
    vtkReal* GetPoint2();
    void GetPoint2(vtkReal xyz[3]);

    // Description:
    // Get the center of the plane.
    vtkReal* GetCenter();
    void GetCenter(vtkReal xyz[3]);

    // Description:
    // Get the normal to the plane.
    vtkReal* GetNormal();
    void GetNormal(vtkReal xyz[3]);

    // Description:
    // Get the vector from the plane origin to point1.
    void GetVector1(vtkReal v1[3]);

    // Description:
    // Get the vector from the plane origin to point2.
    void GetVector2(vtkReal v2[3]);

    // Description:
    // Get the slice position in terms of the data extent.
    int GetSliceIndex();

    // Description:
    // Set the slice position in terms of the data extent.
    void SetSliceIndex(int index);

    // Description:
    // Get the position of the slice along its normal.
    vtkReal GetSlicePosition();

    // Description:
    // Set the position of the slice along its normal.
    void SetSlicePosition(vtkReal position);

    // Description:
    // Set the interpolation to use when texturing the plane.
    void SetResliceInterpolate(int);
    vtkGetMacro(ResliceInterpolate,int);
    void SetResliceInterpolateToNearestNeighbour()
    {
        this->SetResliceInterpolate(VTK_NEAREST_RESLICE);
    }
    void SetResliceInterpolateToLinear()
    {
        this->SetResliceInterpolate(VTK_LINEAR_RESLICE);
    }
    void SetResliceInterpolateToCubic()
    {
        this->SetResliceInterpolate(VTK_CUBIC_RESLICE);
    }

    // Description:
    // Convenience method to get the vtkImageReslice output.
    vtkImageData* GetResliceOutput();

    // Description:
    // Make sure that the plane remains within the volume.
    // Default is On.
    vtkSetMacro(RestrictPlaneToVolume,int);
    vtkGetMacro(RestrictPlaneToVolume,int);
    vtkBooleanMacro(RestrictPlaneToVolume,int);

    // Description:
    // Let the user control the lookup table. NOTE: apply this method BEFORE
    // applying the SetLookupTable method.
    // Default is Off.
    vtkSetMacro(UserControlledLookupTable,int);
    vtkGetMacro(UserControlledLookupTable,int);
    vtkBooleanMacro(UserControlledLookupTable,int);

    // Description:
    // Specify whether to interpolate the texture or not. When off, the
    // reslice interpolation is nearest neighbour regardless of how the
    // interpolation is set through the API. Set before setting the
    // vtkImageData imput. Default is On.
    vtkSetMacro(TextureInterpolate,int);
    vtkGetMacro(TextureInterpolate,int);
    vtkBooleanMacro(TextureInterpolate,int);

    // Description:
    // Control the visibility of the actual texture mapped reformatted plane.
    // in some cases you may only want the plane outline for example.
    virtual void SetTextureVisibility(int);
    vtkGetMacro(TextureVisibility,int);
    vtkBooleanMacro(TextureVisibility,int);

    // Description:
    // Grab the polydata (including points) that defines the plane.  The
    // polydata consists of (res+1)*(res+1) points, and res*res quadrilateral
    // polygons, where res is the resolution of the plane. These point values
    // are guaranteed to be up-to-date when either the InteractionEvent or
    // EndInteraction events are invoked. The user provides the vtkPolyData and
    // the points and polyplane are added to it.
    void GetPolyData(vtkPolyData *pd);

    // Description:
    // Satisfies superclass API.  This will change the state of the widget to
    // match changes that have been made to the underlying PolyDataSource
    void UpdatePlacement(void);

    // Description:
    // Convenience method to get the texture used by this widget.  This can be
    // used in external slice viewers.
    vtkTexture *GetTexture();

    // Description:
    // Convenience method to get the vtkImageMapToColors filter used by this
    // widget.  The user can properly render other transparent actors in a
    // scene by calling the filter's SetOuputFormatToRGB and
    // PassAlphaToOutputOff.
    vtkGetObjectMacro(ColorMap, vtkImageMapToColors);
    virtual void SetColorMap(vtkImageMapToColors *);

    // Description:
    // Set/Get the plane's outline properties. The properties of the plane's
    // outline when selected and unselected can be manipulated.
    virtual void SetPlaneProperty(vtkProperty*);
    vtkGetObjectMacro(PlaneProperty,vtkProperty);
    virtual void SetSelectedPlaneProperty(vtkProperty*);
    vtkGetObjectMacro(SelectedPlaneProperty,vtkProperty);

    // Description:
    // Convenience method sets the plane orientation normal to the
    // x, y, or z axes.  Default is XAxes (0).
    void SetPlaneOrientation(int);
    vtkGetMacro(PlaneOrientation,int);
    void SetPlaneOrientationToXAxes()
    {
        this->SetPlaneOrientation(0);
    }
    void SetPlaneOrientationToYAxes()
    {
        this->SetPlaneOrientation(1);
    }
    void SetPlaneOrientationToZAxes()
    {
        this->SetPlaneOrientation(2);
    }

    // Description:
    // Set the internal picker to one defined by the user.  In this way,
    // a set of three orthogonal planes can share the same picker so that
    // picking is performed correctly.  The default internal picker can be
    // re-set/allocated by setting to 0 (NULL).
    void SetPicker( int index, vtkCellPicker * picker );

    // Description:
    // Set/Get the internal lookuptable (lut) to one defined by the user, or,
    // alternatively, to the lut of another vtkImgePlaneWidget.  In this way,
    // a set of three orthogonal planes can share the same lut so that
    // window-levelling is performed uniformly among planes.  The default
    // internal lut can be re- set/allocated by setting to 0 (NULL).
    virtual void SetLookupTable(vtkLookupTable*);
    vtkGetObjectMacro(LookupTable,vtkLookupTable);

    // Description:
    // Enable/disable text display of window-level, image coords and values in a
    // render window.
    vtkSetMacro(DisplayText,int);
    vtkGetMacro(DisplayText,int);
    vtkBooleanMacro(DisplayText,int);

    // Description:
    // Set the properties of the cross-hair cursor.
    virtual void SetCursorProperty(vtkProperty*);
    vtkGetObjectMacro(CursorProperty,vtkProperty);

    // Description:
    // Set the properties of the margins.
    virtual void SetMarginProperty(vtkProperty*);
    vtkGetObjectMacro(MarginProperty,vtkProperty);

    // Description:
    // Set/Get the text property for the image data and window-level annotation.
    void SetTextProperty(vtkTextProperty* tprop);
    vtkTextProperty* GetTextProperty();

    // Description:
    // Set/Get the property for the resliced image.
    virtual void SetTexturePlaneProperty(vtkProperty*);
    vtkGetObjectMacro(TexturePlaneProperty,vtkProperty);

    // Description:
    // Set/Get the current window and level values.  Set should
    // only be called after SetInput.
    void SetWindowLevel(vtkReal window, vtkReal level);
    void GetWindowLevel(vtkReal wl[2]);

    // Description:
    // Get the image coordinate position and voxel value.  Currently only
    // supports single component image data.
    int GetCursorData(vtkReal xyzv[4]);

    // Description:
    // Set action associated to buttons.
    //BTX
    enum
    {
        CURSOR_ACTION       = 0,
        SLICE_MOTION_ACTION = 1,
        WINDOW_LEVEL_ACTION = 2
    };
    //ETX
    vtkSetClampMacro(LeftButtonAction,int, CURSOR_ACTION, WINDOW_LEVEL_ACTION);
    vtkGetMacro(LeftButtonAction, int);
    vtkSetClampMacro(MiddleButtonAction,int, CURSOR_ACTION, WINDOW_LEVEL_ACTION);
    vtkGetMacro(MiddleButtonAction, int);
    vtkSetClampMacro(RightButtonAction,int, CURSOR_ACTION, WINDOW_LEVEL_ACTION);
    vtkGetMacro(RightButtonAction, int);

    // Description:
    // Set the auto-modifiers associated to buttons.
    // This allows users to bind some buttons to actions that are usually
    // triggered by a key modifier. For example, if you do not need cursoring,
    // you can bind the left button action to SLICE_MOTION_ACTION (see above)
    // and the left button auto modifier to CONTROL_MODIFIER: you end up with
    // the left button controling panning without pressing a key.
    //BTX
    enum
    {
        NO_MODIFIER         = 0,
        SHIFT_MODIFIER      = 1,
        CONTROL_MODIFIER    = 2
    };
    //ETX
    vtkSetClampMacro(LeftButtonAutoModifier,int, SHIFT_MODIFIER, CONTROL_MODIFIER);
    vtkGetMacro(LeftButtonAutoModifier, int);
    vtkSetClampMacro(MiddleButtonAutoModifier,int, SHIFT_MODIFIER, CONTROL_MODIFIER);
    vtkGetMacro(MiddleButtonAutoModifier, int);
    vtkSetClampMacro(RightButtonAutoModifier,int, SHIFT_MODIFIER, CONTROL_MODIFIER);
    vtkGetMacro(RightButtonAutoModifier, int);

    // Description:
    // Added by Anka in order to get world coordinates of points picked on the plane
    // with the right mouse button
    
    vtkActor *GetTexturePlaneActor(int index);
    void GetCurrentWorldPosition( vtkReal* pos);
    vtkTextureMapToPlane *GetTexturePlaneCoords() {return TexturePlaneCoords;}
    vtkImageReslice *GetReslice() {return Reslice;}
    vtkGetObjectMacro(ResliceAxes, vtkMatrix4x4); 
    vtkSetObjectMacro(ResliceAxes, vtkMatrix4x4);
    vtkSetClampMacro(PlaneType,int, 0, 2);
    vtkGetMacro(PlaneType, int);
    void ExternalUpdateCursor(int xy[2], vtkRenderer *ren);
    void ExternalActivateCursor(int active);
        
protected:

    vtkMultiImagePlaneWidget();
    ~vtkMultiImagePlaneWidget();

    // Description:
    // Let subclasses react the the addition of interactors
    void InternalAddRenderer( vtkRenderer * ren, vtkAssembly * assembly );
    void InternalRemoveRenderer( int index );

    void InternalEnable();
    void InternalDisable();
    void DisableForOneRenderer( int rendererIndex );

    void ClearActors();

    int PlaneType; // used for ibis
    
    int TextureVisibility;

    int LeftButtonAction;
    int MiddleButtonAction;
    int RightButtonAction;

    int LeftButtonAutoModifier;
    int MiddleButtonAutoModifier;
    int RightButtonAutoModifier;

    //BTX - manage the state of the widget
    int State;
    enum WidgetState
    {
        Start=0,
        Cursoring,
        WindowLevelling,
        Pushing,
        Spinning,
        Rotating,
        Moving,
        Scaling,
        Outside
    };
    //ETX


    // Catch events from vtk3DWidget2
    virtual void OnMouseMove();
    virtual void OnLeftButtonDown();
    virtual void OnLeftButtonUp();
    virtual void OnMiddleButtonDown();
    virtual void OnMiddleButtonUp();
    virtual void OnRightButtonDown();
    virtual void OnRightButtonUp();

    virtual void StartCursor();
    virtual void StopCursor();
    virtual void StartSliceMotion();
    virtual void StopSliceMotion();
    virtual void StartWindowLevel();
    virtual void StopWindowLevel();

    // controlling ivars
    int   Interaction; // Is the widget responsive to mouse events
    int   PlaneOrientation;
    int   RestrictPlaneToVolume;
    vtkReal OriginalWindow;
    vtkReal OriginalLevel;
    vtkReal CurrentWindow;
    vtkReal CurrentLevel;
    int   ResliceInterpolate;
    int   TextureInterpolate;
    int   UserControlledLookupTable;
    int   DisplayText;

    typedef std::vector<vtkActor*> ActorVec;
    typedef std::vector<vtkPolyDataMapper*> PolyDataMapperVec;

    // The geometric represenation of the plane and it's outline
    vtkPlaneSource    *PlaneSource;
    vtkReal             Normal[3]; // plane normal normalized
    vtkPolyData       *PlaneOutlinePolyData;
    ActorVec           PlaneOutlineActors;
    PolyDataMapperVec  PlaneOutlineMappers;
    void               HighlightPlane(int highlight);
    void               GeneratePlaneOutline();

    // Re-builds the plane outline based on the plane source
    void BuildRepresentation();

    // Do the picking
    typedef std::vector<vtkCellPicker*> PickerVec;
    PickerVec PlanePickers;

    // Methods to manipulate the plane
    void WindowLevel(int X, int Y);
    void Push(vtkReal *p1, vtkReal *p2);
    void Spin(vtkReal *p1, vtkReal *p2);
    void Rotate(vtkReal *p1, vtkReal *p2, vtkReal *vpn);
    void Scale(vtkReal *p1, vtkReal *p2, int X, int Y);
    void Translate(vtkReal *p1, vtkReal *p2);

    vtkImageData         *ImageData;
    vtkImageReslice      *Reslice;
    vtkMatrix4x4         *ResliceAxes;
    vtkTransform         *Transform;
    vtkTextureMapToPlane *TexturePlaneCoords;
    PolyDataMapperVec     TexturePlaneMappers;
    ActorVec              TexturePlaneActors;
    vtkImageMapToColors  *ColorMap;
    vtkTexture           *Texture;
    vtkLookupTable       *LookupTable;
    vtkLookupTable       *CreateDefaultLookupTable();

    // Properties used to control the appearance of selected objects and
    // the manipulator in general.  The plane property is actually that for
    // the outline.  The TexturePlaneProperty can be used to control the
    // lighting etc. of the resliced image data.
    vtkProperty   *PlaneProperty;
    vtkProperty   *SelectedPlaneProperty;
    vtkProperty   *CursorProperty;
    vtkProperty   *MarginProperty;
    vtkProperty   *TexturePlaneProperty;
    void           CreateDefaultProperties();

    // Reslice and texture management
    void UpdateNormal();
    void UpdateOrigin();
    void GenerateTexturePlane();

    // The cross-hair cursor
    vtkPolyData       *CursorPolyData;
    PolyDataMapperVec  CursorMappers;
    ActorVec           CursorActors;
    int                CurrentCursorPosition[3];
    vtkReal            CurrentImageValue; // Set to VTK_FLOAT_MAX when invalid
    vtkReal            CurrentWorldPosition[3]; // Added by Anka
    void               GenerateCursor();
    void               UpdateCursor(int,int);
    void               ActivateCursor(int);

    // The text to display W/L, image data
    vtkTextProperty * TextProperties;
    typedef std::vector<vtkTextActor*> TextActorVec;
    TextActorVec  TextActors;
    char          TextBuff[128];
    void          ManageTextDisplay();
    void          ActivateText(int);

    // Oblique reslice control
    vtkReal RotateAxis[3];
    vtkReal RadiusVector[3];
    void  AdjustState();

    // Visible margins to assist user interaction
    vtkPolyData       *MarginPolyData;
    PolyDataMapperVec  MarginMappers;
    ActorVec           MarginActors;
    int                MarginSelectMode;
    void               GenerateMargins();
    void               UpdateMargins();
    void               ActivateMargins(int);
    
    // Utility function used to transform a vector from
    // world space to input image space.
    void TransformToImageSpace( vtkReal * in, vtkReal * out );

private:
    vtkMultiImagePlaneWidget(const vtkMultiImagePlaneWidget&);  //Not implemented
    void operator=(const vtkMultiImagePlaneWidget&);  //Not implemented
};

#endif //__vtkMultiImagePlaneWidget_h

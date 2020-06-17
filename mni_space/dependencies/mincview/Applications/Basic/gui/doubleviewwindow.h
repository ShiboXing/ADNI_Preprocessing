/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: doubleviewwindow.h,v $
  Language:  C++
  Date:      $Date: 2010-04-08 14:56:03 $
  Version:   $Revision: 1.12 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#ifndef TAG_DOUBLEVIEWWINDOW_H
#define TAG_DOUBLEVIEWWINDOW_H

#include "doubleviewwindowbase.h"
#include <qwidget.h>
#include "vtkObject.h"
#include "vtkReal.h"

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
// IGSTK headers
#include "igstkObject.h"
#include "igstkMacros.h"
#endif

class vtkQtRenderWindow;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkGrid;
class vtkOutlineFilter;
class vtkActor;
class vtkPolyDataMapper;
class vtkImageData;
class vtkImageReslice;
class vtkTransform;
class vtkLookupTable;
class vtkImageShiftScale;
class vtkImageMapToWindowLevelColors;
class vtkImageActor;
class vtkImageBlend;

extern const int defaultWinX;
extern const int defaultWinY;
extern const vtkReal defaultScale;
extern const int maxGridStep;
extern const int minGridStep;
extern const int defaultGridStep;
extern const int defaultOpacity;

class DoubleViewWindow : public DoubleViewWindowBase, public vtkObject
{
    Q_OBJECT

public:

    DoubleViewWindow( QWidget * parent = 0, const char * name = 0, WFlags fl = 0 );
    virtual ~DoubleViewWindow();
    
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    /** Set up variables, types and methods related to the Logger */
    igstkLoggerMacro()
#endif

    vtkQtRenderWindow * m_usImageWindow;
    vtkQtRenderWindow * m_mriImageWindow;

    virtual void InitializeGrid();
    virtual void SetGridActors();
    virtual void InitializeVariables();
    virtual void InitializeActors();
       
    virtual void SetImageData(vtkImageData *data);
    virtual void SetTransform(vtkTransform *t);
    virtual vtkTransform * GetTransform() {return m_transform;}
    virtual void SetDefaultView(vtkImageActor *actor, vtkRenderer *renderer);
   
public slots:
    
    virtual void RestoreViewsButtonClicked();
    virtual void CloseButtonClicked();
    virtual void GridCheckBoxToggled();
    virtual void BlendCheckBoxToggled();
    virtual void DecreaseGridStepButtonClicked();
    virtual void IncreaseGridStepButtonClicked();
    virtual void GridStepEditChanged();
    virtual void GridStepSliderValueChanged(int);
    virtual void DecreaseOpacityButtonClicked();
    virtual void IncreaseOpacityButtonClicked();
    virtual void OpacityLineEditTextChanged();
    virtual void OpacitySliderValueChanged(int);
    virtual void UpdateViews();
    virtual void EnableBlendingButton();
    virtual void DisableBlending();
    virtual void UnBlend();
    virtual void RestoreGridAndBlending();
    
protected:
    
    vtkImageData *m_imageData;
    vtkTransform *m_transform;
    vtkImageReslice *m_reslice;
    vtkImageShiftScale *m_displayImage;        
    vtkImageMapToWindowLevelColors *m_displayWindowLevelColorsImage;
    vtkReal m_win_level[2];
    vtkImageActor *m_usActor;
    vtkImageActor *m_mriActor;
    vtkImageBlend *m_blendedImage;
    vtkLookupTable *m_usLut;
    
    vtkRenderWindowInteractor * m_usInteractor;
    vtkRenderWindowInteractor * m_mriInteractor;
    vtkRenderer *m_usRenderer;
    vtkRenderer *m_mriRenderer;
    vtkGrid  *m_grid;
    vtkActor *m_usGridActor;
    vtkActor *m_mriGridActor;
    vtkActor *m_usOutlineActor;
    vtkOutlineFilter *m_usOutlineFilter;
    vtkPolyDataMapper *m_usOutlineMapper;
    vtkActor *m_mriOutlineActor;
    vtkOutlineFilter *m_mriOutlineFilter;
    vtkPolyDataMapper *m_mriOutlineMapper;
    
    bool m_blending;
    bool m_gridOn;
    int  m_opacity;  
};

#endif //TAG_DOUBLEVIEWWINDOW_H

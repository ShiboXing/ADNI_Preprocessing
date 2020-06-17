/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: doublewindowfileviewer.h,v $
  Language:  C++
  Date:      $Date: 2010-03-31 14:16:47 $
  Version:   $Revision: 1.10 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#ifndef TAG_DOUBLEWINDOWFILEVIEWER_H_
#define TAG_DOUBLEWINDOWFILEVIEWER_H_

#include <qwidget.h>
#include <qstring.h>
#include "doubleviewwindow.h"

class vtkTransform;
class vtkImageData;
class vtkImagePlaneActor;
class vtkMatrix4x4;
class vtkImageMapToColors;
class vtkImageMask;

class DoubleWindowFileViewer :  public DoubleViewWindow
{
    Q_OBJECT

public:
    DoubleWindowFileViewer( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~DoubleWindowFileViewer();
    
    virtual void RestoreViewsButtonClicked();
    virtual void InitializeActors();
    virtual void SetDefaultView(vtkImageActor *actor, vtkRenderer *renderer);
    virtual void InitializeGrid();
    virtual void RestoreGridAndBlending();
    void SetUltrasoundImage(vtkImageData *image);
    void SetImageData(vtkImageData *data, vtkLookupTable *lut);
    void SetResliceTransform(vtkTransform *resTransform);
    void SetResliceSpacing(double stepX, double stepY, double step);
    void InitLeftView();
    void InitRightView(vtkLookupTable *lut=0);
    void SetRightImageWindow(int);
    void SetRightImageLevel(int);
    void SetRightImageColor(vtkLookupTable *lut);
    void SetLeftImageWindow(int);
    void SetLeftImageLevel(int);
    void SetLeftImageColor(vtkLookupTable *lut);
    void SetRightImageTransform(vtkTransform *t);
    void SetRightImageTransform(vtkMatrix4x4 *mat);
    void UpdateBlendedImage();
    vtkImageData *GetCurrentUSImage() {return m_currentUsImage;}
    void SetMask(const QString &maskFile);
    void SetGridScale(vtkReal scale[3]);
    void EnableGrid(int gridOn);
    
public slots:

    virtual void CloseButtonClicked();
    virtual void BlendCheckBoxToggled();
    virtual void GridCheckBoxToggled();
    
protected:
    
    vtkImageData *m_currentUsImage;
    vtkImageMapToColors *m_greenUS;
    vtkImageMapToWindowLevelColors *m_usWindowLevelColorsImage;
    vtkImageMask *m_maskedImage;
    int m_initLeftViewDone;
    int m_initRightViewDone;
    vtkTransform *m_imageTransform;
    vtkTransform *m_inputResliceTransform;
    vtkReal m_us_win_level[2];
    vtkReal m_gridScale[3];
};
    
#endif //TAG_DOUBLEWINDOWFILEVIEWER_H_

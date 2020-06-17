/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: pointsobject.h,v $
  Language:  C++
  Date:      $Date: 2010-04-28 19:06:03 $
  Version:   $Revision: 1.5 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
// .NAME pointsobject 
// .SECTION Description
// Object managed by scenemanager, composed of points represented by spheres in 3D view
// Points are selected from an object vtkImageData * m_imageData;
/*=========================================================================*/

#ifndef POINTSOBJECT_H
#define POINTSOBJECT_H

#include <sceneobject.h>
#include "vtkListOfSphereWithTextPointWidgets.h"
#include "vtkRenderWindowInteractor.h"
#include "movepointcommand.h"
#include "serializer.h"
#include "vtkReal.h"

class vtkImageData;
class vtkSphereWithTextPointWidget;
class ImageObject;
class PickRefiner;
class View;

struct POINT_PROPERTIES
{
    int index;
    vtkReal x;
    vtkReal y;
    vtkReal z;
};

enum POINTS_TYPE {REGULAR, REGISTRATION_SOURCE, REGISTRATION_TARGET};

class PointsObject : public SceneObject
{
Q_OBJECT

public:
    static PointsObject * New() { return new PointsObject; }
    
    PointsObject();
    virtual ~PointsObject();

    void Serialize( Serializer * ser );
    
    virtual QWidget * CreateSettingsDialog( QWidget * parent ); 
    virtual void HideShow();
    virtual void WriteTagFile(const QString & filename, const char *timeStamp = 0);
    virtual void Release( View * view); 
    virtual void Update();
    virtual void Setup( View * view, vtkAssembly * placementHint );

    vtkListOfSphereWithTextPointWidgets *GetPointList() {return m_pointList;}
    void UpdateSettingsWidget();
    
    // wrapper functions for vtkListOfSphereWithTextPointWidgets
    vtkSphereWithTextPointWidget *GetPoint(int index);
    void AddPoint(const QString &name, vtkReal coords[3]);
    void AddPoint(int index, int numericLabel, vtkReal coords[3]);
    void AddPoint(vtkSphereWithTextPointWidget *point);
    void RemovePoint(int index);
    void RemoveAllPoints();
    void SetPointCoordinatesAndUpdate(int idx, vtkReal *pt);
    void SetPointPickable(int index, int yes);
        
    void SetImageObject(ImageObject * obj);
    void SetImageObject(SceneObject * obj);
    void RemoveImageObject();
    ImageObject *GetImageObject() {return m_imageObject;}
    void SetInteractor(vtkRenderWindowInteractor * iteract);
    
    vtkReal GetPointRadius() {return m_pointRadius;}
    void SetPointRadius(vtkReal r);
    vtkReal GetLabelSize() {return m_labelSize;}
    void SetLabelSize(vtkReal s);
    void SetShowLabel(int show);
    int GetShowLabel() {return m_showLabel;}
    QString GetImageObjectName() {return m_imageObjectName;}
    
    void UpdatePoints();
    int GetNumberOfPoints();
    void HighlightPoint(int index, int on);
    int GetHighlightedPointIndex();
    
    const char* GetPointLabel(int index);
    const char* GetHighlightedPointLabel();
    void GetPointCoordinates( int index, vtkReal xyz[3] );
    vtkReal *GetPointCoordinates( int index );
    // correcting points position  and viewing in all 4 views
    void SetCorrectionMode(int mode);
    int GetCorrectionMode() {return m_inCorrectionMode;}
    
    void CopyPointList(vtkListOfSphereWithTextPointWidgets *sourceList);

    void EnablePickRefiner(int enable);
    void ShowCrosshair(int show);
    void SetImagePlanesPosition(int idx, vtkReal posX, vtkReal posY, vtkReal posZ);
    void OverrideMouseActions();
    void RestoreMouseActions();
    void MovePoint(POINT_PROPERTIES* point);
    bool GetEnableRefining() {return m_enableRefining;}
    void EnablePickingOnAllPoints(int yes);

    void ActivateCursorOnAllPlanes(int active);
    void ShowPointInAllViews(int index);
    
    MovePointCommand *GetMovePointCommand() {return m_movePointCommand;}
    void MouseInteraction(unsigned long event);

#define MIN_RADIUS 1
#define MAX_RADIUS 10
#define MIN_LABEl_SIZE 6
#define MAX_LABEL_SIZE 16
    void SetPointsType(POINTS_TYPE type) {m_points_type = type;}
    POINTS_TYPE GetPointsType() {return m_points_type;}

    void SetEnabledColor(vtkReal color[3]);
    void GetEnabledColor(vtkReal color[3]);
    vtkReal *GetEnabledColor() {return m_enabledColor;}
    void SetDisabledColor(vtkReal color[3]);
    void GetDisabledColor(vtkReal color[3]);
    vtkReal *GetDisabledColor() {return m_disabledColor;}
    void SetSelectedColor(vtkReal color[3]);
    void GetSelectedColor(vtkReal color[3]);
    vtkReal *GetSelectedColor() {return m_selectedColor;}
    void SetOpacity(vtkReal opacity);
    vtkReal GetOpacity() {return m_opacity;}
    
public slots:

    void TransformationChanged(QString, int);
    void UndoLastTransformation(int);

signals:
    void PointsChanged();
    void PointRemoved();
    void PointsUpdated();
    void RestoreSettingsWidget(int index);

protected:
    vtkListOfSphereWithTextPointWidgets *m_pointList;
    ImageObject *m_imageObject;
    vtkImageData * m_imageData;
    vtkRenderWindowInteractor * m_interactor;
    vtkReal m_pointRadius;
    vtkReal m_labelSize;
    int m_showLabel;
    bool m_enableRefining;
    PickRefiner *m_pickRefiner;
    QString m_imageObjectName;
    MovePointCommand *m_movePointCommand;
    int m_inCorrectionMode;
    POINTS_TYPE m_points_type;
    vtkReal m_enabledColor[3];
    vtkReal m_disabledColor[3];
    vtkReal m_selectedColor[3];
    vtkReal m_opacity;
};

ObjectSerializationHeaderMacro( PointsObject );

#endif //POINTSOBJECT_H

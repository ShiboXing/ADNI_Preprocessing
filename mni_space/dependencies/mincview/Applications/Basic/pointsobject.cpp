/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: pointsobject.cpp,v $
  Language:  C++
  Date:      $Date: 2010-04-29 18:04:29 $
  Version:   $Revision: 1.9 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.
  
=========================================================================*/
#include "pointsobject.h"
#include "imageobject.h"
#include "vtkImageData.h"
#include "vtkCoordinate.h"
#include "vtkRenderer.h"
#include "pointsobjectsettingsdialog.h"
#include "vtkTagWriter.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"
#include "pickrefiner.h"
#include "vtkIgnsEvents.h"
#include "view.h"
#include "scenemanager.h"
#include "version.h"
#include "usertransformations.h"

ObjectSerializationMacro( PointsObject );

PointsObject::PointsObject()
 : SceneObject()
{
    this->ObjectType = POINTS_OBJ_TYPE;
    m_imageObject = 0;
    m_imageData = 0;
    m_pointList = vtkListOfSphereWithTextPointWidgets::New();
    m_interactor = 0;
    m_pointRadius = 2.0;
    m_labelSize = 8.0;
    m_showLabel = 1;
    m_enableRefining = false;
    m_pickRefiner = 0;
    m_imageObjectName = "none";
    m_movePointCommand = MovePointCommand::New();
    m_movePointCommand->SetPointsObject(this);
    m_inCorrectionMode = 0;
    m_points_type = REGULAR;
    this->AppliedTransforms.clear();
    
    m_enabledColor[0] = 0.0;
    m_enabledColor[1] = 0.0;
    m_enabledColor[2] = 1.0;
    m_disabledColor[0] = 0.75;
    m_disabledColor[1] = 0.5;
    m_disabledColor[2] = 0.0;
    m_selectedColor[0] = 0.0;
    m_selectedColor[1] = 1.0;
    m_selectedColor[2] = 0.0;
    m_opacity = 1.0;
}


PointsObject::~PointsObject()
{
    if (m_pickRefiner)
        m_pickRefiner->Delete();
    if (m_pointList)
        m_pointList->Delete();
    if(m_interactor)
        m_interactor->UnRegister(this);
    if(m_imageObject)
        m_imageObject->UnRegister(this);
    m_movePointCommand->Delete();
}

void PointsObject::Serialize( Serializer * ser )
{
    SceneObject::Serialize(ser);
    ::Serialize( ser, "ImageObjectName", m_imageObjectName );
    ::Serialize( ser, "PointRadius", m_pointRadius );
    ::Serialize( ser, "LabelSize", m_labelSize );
    ::Serialize( ser, "ShowLabel", m_showLabel );

    if (!ser->IsReader() || QString::compare(this->Manager->GetSceneSaveVersionFromFile(), "1.0") > 0)
    {
        ::Serialize( ser, "EnabledColor", m_enabledColor, 3 );
        ::Serialize( ser, "DisabledColor", m_disabledColor, 3 );
        ::Serialize( ser, "SelectededColor", m_selectedColor, 3 );
        ::Serialize( ser, "Opacity", m_opacity );
    }
}

void PointsObject::Release( View * view )
{
    if (view->GetType() == THREED_VIEW_TYPE)
    {
        if (m_pickRefiner && m_inCorrectionMode > 0)
            m_pickRefiner->HideCrosshair();
        if (m_pointList)
        {
            m_pointList->Delete();
            m_pointList = 0;
        }
        view->NotifyNeedRender();
    }
}

void PointsObject::Update()
{
    m_pointList->Update();
}

void PointsObject::Setup( View * view, vtkAssembly * placementHint )
{
    connect( this, SIGNAL( Modified() ), view, SLOT( NotifyNeedRender() ) );
}

void PointsObject::SetImageObject(ImageObject * image)
{
    if(m_imageObject == image)
        return;
    if(m_imageObject)
        m_imageObject->UnRegister(this);
    m_imageObject = image;
    if(m_imageObject)
        m_imageObject->Register(this);
    m_imageData = m_imageObject->GetImage();
    m_imageObjectName = m_imageObject->GetName();
    vtkReal spacing[3];
    m_imageData->GetSpacing(spacing);
    m_pointRadius =  spacing[0];
    if (m_pointRadius < 1)
        m_pointRadius = 1;
}

void PointsObject::SetImageObject(SceneObject * obj)
{
    if (!obj->GetObjectType() & MINC_TYPE)
        return;
    ImageObject *image = reinterpret_cast<ImageObject*>(obj);        
    if(m_imageObject == image)
        return;
    if(m_imageObject)
        m_imageObject->UnRegister(this);
    m_imageObject = image;
    if(m_imageObject)
        m_imageObject->Register(this);
    m_imageData = m_imageObject->GetImage();
    m_imageObjectName = m_imageObject->GetName();
    vtkReal spacing[3];
    m_imageData->GetSpacing(spacing);
    m_pointRadius =  spacing[0];
    if (m_pointRadius < 1)
        m_pointRadius = 1;
}

void PointsObject::RemoveImageObject()
{
    m_imageObjectName = "none";
    m_imageData = 0;
    if(m_imageObject)
    {
        m_imageObject->UnRegister(this);
        m_imageObject = 0;
    }
    
}

void PointsObject::SetInteractor(vtkRenderWindowInteractor * iteract)
{
    if(m_interactor == iteract)
        return;
    if(m_interactor)
        m_interactor->UnRegister(this);
    m_interactor = iteract;
    if(m_interactor)
        m_interactor->Register(this);
}

void PointsObject::SetPointRadius(vtkReal r)
{
    if (m_pointRadius == r)
        return;
    if (r < MIN_RADIUS)
        r = MIN_RADIUS;
    if (r > MAX_RADIUS)
        r = MAX_RADIUS;
    m_pointRadius = r;
    this->UpdatePoints();
}

void PointsObject::SetLabelSize(vtkReal s)
{
    if (m_labelSize == s)
        return;
    if (s < MIN_LABEl_SIZE)
        s = MIN_LABEl_SIZE;
    if (s > MAX_LABEL_SIZE)
        s = MAX_LABEL_SIZE;
    m_labelSize = s;
    this->UpdatePoints();
}

void PointsObject::SetShowLabel(int show)
{
    if (m_showLabel == show)
        return;
    m_showLabel = show;
    this->UpdatePoints();
}

int PointsObject::GetNumberOfPoints()
{
    if (m_pointList)
        return m_pointList->GetNumberOfPoints();
    return 0;
}

vtkSphereWithTextPointWidget * PointsObject::GetPoint(int index)
{
    return m_pointList->GetPoint(index);
}

const char* PointsObject::GetPointLabel(int index)
{
    return m_pointList->GetPointLabel(index);
}

const char* PointsObject::GetHighlightedPointLabel()
{
    int index = m_pointList->GetHighlightedPointIndex();
    if (index >= 0)
        return m_pointList->GetPointLabel(index);
    else
        return 0;
}

void PointsObject::GetPointCoordinates(int index, vtkReal xyz[3])
{
    m_pointList->GetPointCoordinates(index, xyz);
}

vtkReal * PointsObject::GetPointCoordinates( int index )
{
    return m_pointList->GetPointCoordinates(index);
}

void PointsObject::AddPoint(const QString &name, vtkReal coords[3])
{
    vtkSphereWithTextPointWidget *newPoint = vtkSphereWithTextPointWidget::New();
    if (m_points_type != REGISTRATION_TARGET)
    {
        SceneObject *rootObj = this->Manager->GetObject(PREOP_ROOT_OBJECT_NAME);
        if (rootObj)
            newPoint->SetTransform(rootObj->GetTransform());
        if(m_imageObject && m_imageData)
        {
            newPoint->SetInput(m_imageData);
            if (!this->Manager->GetRegistrationFlag())
                newPoint->SetTransform(m_imageObject->GetCurrentTransform());
            newPoint->PlaceWidget();
        }
        else
            newPoint->PlaceWidget(coords[0]-m_pointRadius-2, coords[0]+m_pointRadius+2, coords[1]-m_pointRadius-2, coords[1]+m_pointRadius+2, coords[2]-m_pointRadius-2, coords[2]+m_pointRadius+2);
    }
    else
        newPoint->PlaceWidget(coords[0]-m_pointRadius-2, coords[0]+m_pointRadius+2, coords[1]-m_pointRadius-2, coords[1]+m_pointRadius+2, coords[2]-m_pointRadius-2, coords[2]+m_pointRadius+2);
    newPoint->SetInteractor(m_interactor);
    newPoint->SetLabel(name);
    newPoint->SetEnabled(1);
    newPoint->SetPickable(0);
    newPoint->ShowLabel(m_showLabel);
    newPoint->SetRadius(m_pointRadius);
    newPoint->SetLabelScale(m_labelSize);
    newPoint->SetHotSpotSize(m_pointRadius+0.25*m_pointRadius);
    newPoint->SetPosition(coords[0], coords[1], coords[2]);
    newPoint->SetOpacity(m_opacity);
/*    if (m_points_type == REGISTRATION_TARGET)
        newPoint->SetOpacity(0.5);*/
    newPoint->SetPropertyColor(m_enabledColor);
    newPoint->SetDisabledPropertyColor(m_disabledColor);
    newPoint->SetSelectedPropertyColor(m_selectedColor);
    newPoint->Update();
    m_pointList->InsertNextPoint(newPoint);
    int num = m_pointList->GetNumberOfPoints();
    if (m_points_type != REGISTRATION_TARGET)
        m_pointList->HighlightPoint(num-1, 1);
    newPoint->SetPointIndex(num-1);
    newPoint->Delete();
    emit PointsChanged();
    emit Modified();
}

void PointsObject::AddPoint(int index, int numericLabel, vtkReal coords[3])
{
    vtkSphereWithTextPointWidget *newPoint = vtkSphereWithTextPointWidget::New();
    if (m_points_type != REGISTRATION_TARGET)
    {
        SceneObject *rootObj = this->Manager->GetObject(PREOP_ROOT_OBJECT_NAME);
        if (rootObj)
            newPoint->SetTransform(rootObj->GetTransform());
        if(m_imageObject && m_imageData)
        {
            newPoint->SetInput(m_imageData);
            if (!this->Manager->GetRegistrationFlag())
                newPoint->SetTransform(m_imageObject->GetCurrentTransform());
            newPoint->PlaceWidget();
        }
        else
            newPoint->PlaceWidget(coords[0]-m_pointRadius-2, coords[0]+m_pointRadius+2, coords[1]-m_pointRadius-2, coords[1]+m_pointRadius+2, coords[2]-m_pointRadius-2, coords[2]+m_pointRadius+2);
    }
    else
        newPoint->PlaceWidget(coords[0]-m_pointRadius-2, coords[0]+m_pointRadius+2, coords[1]-m_pointRadius-2, coords[1]+m_pointRadius+2, coords[2]-m_pointRadius-2, coords[2]+m_pointRadius+2);
    newPoint->SetInteractor(m_interactor);
    newPoint->SeNumericLabel(numericLabel);
    newPoint->SetPointIndex(index);
    newPoint->SetEnabled(1);
    newPoint->SetPickable(0);
    newPoint->ShowLabel(m_showLabel);
    newPoint->SetRadius(m_pointRadius);
    newPoint->SetLabelScale(m_labelSize);
    newPoint->SetHotSpotSize(m_pointRadius+0.25*m_pointRadius);
    newPoint->SetPosition(coords[0], coords[1], coords[2]);
    newPoint->SetOpacity(m_opacity);
/*    if (m_points_type == REGISTRATION_TARGET)
        newPoint->SetOpacity(0.5);*/
    newPoint->SetPropertyColor(m_enabledColor);
    newPoint->SetDisabledPropertyColor(m_disabledColor);
    newPoint->SetSelectedPropertyColor(m_selectedColor);
    newPoint->Update();
    m_pointList->InsertNextPoint(newPoint);
    int num = m_pointList->GetNumberOfPoints();
    if (m_points_type != REGISTRATION_TARGET)
        m_pointList->HighlightPoint(num-1, 1);
    newPoint->Delete();
    emit PointsChanged();
    emit PointsUpdated();
    emit Modified();
}

void PointsObject::AddPoint(vtkSphereWithTextPointWidget *point)
{
    m_pointList->InsertNextPoint(point);
    int num = m_pointList->GetNumberOfPoints();
    if (m_points_type != REGISTRATION_TARGET)
        m_pointList->HighlightPoint(num-1, 1);
    emit PointsChanged();
    emit Modified();
}

void PointsObject::RemovePoint(int index)
{
    m_pointList->RemovePoint(index);
    emit PointRemoved();
    emit Modified();
}

void PointsObject::RemoveAllPoints()
{
    m_pointList->EnableAllPointsOff();
    m_pointList->Reset();
    emit PointsChanged();
    emit Modified();
}

void PointsObject::SetPointPickable(int idx, int yes)
{
    vtkSphereWithTextPointWidget *point = m_pointList->GetPoint(idx);
    point->SetPickable(yes);
    emit Modified();
}

void PointsObject::EnablePickingOnAllPoints(int yes)
{
    m_pointList->EnablePickingOnAllPoints(yes);
    emit Modified();
}

void PointsObject::SetPointCoordinatesAndUpdate(int idx, vtkReal *pt)
{
    if (pt)
    {
        vtkSphereWithTextPointWidget *point = m_pointList->GetPoint(idx);
        point->SetPosition(pt);
        point->UpdateLabelPosition();
        point->Update();
        if (m_points_type != REGISTRATION_TARGET)
            this->HighlightPoint(idx, 1);
        emit PointsChanged();
        emit PointsUpdated();
        emit Modified();
    }
}

void PointsObject::HighlightPoint(int index, int on)
{
    int hilighted =  m_pointList->GetHighlightedPointIndex();
    if (hilighted != index)
    {
        if (on)
            m_pointList->HighlightPoint(hilighted,0);
        m_pointList->HighlightPoint(index, on);
        if (m_enableRefining)
        {
            vtkReal pos[3];
            m_pointList->GetPointCoordinates(index, pos);
            this->SetImagePlanesPosition(index, pos[0], pos[1], pos[2]);
        }
        emit Modified();
    }
}

int PointsObject::GetHighlightedPointIndex()
{
    return m_pointList->GetHighlightedPointIndex();
}

void PointsObject::CopyPointList(vtkListOfSphereWithTextPointWidgets *sourceList)
{
    if (sourceList)
        m_pointList->DeepCopy(sourceList);
}

void PointsObject::UpdatePoints()
{
    int i, n = m_pointList->GetNumberOfPoints();
    vtkSphereWithTextPointWidget *point;
    for (i = 0; i < n; i++)
    {
        point = m_pointList->GetPoint(i);
        point->SetRadius(m_pointRadius);
        point->SetLabelScale(m_labelSize);
        point->ShowLabel((m_showLabel>0)?true:false);
        point->SetHotSpotSize(m_pointRadius+0.25*m_pointRadius);
        point->Update();
    }        
    emit Modified();
}

void PointsObject::HideShow()
{
    if(m_pointList && this->ObjectHidden)
    {
        m_pointList->EnableAllPointsOff();
    }
    else
    {
        m_pointList->EnableAllPointsOn();
    }
}

void PointsObject::EnablePickRefiner(int enable)
{
    m_enableRefining = false;
    if (m_imageObject)
    {
        int i, n = m_pointList->GetNumberOfPoints();   
        if (enable)
        {
            m_enableRefining = true;
            m_pointList->EnableInteraction(1);
            for (i = 0; i < n; i++)
            {
                vtkSphereWithTextPointWidget *point =  m_pointList->GetPoint(i);
                if (point->GetPickable())
                {
                    point->AddObserver(vtkCommand::UserEvent+IGNS_MOVE_3D_POINT, m_movePointCommand, 1.0);
                    point->AddObserver(vtkCommand::UserEvent+IGNS_HIGHLIGHT_POINT, m_movePointCommand, 1.0);
                }
            }
            if (!m_pickRefiner)
                m_pickRefiner = PickRefiner::New();
            m_pickRefiner->SetPoints(this);
            m_pickRefiner->SetImageObject(m_imageObject);
            m_pickRefiner->EnablePickerOnPlanes();
            m_pickRefiner->ShowCrosshair();
            if (n > 0)
            {
                int idx = m_pointList->GetHighlightedPointIndex();
                if (idx < 0)
                    idx = 0;
                vtkReal pos[3];
                m_pointList->GetPointCoordinates(idx, pos);
                this->SetImagePlanesPosition(idx, pos[0], pos[1], pos[2]);
            }
        }
        else
        {           
            m_pointList->EnableInteraction(0);
            for (i = 0; i < n; i++)
            {
                vtkSphereWithTextPointWidget *point =  m_pointList->GetPoint(i);
                point->RemoveObserver(vtkCommand::UserEvent+IGNS_MOVE_3D_POINT); 
                point->RemoveObserver(vtkCommand::UserEvent+IGNS_HIGHLIGHT_POINT); 
            }
            m_pickRefiner->HideCrosshair();
            m_pickRefiner->DisablePickerOnPlanes();
            m_pickRefiner->Delete();
            m_pickRefiner = 0;
        }
    }
}

void PointsObject::ShowCrosshair(int show)
{
    if (m_imageObject && m_pickRefiner)
    {
        if (show)
            m_pickRefiner->ShowCrosshair();
        else            
            m_pickRefiner->HideCrosshair();
    }        
}

void PointsObject::SetImagePlanesPosition(int idx, vtkReal posX, vtkReal posY, vtkReal posZ)
{
    if (m_imageObject && m_pickRefiner)
        m_pickRefiner->SetPlanesPosition(idx, posX, posY, posZ);
}

void PointsObject::OverrideMouseActions()
{
    if (m_imageObject && m_pickRefiner)
        m_pickRefiner->OverrideMouseActions();
}

void PointsObject::RestoreMouseActions()
{
    if (m_imageObject && m_pickRefiner)
        m_pickRefiner->RestoreMouseActions();
}

void PointsObject::MovePoint(POINT_PROPERTIES* point)
{
    if (m_imageObject && m_pickRefiner)
    {
        vtkReal *pt = &point->x;
        vtkSphereWithTextPointWidget *currentPoint = m_pointList->GetPoint(point->index);
        currentPoint->SetPosition(pt);
        m_pickRefiner->SetPlanesPosition(point->index, point->x, point->y, point->z);
        m_pointList->HighlightPoint(point->index, 1);
        this->UpdatePoints();
        emit PointsChanged();
        emit PointsUpdated();
        emit Modified();
    }
}

void PointsObject::ActivateCursorOnAllPlanes(int active)
{
    if (m_imageObject)
    {
        for (int i = 0; i < 3; i++)
        {
            m_imageObject->GetPlane(i)->ExternalActivateCursor(active);
            this->Manager->MoveCamera(m_imageObject, i);
        }
    }
}

void PointsObject::ShowPointInAllViews(int index)
{
    if(!m_inCorrectionMode)
    {
        if (m_imageObject)
        {
            vtkReal xyz[3];
            vtkReal *pos;
            m_pointList->GetPointCoordinates(index, xyz);
            View * v;
            int i;
            for (i = 0; i < 3; i++)
            {
                m_imageObject->GetPlane(i)->SetSlicePosition(xyz[i]);
            }
            if (this->Manager->GetRegistrationFlag())
            {
                vtkTransform *t = m_imageObject->GetCurrentTransform();
                pos = t->TransformDoublePoint(xyz[0], xyz[1], xyz[2]);
                xyz[0] = pos[0];
                xyz[1] = pos[1];
                xyz[2] = pos[2];
            }
            for (i = 0; i < 3; i++)
            {
                v = this->Manager->GetView(i);
                vtkCoordinate* coord = vtkCoordinate::New();
                coord->SetCoordinateSystemToWorld();
                coord->SetValue(xyz[0], xyz[1], xyz[2]);
                int *dispPos = coord->GetComputedDisplayValue(v->GetRenderer());
                m_imageObject->GetPlane(i)->ExternalUpdateCursor(dispPos, v->GetRenderer());
                this->Manager->MoveCamera(m_imageObject, i);
                v->GetRenderer()->ResetCameraClippingRange();
                v->NotifyNeedRender();
                m_imageObject->GetPlane(i)->ExternalUpdateCursor(dispPos, v->GetRenderer()); //TODO this is a patch which forces showing cursor
            }
            if (m_points_type != REGISTRATION_TARGET)
                m_pointList->HighlightPoint(index, 1);
        }
        else if (m_points_type != REGISTRATION_TARGET)
            this->HighlightPoint(index, 1);
    }
    else if (m_points_type != REGISTRATION_TARGET)
        this->HighlightPoint(index, 1);
    emit Modified();
}

void PointsObject::SetCorrectionMode(int mode)
{
    if (m_inCorrectionMode == mode)
        return;
    if (m_inCorrectionMode > 0)
    {
        if (m_imageObject)
            m_imageObject->EnableSavingMousePosition();
        this->EnablePickingOnAllPoints(0);
        this->EnablePickRefiner(0);
    }
    else
    {
        if (m_imageObject)
            m_imageObject->DisableSavingMousePosition();
        this->EnablePickingOnAllPoints(1);
        this->EnablePickRefiner(1);
    }
    m_inCorrectionMode = mode;
    emit Modified();
}

void PointsObject::SetEnabledColor(vtkReal color[3])
{
    int i, n = m_pointList->GetNumberOfPoints();
    for (i = 0; i < 3; i++)
        m_enabledColor[i] = color[i];
    for (i = 0; i < n; i++)
    {
        vtkSphereWithTextPointWidget *point =  m_pointList->GetPoint(i);
        point->SetPropertyColor(m_enabledColor);
        point->Update();
    }
    emit Modified();
}

void PointsObject::GetEnabledColor(vtkReal color[3])
{
    for (int i = 0; i < 3; i++)
        color[i] = m_enabledColor[i];
}

void PointsObject::SetDisabledColor(vtkReal color[3])
{
    int i, n = m_pointList->GetNumberOfPoints();
    for (i = 0; i < 3; i++)
        m_disabledColor[i] = color[i];
    for (i = 0; i < n; i++)
    {
        vtkSphereWithTextPointWidget *point =  m_pointList->GetPoint(i);
        point->SetDisabledPropertyColor(m_disabledColor);
        point->Update();
    }
    emit Modified();
}

void PointsObject::GetDisabledColor(vtkReal color[3])
{
    for (int i = 0; i < 3; i++)
        color[i] = m_disabledColor[i];
}

void PointsObject::SetSelectedColor(vtkReal color[3])
{
    int i, n = m_pointList->GetNumberOfPoints();
    for (i = 0; i < 3; i++)
        m_selectedColor[i] = color[i];
    for (i = 0; i < n; i++)
    {
        vtkSphereWithTextPointWidget *point =  m_pointList->GetPoint(i);
        point->SetSelectedPropertyColor(m_selectedColor);
        point->Update();
    }
    emit Modified();
}

void PointsObject::GetSelectedColor(vtkReal color[3])
{
    for (int i = 0; i < 3; i++)
        color[i] = m_selectedColor[i];
}

void PointsObject::SetOpacity(vtkReal opacity)
{
    m_opacity = opacity;
    int i, n = m_pointList->GetNumberOfPoints();
    for (i = 0; i < n; i++)
    {
        vtkSphereWithTextPointWidget *point =  m_pointList->GetPoint(i);
        point->SetOpacity(m_opacity);
        point->Update();
    }
    emit Modified();
}

void PointsObject::TransformationChanged(QString transformName, int index)
{
    this->TransformName = transformName;
    vtkTransform *newTransform = this->Manager->GetUserTransforms()->GetUserTransform(transformName);
    int type = this->Manager->GetUserTransforms()->GetTransformType(transformName);
    if (this->ObjectType & INTRAOP_TYPE)
    {
        if (type == ABSOLUTE_TRANSFORM)
        {
            m_pointList->SetUserTransform(newTransform);
            this->AppliedTransforms.clear();
        }
        else
        {
            vtkTransform *tmpTrans = vtkTransform::New();
            tmpTrans->Identity();
            vtkTransform * tmp1Trans = m_pointList->GetUserTransform();
            if (tmp1Trans)
                tmpTrans->SetInput(tmp1Trans);
            tmpTrans->Concatenate(newTransform->GetMatrix());
            m_pointList->SetUserTransform(tmpTrans);
            tmpTrans->Delete();
        }
        this->AppliedTransforms.push_back(this->Manager->GetUserTransforms()->GetTransformIndex(this->TransformName));
        if (!this->Manager->GetUserTransforms()->TransformIsPredefined(transformName))
        {
            this->Manager->GetUserTransforms()->IncrementReferenceCounter(transformName);
        }
        emit TransformSet();
        emit Modified();
        emit RestoreSettingsWidget(index);
    }
}

void PointsObject::UndoLastTransformation(int index)
{
    if (AppliedTransforms.size() > 1 && this->ObjectType & INTRAOP_TYPE)
    {
        QString transformName;
        int lastTransformIndex = this->AppliedTransforms.back();
        transformName = this->Manager->GetUserTransforms()->GetTransformName(lastTransformIndex);
        vtkMatrix4x4 *lastTransformMatrix = this->Manager->GetUserTransforms()->GetTransformMatrix(lastTransformIndex);
        vtkMatrix4x4 *invertedLastTransformMatrix = vtkMatrix4x4::New();
        invertedLastTransformMatrix->Identity();
        vtkMatrix4x4::Invert(lastTransformMatrix, invertedLastTransformMatrix);
        vtkTransform *tmpTrans = vtkTransform::New();
        tmpTrans->Identity();
        vtkTransform * tmp1Trans = m_pointList->GetUserTransform();
        if (tmp1Trans)
            tmpTrans->SetInput(tmp1Trans);
        tmpTrans->Concatenate(invertedLastTransformMatrix);
        m_pointList->SetUserTransform(tmpTrans);
        tmpTrans->Delete();
        this->AppliedTransforms.pop_back();
        this->SetTransformName(this->Manager->GetUserTransforms()->GetTransformName(this->AppliedTransforms.back()));
        if (!this->Manager->GetUserTransforms()->TransformIsPredefined(transformName))
        {
            this->Manager->GetUserTransforms()->DecrementReferenceCounter(transformName);
        }
        emit TransformSet();
        emit Modified();
        emit RestoreSettingsWidget(index);
    }
}

void PointsObject::WriteTagFile(const QString & filename, const char * timeStamp)
{
    vtkPoints *pts = vtkPoints::New();
    vtkReal pt[3];
    int i, n = m_pointList->GetNumberOfPoints();
    std::vector<std::string> pointNames;
    for (i = 0; i < n; i++)
    {
        m_pointList->GetPointCoordinates(i, pt);
        pts->InsertNextPoint(pt);
        pointNames.push_back( std::string(m_pointList->GetPointLabel(i)));
    }
    vtkTagWriter * writer = vtkTagWriter::New();
    writer->SetFileName( filename );
    writer->SetPointNames( pointNames );
    writer->AddVolume(pts, this->Name);
    if (timeStamp)
        writer->SetTimeStamp(timeStamp);
    if (m_imageObject)
    {
        writer->SetReferenceDataFile(m_imageObject->GetDataFileName());
        vtkTransform *t = m_imageObject->GetCurrentTransform();
        if (t)
        {
            vtkMatrix4x4 *m = t->GetMatrix();
            QString tr = "";
            double elem;
            for (int i = 0; i < 3; i++)
                for(int j = 0; j < 4; j++)
                {
                    elem = m->GetElement(i,j);
                    tr.append(QString::number(elem));
                    tr.append(" ");
                }
            writer->SetTransformToSave(tr.ascii());
        }
    }
    writer->Write();
    writer->Delete();
    pts->Delete();
}

void PointsObject::UpdateSettingsWidget()
{
    emit PointsChanged();
}

QWidget * PointsObject::CreateSettingsDialog( QWidget * parent )
{ 
    PointsObjectSettingsDialog * res = new PointsObjectSettingsDialog( parent, "PointsObjectSettingsDialog", Qt::WDestructiveClose );
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    res->SetLogger(this->GetLogger());
#endif        
    res->SetPointsObject(this);
    if (this->ObjectType & INTRAOP_TYPE)
        res->SetAppliedTransforms(this->AppliedTransforms);
    connect( res, SIGNAL( TransformationChanged(QString, int) ), this, SLOT(TransformationChanged(QString, int)) );
    connect( this, SIGNAL( RestoreSettingsWidget(int) ), res, SLOT(ShowLastUsedWidget(int)) );
    connect( res, SIGNAL( UndoTransformation(int) ), this, SLOT(UndoLastTransformation(int)) );
    return res;
}


/*=========================================================================

  Program:   IGNS Visualization Toolkit
  Module:    $RCSfile: vtkListOfSphereWithTextPointWidgets.cxx,v $

  Copyright (c) Anka, IPL, BIC, MNI, McGill 
  All rights reserved.

=========================================================================*/
#include "vtkListOfSphereWithTextPointWidgets.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkListOfSphereWithTextPointWidgets, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkListOfSphereWithTextPointWidgets);

vtkListOfSphereWithTextPointWidgets::vtkListOfSphereWithTextPointWidgets()
{
}

vtkListOfSphereWithTextPointWidgets::~vtkListOfSphereWithTextPointWidgets()
{
    this->Reset();
}

void vtkListOfSphereWithTextPointWidgets::Reset( )
{
    SpherePointsVec::iterator it = this->SpherePointList.begin();
    for( ; it != this->SpherePointList.end(); ++it )
    {
        if ((*it))
        {
            (*it)->SetEnabled(0);
            (*it)->Delete();
            (*it) = 0;
        }
    }
    this->SpherePointList.clear();
}

void vtkListOfSphereWithTextPointWidgets::Update()
{
    SpherePointsVec::iterator it = this->SpherePointList.begin();
    for( ; it != this->SpherePointList.end(); ++it )
    {
        if ((*it))
        {
            (*it)->Update();
        }
    }
}

vtkSphereWithTextPointWidget *vtkListOfSphereWithTextPointWidgets::GetPoint( unsigned int index )
{
    if (index < this->SpherePointList.size())
        return this->SpherePointList[index];
    return 0;
}

void vtkListOfSphereWithTextPointWidgets::SetPoint( int index, vtkSphereWithTextPointWidget * pt )
{
    vtkSphereWithTextPointWidget *old_point = this->GetPoint(index);
    old_point->SetPosition(pt->GetPosition());
    old_point->Update();
}

void vtkListOfSphereWithTextPointWidgets::InsertPoint( int index, vtkSphereWithTextPointWidget * pt ) 
{
    vtkSphereWithTextPointWidget *tmp;
    int n = this->SpherePointList.size();
    pt->Register(this);
    SpherePointsVec::iterator it = this->SpherePointList.begin() + index;
    this->SpherePointList.insert(it, pt);
    for (int i = index; i < n-1; i++)
    {
        tmp = this->GetPoint( i + 1 );
        tmp->SeNumericLabel(i);
    }
}

void vtkListOfSphereWithTextPointWidgets::InsertNextPoint( vtkSphereWithTextPointWidget * pt )
{
    pt->Register(this);
    this->SpherePointList.push_back(pt);
}

void vtkListOfSphereWithTextPointWidgets::RemovePoint(int index)
{
    int n = this->SpherePointList.size();
    if (index >= 0 && index < n)
    {
        vtkSphereWithTextPointWidget *pt = this->GetPoint( index );
        pt->SetEnabled(0);
        SpherePointsVec::iterator it = this->SpherePointList.begin() + index;
        this->SpherePointList.erase(it);
        pt->Delete();
        it = this->SpherePointList.begin();
        for(int i = 0; it != this->SpherePointList.end(); ++it, i++ )
        {
            (*it)->SetPointIndex(i);
        }
    }
}

void  vtkListOfSphereWithTextPointWidgets::SeNumericLabels()
{
    SpherePointsVec::iterator it = this->SpherePointList.begin();
    for(int i = 0; it != this->SpherePointList.end(); ++it, i++ )
    {
        (*it)->SeNumericLabel(i+1);
    }
}

void vtkListOfSphereWithTextPointWidgets::DisableEnablePoint( int index, bool enable )
{
    int n = this->SpherePointList.size();
    if (index >= 0 && index < n)
    {
        vtkSphereWithTextPointWidget *pt = this->GetPoint( index );
        pt->SetEnabled(enable?1:0);
    }
}

void vtkListOfSphereWithTextPointWidgets::DeepCopy(vtkListOfSphereWithTextPointWidgets *list)
{
    int i, n = list->GetNumberOfPoints();
    vtkSphereWithTextPointWidget *newPoint;
    for( i = 0; i < n; i++ )
    {
        newPoint = vtkSphereWithTextPointWidget::New();
        newPoint->DeepCopy(list->GetPoint( i ));
        this->InsertNextPoint(newPoint);
    }
}

double *vtkListOfSphereWithTextPointWidgets::GetPointCoordinates( int index )
{
    if (index < this->GetNumberOfPoints())
    {
        vtkSphereWithTextPointWidget *old_point = this->GetPoint(index);
        return old_point->GetPosition();
    }
    return 0;
}

void vtkListOfSphereWithTextPointWidgets::GetPointCoordinates( int index, double xyz[3] )
{
    if (index < this->GetNumberOfPoints())
    {
        vtkSphereWithTextPointWidget *old_point = this->GetPoint(index);
        old_point->GetPosition(xyz);
    }
}
 
void vtkListOfSphereWithTextPointWidgets::SetPointCoordinates( int index, double xyz[3] )
{
    if (index < this->GetNumberOfPoints())
    {
        vtkSphereWithTextPointWidget *old_point = this->GetPoint(index);
        old_point->SetPosition(xyz);
        old_point->Update();
    }
}

void vtkListOfSphereWithTextPointWidgets::SetPointCoordinates( int index, double x, double y, double z )
{
    if (index < this->GetNumberOfPoints())
    {
        vtkSphereWithTextPointWidget *old_point = this->GetPoint(index);
        old_point->SetPosition(x, y, z);
        old_point->Update();
    }
}

const char* vtkListOfSphereWithTextPointWidgets::GetPointLabel(int index)
{
    if (index < this->GetNumberOfPoints())
    {
        vtkSphereWithTextPointWidget *old_point = this->GetPoint(index);
        return old_point->GetLabel();
    }
    return 0;
}

void  vtkListOfSphereWithTextPointWidgets::SetPointLabel(int index, const char* label)
{
    if (index < this->GetNumberOfPoints())
    {
        vtkSphereWithTextPointWidget *old_point = this->GetPoint(index);
        old_point->SetLabel(label);
    }
}

int vtkListOfSphereWithTextPointWidgets::GetHighlightedPointIndex()
{
    int i, n = this->GetNumberOfPoints();
    for (i = 0; i <n; i++)
    {
        vtkSphereWithTextPointWidget *point = this->GetPoint(i);
        if (point->GetHighlightedFromOutside() == 1)
            return i;
    }
    return -1;
}

void vtkListOfSphereWithTextPointWidgets::SetPointOpacity(int index, double op)
{
    vtkSphereWithTextPointWidget *point = this->GetPoint(index);
    if (point)
        point->SetOpacity(op);
}
void vtkListOfSphereWithTextPointWidgets::SetAllPointsOpacity(double op)
{
    SpherePointsVec::iterator it = this->SpherePointList.begin();
    for( ; it != this->SpherePointList.end(); ++it )
    {
        if ((*it))
        {
            (*it)->SetOpacity(op);
        }
    }
}

void vtkListOfSphereWithTextPointWidgets::HighlightPoint(int index, int highlightSetting)
{
    int n = this->GetHighlightedPointIndex();
    if (highlightSetting && n == index || index < 0 )
        return;
    if (n >= 0 && highlightSetting)
    {
        vtkSphereWithTextPointWidget *point = this->GetPoint(n);
        point->ExternalHighlight(0);
    }
    vtkSphereWithTextPointWidget *point1 = this->GetPoint(index);
    if (point1)
        point1->ExternalHighlight(highlightSetting);
}

void vtkListOfSphereWithTextPointWidgets::EnableInteraction(int yes)
{
    SpherePointsVec::iterator it = this->SpherePointList.begin();
    for( ; it != this->SpherePointList.end(); ++it )
    {
        if ((*it))
        {
            (*it)->EnableInteraction(yes);
        }
    }
}

void vtkListOfSphereWithTextPointWidgets::EnableAllPoints(int yes)
{
    if (yes)
        EnableAllPointsOn();
    else
        EnableAllPointsOff();
}

void vtkListOfSphereWithTextPointWidgets::EnableAllPointsOn()
{
    int i, n = this->GetNumberOfPoints();
    for (i = 0; i <n; i++)
    {
        EnablePointOn(i);
    }   
}

void vtkListOfSphereWithTextPointWidgets::EnableAllPointsOff()
{
    int i, n = this->GetNumberOfPoints();
    for (i = 0; i <n; i++)
    {
        EnablePointOff(i);
    }   
}

void vtkListOfSphereWithTextPointWidgets::EnablePickingOnAllPointsOn()
{
    int i, n = this->GetNumberOfPoints();
    for (i = 0; i <n; i++)
    {
        vtkSphereWithTextPointWidget *point = this->GetPoint(i);
        point->SetPickable(1);
    }
}

void vtkListOfSphereWithTextPointWidgets::EnablePickingOnAllPointsOff()
{
    int i, n = this->GetNumberOfPoints();
    for (i = 0; i <n; i++)
    {
        vtkSphereWithTextPointWidget *point = this->GetPoint(i);
        point->SetPickable(0);
    }
}

void vtkListOfSphereWithTextPointWidgets::EnablePickingOnAllPoints(int yes)
{
    int i, n = this->GetNumberOfPoints();
    for (i = 0; i <n; i++)
    {
        vtkSphereWithTextPointWidget *point = this->GetPoint(i);
        point->SetPickable(yes);
    }
}

void vtkListOfSphereWithTextPointWidgets::EnablePoint(int index, int yes)
{
    if (yes)
        EnablePointOn(index);
    else
        EnablePointOff(index);
}

void vtkListOfSphereWithTextPointWidgets::EnablePointOn(int index)
{
    vtkSphereWithTextPointWidget *point = this->GetPoint(index);
    if (point)
    {
        point->SetEnabled(1);
        point->Update();
    }
    else
        vtkErrorMacro(<< "No Point!!!\n");
}

void vtkListOfSphereWithTextPointWidgets::EnablePointOff(int index)
{
    vtkSphereWithTextPointWidget *point = this->GetPoint(index);
    if (point)
        point->SetEnabled(0);
    else
        vtkErrorMacro(<< "No Point!!!\n");
}

void vtkListOfSphereWithTextPointWidgets::SetUserTransform( vtkTransform * tr )
{
    int i, n = this->GetNumberOfPoints();
    for (i = 0; i <n; i++)
    {
        vtkSphereWithTextPointWidget *point = this->GetPoint(i);
        point->SetTransform(tr);
        point->UpdateLabelPosition();
    }
}

vtkTransform * vtkListOfSphereWithTextPointWidgets::GetUserTransform()
{
    int n = this->GetNumberOfPoints();
    if (n > 0)
    {
        vtkSphereWithTextPointWidget *point = this->GetPoint(0);
        return point->GetTransform();
    }
    return 0;
}

void vtkListOfSphereWithTextPointWidgets::PrintSelf(ostream& os, vtkIndent indent)
{
    int i, n = this->GetNumberOfPoints();
    os << indent << "Number of Points = " << n << endl;
    for (i = 0; i <n; i++)
    {
        vtkSphereWithTextPointWidget *point = this->GetPoint(i);
        if (!point)
            os << indent << "Problem - no point!!!!\n";
        else
        {
            os << indent << "point : " << point << endl;
            point->PrintSelf(os,indent.GetNextIndent());
        }
    }   
}



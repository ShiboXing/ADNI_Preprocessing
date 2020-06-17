#include <iostream>

#include "movepointcommand.h"
#include "pointsobject.h"
#include "vtkSphereWithTextPointWidget.h"
#include "vtkIgnsEvents.h"


MovePointCommand::MovePointCommand()
{
    m_points = 0;
}

MovePointCommand::~MovePointCommand()
{
    if (m_points)
        m_points->UnRegister(this);
    m_points = 0;
}

void MovePointCommand::SetPointsObject(PointsObject *pts)
{
    if (m_points == pts)
        return;
    if (m_points)
        m_points->UnRegister(this);
    m_points = pts;
    if (m_points)
        m_points->Register(this);
}

void MovePointCommand::Execute(vtkObject *caller, unsigned long event, void *callData)
{
    if (m_points)
    {
        vtkSphereWithTextPointWidget *widget = reinterpret_cast<vtkSphereWithTextPointWidget*>(caller);
        vtkReal pos[3];
        widget->GetPosition(pos);
        POINT_PROPERTIES pt;
        pt.index = widget->GetPointIndex();
        pt.x = pos[0];
        pt.y = pos[1];
        pt.z = pos[2];
        if (event == vtkCommand::UserEvent+IGNS_MOVE_3D_POINT)
            m_points->MovePoint(&pt);
        else
            m_points->HighlightPoint(pt.index, 1);
    }
}


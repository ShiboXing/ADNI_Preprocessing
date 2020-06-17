#include "polydataobject.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "view.h"
#include "vtkReal.h"
#include "vtkTransform.h"
#include "vtkAssembly.h"
#include "vtkProperty.h"
#include "polydataobjectsettingsdialog.h"
#include "scenemanager.h"
#include "usertransformations.h"
#include "vtkTubeFilter.h"

vtkCxxSetObjectMacro(PolyDataObject, PolyData, vtkPolyData);
vtkCxxSetObjectMacro(PolyDataObject, Property, vtkProperty);
ObjectSerializationMacro( PolyDataObject );

PolyDataObject::PolyDataObject()
{
    this->PolyData = 0;
    this->renderingMode = VTK_SURFACE;
    this->ScalarsVisible = 1;
    this->opacity = 1.0;
    this->Property = vtkProperty::New();
    this->AppliedTransforms.clear();
    this->DTITracks = 0;
    this->DTITubeFilter = 0;
    this->DTITubeRadius = 1.0;
    this->DTITubeResolution = 6;
    this->OriginalData = 0;
    for (int i = 0; i< 3; i++)
       objectColor[i] = 1.0;
}

PolyDataObject::~PolyDataObject()
{
    if( this->PolyData )
    {
        this->PolyData->UnRegister( this );
    }
    this->Property->Delete();
    if (this->DTITubeFilter)
        this->DTITubeFilter->Delete();
    if (this->OriginalData)
        this->OriginalData->Delete();        
}

void PolyDataObject::Serialize( Serializer * ser )
{
    SceneObject::Serialize(ser);
    ::Serialize( ser, "RenderingMode", this->renderingMode );
    ::Serialize( ser, "ScalarsVisible", this->ScalarsVisible );
    ::Serialize( ser, "Opacity", this->opacity );
    ::Serialize( ser, "ObjectColor", this->objectColor, 3 );
    ::Serialize( ser, "DTITracks", this->DTITracks );
    ::Serialize( ser, "DTITubeRadius", this->DTITubeRadius );
    ::Serialize( ser, "DTITubeResolution", this->DTITubeResolution );
}


void PolyDataObject::Setup( View * view, vtkAssembly * placementHint ) 
{
    if( view->GetType() == THREED_VIEW_TYPE )
    {
        vtkPolyDataMapper * mapper = vtkPolyDataMapper::New();
        mapper->SetInput( this->PolyData );
        mapper->SetScalarVisibility( this->ScalarsVisible );
        vtkActor * actor = vtkActor::New();
        actor->SetMapper( mapper );
        actor->SetProperty( this->Property );
        if( this->Transform )
        {
            actor->SetUserTransform( this->Transform );
        }
        mapper->Delete();
        actor->PickableOff();
        if( placementHint )
            placementHint->AddPart( actor );
        else
            view->GetRenderer()->AddActor( actor );
        this->polydataObjectInstances[ view ] = actor;
        
        connect( this, SIGNAL( Modified() ), view, SLOT( NotifyNeedRender() ) );
        this->GetProperty()->GetColor(this->objectColor);
    }
}

void PolyDataObject::GetColor(double color[3], int fromProperty)
{
    if (fromProperty)
        this->GetProperty()->GetColor(this->objectColor);
    for (int i = 0; i < 3; i++)
        color[i] = this->objectColor[i];
}

void PolyDataObject::SetColor(double color[3])
{
    for (int i = 0; i < 3; i++)
        this->objectColor[i] = color[i];
    this->GetProperty()->SetColor(this->objectColor);
}

void PolyDataObject::PreDisplaySetup()
{
}

void PolyDataObject::Release( View * view ) 
{
    if( view->GetType() == THREED_VIEW_TYPE )
    {
        this->disconnect( view );
        
        PolyDataObjectViewAssociation::iterator itAssociations = this->polydataObjectInstances.find( view );
    
        if( itAssociations != this->polydataObjectInstances.end() )
        {
            vtkActor * actor = (*itAssociations).second;
        
            // delete the vtkAssembly we have
            this->RemoveFromParent( actor, view );
            actor->Delete();
            this->polydataObjectInstances.erase( itAssociations );
        }
    }
}

QWidget * PolyDataObject::CreateSettingsDialog( QWidget * parent )
{
    PolyDataObjectSettingsDialog * res = new PolyDataObjectSettingsDialog( parent, "Object settings dialog", Qt::WDestructiveClose );
    res->SetPolyDataObject( this );
    if (this->ObjectType & INTRAOP_TYPE)
        res->SetAppliedTransforms(this->AppliedTransforms);
    connect( res, SIGNAL( TransformationChanged(QString, int) ), this, SLOT(TransformationChanged(QString, int)) );
    connect( res, SIGNAL( UndoTransformation(int) ), this, SLOT(UndoLastTransformation(int)) );
    connect( this, SIGNAL( RestoreSettingsWidget(int) ), res, SLOT(ShowLastUsedWidget(int)) );
    connect( this, SIGNAL( ObjectViewChanged() ), res, SLOT(UpdateSettings()) );
    return res;
}

void PolyDataObject::SetRenderingMode( int renderingMode )
{
    if( renderingMode != this->renderingMode )
    {
        this->renderingMode = renderingMode;
        this->Property->SetRepresentation( this->renderingMode );
        emit Modified();
    }
}

void PolyDataObject::SetScalarsVisible( int use )
{
    if( use != this->ScalarsVisible )
    {
        this->ScalarsVisible = use;
        
        PolyDataObjectViewAssociation::iterator it = this->polydataObjectInstances.begin();
        for( ; it != this->polydataObjectInstances.end(); ++it )
        {
            (*it).second->GetMapper()->SetScalarVisibility( use );
        }
        
        emit Modified();
    }
}

void PolyDataObject::SetOpacity( double opacity )
{
    if( opacity != this->opacity )
    {
        if( opacity > 1 )
            this->opacity = 1;
        else if( opacity < 0 )
            this->opacity = 0;
        else 
            this->opacity = opacity;
        
        this->Property->SetOpacity( this->opacity );
        
        emit Modified();
    }
}

vtkActor *PolyDataObject::Get3DViewActor()
{
    View * v = 0;
    vtkActor *a = 0;
    PolyDataObjectViewAssociation::iterator it = this->polydataObjectInstances.begin();
    for( ; it != this->polydataObjectInstances.end(); ++it )
    {
        v = (*it).first;
        if (v->GetType() == THREED_VIEW_TYPE)
            a = (*it).second;
    }
    return a;
}

void PolyDataObject::HideShow()
{
    View * view = 0;
    vtkActor *actor = 0;
    PolyDataObjectViewAssociation::iterator it = this->polydataObjectInstances.begin();
    for( ; it != this->polydataObjectInstances.end(); ++it )
    {
        view = (*it).first;
        if (view->GetType() == THREED_VIEW_TYPE)
            actor = (*it).second;
    }
    if (view)
    {
        vtkAssembly * placementHint = GetParentAssembly(view);
        if(ObjectHidden)
        {
            if( placementHint )
                placementHint->RemovePart( actor );
            else
                view->GetRenderer()->RemoveActor( actor );
        }
        else
        {
            if( placementHint )
                placementHint->AddPart( actor );
            else
                view->GetRenderer()->AddActor( actor );
        }
        view->NotifyNeedRender();
    }
}

void PolyDataObject::TransformationChanged(QString transformName, int index)
{
    this->TransformName = transformName;
    vtkTransform *newTransform = this->Manager->GetUserTransforms()->GetUserTransform(transformName);
    int type = this->Manager->GetUserTransforms()->GetTransformType(transformName);
    if (this->ObjectType & INTRAOP_TYPE)
    {
        if (type == ABSOLUTE_TRANSFORM)
        {
            this->Get3DViewActor()->SetUserTransform(newTransform);
            this->AppliedTransforms.clear();
        }
        else
        {
            vtkTransform *tmpTrans = vtkTransform::New();
            tmpTrans->Identity();
            tmpTrans->SetInput(this->Get3DViewActor()->GetUserTransform());
            tmpTrans->Concatenate(newTransform->GetMatrix());
            this->Get3DViewActor()->SetUserTransform(tmpTrans);
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

void PolyDataObject::UndoLastTransformation(int index)
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
        tmpTrans->SetInput(this->Get3DViewActor()->GetUserTransform());
        tmpTrans->Concatenate(invertedLastTransformMatrix);
        this->Get3DViewActor()->SetUserTransform(tmpTrans);
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

void PolyDataObject::UpdateSettingsWidget()
{
    emit ObjectViewChanged();
}

void PolyDataObject::PrepareDTITracks()
{
    if (this->PolyData && this->DTITracks)
    {
        this->OriginalData = vtkPolyData::New();
        this->OriginalData->DeepCopy(this->PolyData);
        this->DTITubeFilter = vtkTubeFilter::New();
        this->DTITubeFilter->SetInput(this->OriginalData);

        this->DTITubeFilter->SetNumberOfSides(DTITubeResolution);
        this->DTITubeFilter->SetRadius(this->DTITubeRadius);
        this->DTITubeFilter->CappingOn();
        this->SetPolyData(this->DTITubeFilter->GetOutput());
    }
}

void PolyDataObject::SetDTITubeResolution( int res )
{
    this->DTITubeResolution = res;
    if (this->DTITubeFilter && this->DTITubeFilter->GetNumberOfSides() != res)
    {
        this->DTITubeFilter->SetNumberOfSides(DTITubeResolution);
        this->DTITubeFilter->Update();
        emit Modified();
    }
}

void PolyDataObject::SetDTITubeRadius( double radius)
{
    this->DTITubeRadius = radius;
    if (this->DTITubeFilter && this->DTITubeFilter->GetNumberOfSides() != radius)
    {
        this->DTITubeFilter->SetRadius(this->DTITubeRadius);
        this->DTITubeFilter->Update();
        emit Modified();
    }
}

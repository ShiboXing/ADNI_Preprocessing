#include "sceneobject.h"
#include "view.h"
#include "vtkLandmarkTransform.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"
#include "vtkAssembly.h"
#include "vtkRenderer.h"
#include "scenemanager.h"
#include "usertransformations.h"
#include "transformlistdialog.h"
#include "vtkmatrix4x4utilities.h"

vtkCxxSetObjectMacro( SceneObject, Transform, vtkTransform );
ObjectSerializationMacro( SceneObject );

SceneObject::SceneObject() 
{ 
    this->Transform = 0;
    this->Parent = 0;
    this->ObjectType = SYSTEM_TYPE; 
    this->Manager = 0;
    this->ObjectHidden = FALSE;
    this->MincAndObj = 0;
    this->RegTransform = 0;
    this->ObjectReplacedInSynchronizedView = FALSE;
    this->NumberOfChildrenInLoadedScene = 0;
}

SceneObject::~SceneObject() 
{
    if( !this->Name.isNull() && !this->Name.isEmpty() )
    {
        this->Name.setLength(0);
        this->Name.squeeze();
    }
    if( !this->DataFileName.isNull() && !this->DataFileName.isEmpty())
    {
        this->DataFileName.setLength(0);
        this->DataFileName.squeeze();
    }
    if( !this->FullFileName.isNull() && !this->FullFileName.isEmpty())
    {
        this->FullFileName.setLength(0);
        this->FullFileName.squeeze();
    }
    if( !this->TransformName.isNull() && !this->TransformName.isEmpty())
    {
        this->TransformName.setLength(0);
        this->TransformName.squeeze();
    }
    if (this->Transform)
        this->Transform->UnRegister( this );
    if (this->RegTransform)
        this->RegTransform->UnRegister( this );
    if (this->Manager) 
        this->Manager->UnRegister(0); 
}

void SceneObject::Serialize( Serializer * ser )
{
    ::Serialize( ser, "ObjectName", this->Name );
    ::Serialize( ser, "FullFileName", this->FullFileName );
    ::Serialize( ser, "ObjectHidden", this->ObjectHidden );
    if (this->ObjectType & PREOP_ROOT_TYPE)
    {
        vtkMatrix4x4 *mat = vtkMatrix4x4::New();
        mat->Identity();
        int nChildren = this->GetNumberOfChildren();
        if( ser->IsReader())
        {
            ::Serialize( ser, "NumberOfChildren", this->NumberOfChildrenInLoadedScene );
//            ::Serialize( ser, "RegistrationTransform", mat );
        }
        else
        {
            ::Serialize( ser, "NumberOfChildren", nChildren );
/*            if (this->RegTransform)
            {
                vtkMatrix4x4 *mat1 = vtkMatrix4x4::New();
                this->RegTransform->GetMatrix(mat1);
                if (mat1)
                    ::Serialize( ser, "RegistrationTransform", mat1 );
                else                
                    ::Serialize( ser, "RegistrationTransform", mat );
                mat1->Delete();
            }*/
        }
        mat->Delete();
    }
}

void SceneObject::SetName( QString name ) 
{ 
    this->Name = name;
    emit NameChanged();
}

void SceneObject::SetDataFileName( QString name ) 
{ 
    this->DataFileName = name;
    if (this->Name.isEmpty())
    {
        this->Name = name;
        emit NameChanged();
    }
}

void SceneObject::NotifyTransformChanged()
{
    emit Modified();
}

void SceneObject::SetRegData( vtkLinearTransform * registrstionTransform )
{    
    if (this->RegTransform == registrstionTransform || registrstionTransform == 0)
        return;
    if( this->Transform )
    {
        this->Transform->UnRegister( this );
        this->Transform->Delete();
    }
    
    this->Transform = vtkTransform::New();
    this->Transform->Identity();
    // at this point the registration is not done, only pointer is saved
    if (this->RegTransform)
        this->RegTransform->UnRegister(this);
    this->RegTransform = registrstionTransform;
    this->RegTransform->Register(this);
    this->AppliedTransforms.clear();
    this->AppliedTransforms.push_back(0);
}

vtkMatrix4x4* SceneObject::GetTransformMatrix()
{
    return this->Transform->GetMatrix();
}

void SceneObject::SetTransformMatrix( vtkMatrix4x4 * matrix )
{
    this->Transform->SetMatrix(matrix);
}

void SceneObject::Setup( View * view, vtkAssembly * placementHint ) 
{
    vtkAssembly * assembly = vtkAssembly::New();
    
    if( this->Transform )
        assembly->SetUserTransform( this->Transform );
    
    if( placementHint )
        placementHint->AddPart( assembly );
    else
        view->GetRenderer()->AddViewProp( assembly );
    
    SceneObjectVec::iterator it = this->Children.begin();
    for( ; it != this->Children.end(); ++it )
    {
        (*it)->Setup( view, assembly );
    }
    
    connect( this, SIGNAL( Modified() ), view, SLOT(NotifyNeedRender()) );
    this->Instances[ view ] = assembly;
}
    
void SceneObject::Update()
{
}

void SceneObject::UpdateChildren()
{
    SceneObjectVec::iterator it = this->Children.begin();
    for( ; it != this->Children.end(); ++it )
    {
        (*it)->Update();
    }
}

void SceneObject::PreDisplaySetup() 
{
    SceneObjectVec::iterator it = this->Children.begin();
    for( ; it != this->Children.end(); ++it )
    {
        (*it)->PreDisplaySetup();
    }
}

void SceneObject::Release( View * view ) 
{
    this->disconnect( view );
    ViewAssociation::iterator itAssociations = this->Instances.find( view );
    
    if( itAssociations != this->Instances.end() )
    {
        vtkAssembly * assembly = (*itAssociations).second;
        
        // release children first
        SceneObjectVec::iterator itChildren = this->Children.begin();
        for( ; itChildren != this->Children.end(); ++itChildren )
        {
            (*itChildren)->Release( view );
        }
    
        // delete the vtkAssembly we have
        this->RemoveFromParent( assembly, view );
        assembly->Delete();
        this->Instances.erase( itAssociations );
    }
}

void SceneObject::AddChild( SceneObject * child )
{
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    igstkLogMacro( DEBUG, "AddChild called " << child->GetName() << endl);
#endif    
    // Add object to list of children
    if( child )
    {
        this->Children.push_back( child );
        child->Parent = this;
    }
    
    // if this hierarchy object is already associated with views,
    // setup the new child too
    ViewAssociation::iterator it = this->Instances.begin();
    ViewAssociation::iterator itEnd = this->Instances.end();
    for( ; it != itEnd; ++it )
    {
        child->Setup( (*it).first, (*it).second );
    }
}


int SceneObject::GetNumberOfChildren()
{
    return this->Children.size();
}


SceneObject * SceneObject::GetChild( int index )
{
    if( index < (int)this->Children.size() )
    {
        return this->Children[ index ];
    }
    return 0;
}

SceneObject * SceneObject::GetChildOfType( int type )
{
    SceneObjectVec::iterator it = this->Children.begin();
    for( ; it != this->Children.end(); ++it )
    {
        if( (*it)->GetObjectType() & type )
        {
            return (*it);
        }
    }
    return 0;
}

SceneObject * SceneObject::GetObject( QString & objectName )
{
    SceneObjectVec::iterator it = this->Children.begin();
    for( ; it != this->Children.end(); ++it )
    {
        if( !strcmp( (*it)->GetName(), objectName ) )
        {
            return (*it);
        }
        SceneObject * result = (*it)->GetObject( objectName );
        if( result )
        {
            return result;
        }
    }
    return 0;
}

void SceneObject::DetachFromParent()
{
    if( this->Parent )
    {
        SceneObjectVec::iterator it = find( this->Parent->Children.begin(), this->Parent->Children.end(), this );
        if( it != this->Parent->Children.end() )
        {
            this->Parent->Children.erase( it );
        }
    }
}

vtkAssembly * SceneObject::GetParentAssembly( View * view )
{
    if( this->Parent )
    {
        ViewAssociation::iterator itAssociations = this->Parent->Instances.find( view );
    
        if( itAssociations != this->Parent->Instances.end() )
        {
            return (*itAssociations).second;
        }
    }
    return 0;
}
    
void SceneObject::RemoveFromParent( vtkProp3D * prop, View * view )
{
    vtkAssembly * parentAssembly = this->GetParentAssembly( view );
    if( parentAssembly)
    {
        parentAssembly->RemovePart( prop );
    }
    else
    {
        view->GetRenderer()->RemoveViewProp( prop );
    }
}

void SceneObject::SetManager( SceneManager *manager )
{
    if (this->Manager != manager)                                       
    {                                                           
        if (this->Manager != NULL) 
        { 
            this->Manager->UnRegister(0); 
        }   
        this->Manager = manager;                                          
        if (this->Manager != NULL) 
        { 
            this->Manager->Register(0); 
        }     
    }                                                           
}

void SceneObject::RemoveMincAndObj()
{
    if (this->MincAndObj)
        this->MincAndObj->UnRegister(this);
    this->MincAndObj = 0;
    if (this->GetObjectIsImported())
        this->SetObjectIsImported(FALSE);
}

void SceneObject::HideShow()
{
}

QWidget * SceneObject::CreateSettingsDialog( QWidget * parent )
{ 
    if (this->ObjectType & PREOP_ROOT_TYPE) 
    {
        TransformListDialog * res = new TransformListDialog( parent, "TransformListDialog", Qt::WDestructiveClose );
        if (this->Manager)
            res->SetUserTransforms(this->Manager->GetUserTransforms());
        res->SelectTransform(this->TransformName);
        res->SetAppliedTransforms(this->AppliedTransforms);
        connect( res, SIGNAL( TransformationChanged(QString) ), this, SLOT(TransformationChanged(QString)) );
        connect( res, SIGNAL( UndoTransformation() ), this, SLOT(UndoLastTransformation()) );
        return res;
    }
    return 0;
}

void SceneObject::TransformationChanged(QString transformName)
{
    if (this->TransformName != transformName)
    {
        this->TransformName = transformName;
        vtkTransform *newTransform = this->Manager->GetUserTransforms()->GetUserTransform(transformName);
        int type = this->Manager->GetUserTransforms()->GetTransformType(transformName);
        if (this->ObjectType & PREOP_ROOT_TYPE)
        {
            vtkTransform *rootTransform = this->Transform;
            if (type == ABSOLUTE_TRANSFORM)
            {
                rootTransform->SetMatrix(newTransform->GetMatrix());
                this->AppliedTransforms.clear();
                this->AppliedTransforms.push_back(this->Manager->GetUserTransforms()->GetTransformIndex(this->TransformName));
            }
            else
            {
                rootTransform->Concatenate(newTransform->GetMatrix());
                this->AppliedTransforms.push_back(this->Manager->GetUserTransforms()->GetTransformIndex(this->TransformName));
            }
            if (this->Manager->GetViewMode() == VIEW_SYNCHRONIZED)
            {
                this->Manager->AllImagesResetPlanes();
            }
            this->Manager->SetCameraPositionInAllViews();
            if (!this->Manager->GetUserTransforms()->TransformIsPredefined(transformName))
            {
                this->Manager->GetUserTransforms()->IncrementReferenceCounter(transformName);
            }
            emit TransformSet();
            emit Modified();
        }       
    }
}

void SceneObject::UndoLastTransformation()
{
    if (AppliedTransforms.size() > 1 && this->ObjectType & PREOP_ROOT_TYPE)
    {
        QString transformName;
        int lastTransformIndex = this->AppliedTransforms.back();
        transformName = this->Manager->GetUserTransforms()->GetTransformName(lastTransformIndex);
        vtkMatrix4x4 *lastTransformMatrix = this->Manager->GetUserTransforms()->GetTransformMatrix(lastTransformIndex);
        vtkMatrix4x4 *invertedLastTransformMatrix = vtkMatrix4x4::New();
        invertedLastTransformMatrix->Identity();
        vtkMatrix4x4::Invert(lastTransformMatrix, invertedLastTransformMatrix);
        this->Transform->Concatenate(invertedLastTransformMatrix);
        invertedLastTransformMatrix->Delete();
        this->AppliedTransforms.pop_back();
        if (this->Manager->GetViewMode() == VIEW_SYNCHRONIZED)
        {
            this->Manager->AllImagesResetPlanes();
        }
        if (!this->Manager->GetUserTransforms()->TransformIsPredefined(transformName))
        {
            this->Manager->GetUserTransforms()->DecrementReferenceCounter(transformName);
        }
        emit TransformSet();
        emit Modified();
    }
}

void SceneObject::SetTransformName( QString name )
{
    this->TransformName = name;
    emit TransformSet();
}

void SceneObject::SetRegistrationTransform()
{
    //this is called only for preop_root
    if (this->ObjectType & PREOP_ROOT_TYPE)
    {
        this->SetTransformMatrix(this->RegTransform->GetMatrix());
        this->SetTransformName(IGNS_REGISTRATION);
        this->AppliedTransforms.clear();
        this->AppliedTransforms.push_back(this->Manager->GetUserTransforms()->GetTransformIndex(IGNS_REGISTRATION));
        emit TransformSet();
    }
}

std::vector<int> & SceneObject::GetAppliedTransforms()
{
    if (this->AppliedTransforms.size() == 0 && !this->TransformName.isEmpty())
        this->AppliedTransforms.push_back(this->Manager->GetUserTransforms()->GetTransformIndex(this->TransformName));
    return AppliedTransforms;    
}

void SceneObject::SetObjectIsImported( bool isImported)
{
    if (isImported)
        this->SetObjectType(this->ObjectType | IMPORTED_OBJECT_TYPE);
    else
        this->SetObjectType(this->ObjectType & !IMPORTED_OBJECT_TYPE);
}

bool SceneObject::GetObjectIsImported( )
{
    return this->ObjectType & IMPORTED_OBJECT_TYPE;
}

void SceneObject::RefreshObject()
{
    emit Modified();
}

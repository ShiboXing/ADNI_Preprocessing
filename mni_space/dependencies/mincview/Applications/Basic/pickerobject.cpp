#include <strstream>
#include <vector>
#include <qdatetime.h>

#include "vtkIndent.h"
#include "vtkAssembly.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCellPicker.h"
#include "vtkActor2D.h"
#include "vtkLODActor.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkProperty.h"
#include "vtkPolyData.h" 
#include "vtkImageData.h"
#include "vtkPolyDataMapper.h" 
#include "vtkSphereWithTextPointWidget.h"
#include "vtkListOfSphereWithTextPointWidgets.h"
#include "vtkPoints.h"
#include "vtkTransform.h"
#include "vtkPropPicker.h"
#include "vtkImageData.h"
#include "vtkTagReader.h"
#include "vtkTagWriter.h"
#include "pickerobject.h"
#include "pickcommand.h"
#include "scenemanager.h"
#include "generatedsurface.h"
#include "view.h"
#include "pickerobjectsettingsdialog.h"

#include "vtkObject.h"
#include "vtkCallbackCommand.h"
#include "vtkIgnsEvents.h"

void PickerObjectInteractionCallback( vtkObject* caller,
                                       unsigned long event,
                                       void* clientdata,
                                       void* calldata )
{
    PickerObject * self = (PickerObject*)clientdata;
    self->PickerInteraction(event);
}

PickerObject::PickerObject()
{
    this->PickerType = UNDEFINED;
    this->InitDone = FALSE;
    this->ImageObj = 0;
    this->PolyDataObj = 0;
    this->ImagePlane = 0;
    this->renderer = 0;
    this->interactor = 0;
    this->picker = vtkCellPicker::New();
    this->savePicker = 0;
    this->pickActor = 0;
    this->pickObserver = PickCommand::New();
    this->savedPoints = 0;  
    this->selectedPoints = PointsObject::New();
    this->tempPoints = PointsObject::New();  
    connect(this->selectedPoints, SIGNAL(PointsChanged()), this, SLOT(RefreshObject()));
    connect(this->selectedPoints, SIGNAL(PointRemoved()), this, SLOT(RemovePoint()));
    this->textActor = vtkActor2D::New();
    this->textMapper = vtkTextMapper::New();
    this->textActor->SetMapper( textMapper );
    this->movePointCommand = this->selectedPoints->GetMovePointCommand();
    this->mousePickCommand = vtkCallbackCommand::New();
    this->mousePickCommand->SetAbortFlagOnExecute(1);
    this->InteractorStyle = 0;
    this->CurrentPointIndex = -1;
    this->PointRadius = 3.0;
    this->TagLabelSize = INITIAL_TAG_LABEL_SCALE;
    
    this->SetObjectType(PICKER_TYPE);
}

PickerObject::~PickerObject()
{
    if (this->ImageObj)
        this->ImageObj->UnRegister(this);
    if (this->PolyDataObj)
        this->PolyDataObj->UnRegister(this);
    if (this->ImagePlane)
        this->ImagePlane->UnRegister(this);
    if (this->renderer)
        this->renderer->UnRegister( this );
    if (this->interactor)
        this->interactor->UnRegister( this );
    if (this->savePicker)
        this->savePicker->UnRegister(this);
    if (this->picker)
        this->picker->Delete();
    if (this->selectedPoints)
        this->selectedPoints->Delete();
    if (this->tempPoints)
        this->tempPoints->Delete();
    if (this->savedPoints)
        this->savedPoints->Delete();
    if (this->textActor)
        this->textActor->Delete();
    if (this->textMapper)
        this->textMapper->Delete( );
    if (this->pickActor)
    {
        if (this->PickerType == OBJ)
            this->pickActor->PickableOff();
        this->pickActor->UnRegister(this);
    }
    this->pickObserver->disconnect(this);
    this->pickObserver->Delete();
    if (this->InteractorStyle)
    {
        this->InteractorStyle->RemoveObserver (this->mousePickCommand);
        this->InteractorStyle->UnRegister(this);
    }
    this->mousePickCommand->Delete();
}

void PickerObject::SetImageObject( ImageObject * obj )
{
    if( this->ImageObj == obj )
    {
        return;
    }
    if( this->ImageObj )
    {
        this->ImageObj->UnRegister( this );
    }
    this->ImageObj = obj;
    if( this->ImageObj )
    {
        this->ImageObj->Register( this );
        this->objectName = obj->GetName();
        vtkReal spacing[3];
        this->ImageObj->GetImage()->GetSpacing(spacing);
        this->PointRadius =  spacing[0]*1.5;
        if (this->PointRadius < 0)
            this->PointRadius = -this->PointRadius;
    }
    this->selectedPoints->SetImageObject(obj);
}

bool PickerObject::Init()
{
    this->selectedPoints->SetManager(this->Manager);

    switch (this->PickerType)
    {
    case SURFACE:
    case OBJ:
        this->selectedView = this->Manager->GetView(THREED_VIEW_TYPE);
        if (this->selectedView)
        {

            if (this->renderer != this->selectedView->GetRenderer())
            {
                if (this->renderer)
                    this->renderer->UnRegister( this );
                this->renderer = this->selectedView->GetRenderer();
                this->renderer->Register( this );
            }
            if (this->interactor != this->selectedView->GetInteractor())
            {
                if (this->interactor)
                    this->interactor->UnRegister( this );
                this->interactor = this->selectedView->GetInteractor();
                this->interactor->Register( this );
            }
            this->savePicker = vtkPropPicker::SafeDownCast(this->selectedView->GetPicker());
            if (this->savePicker)
                this->savePicker->Register( this );
       }
        else
            return FALSE;
        break;
    default:
        return FALSE; 
    }
    if (!this->InitDone)
    {
        switch (this->PickerType)
        {
        case SURFACE:
            if (this->ImageObj->GetObjectSurface())
            {
                this->pickActor = this->ImageObj->GetObjectSurface()->GetSurfaceActor();
                this->Transform = this->ImageObj->GetCurrentTransform();
                this->SetName("SurfacePicker");
                break;
            }
            return FALSE;
        case OBJ:
            if (this->PolyDataObj->Get3DViewActor())
            {
                this->SetName("ObjPicker");
                this->pickActor = this->PolyDataObj->Get3DViewActor();
                break;
            }
            return FALSE;
        default:
            return FALSE;
        }

        // Create a text mapper and actor to display the results of picking.
        vtkTextProperty* tprop = textMapper->GetTextProperty();
        tprop->SetFontFamilyToArial();
        tprop->SetFontSize( 15 );
        tprop->BoldOn();
        tprop->SetColor( 1, 0, 0 );

        this->textActor->VisibilityOff();
        this->picker->Register(this);
        this->picker->AddObserver( vtkCommand::EndPickEvent, this->pickObserver );
        this->picker->SetTolerance(0.005);
        this->picker->AddPickList( this->pickActor );
        this->picker->PickFromListOn();

        this->pickObserver->SetPicker(this->picker);

        if (this->PickerType == SURFACE || this->PickerType == OBJ)
        {
            this->mousePickCommand->SetClientData(this); 
            this->mousePickCommand->SetCallback(PickerObjectInteractionCallback);
        }
        if (this->PickerType == SURFACE || this->PickerType == OBJ)
        {
            this->selectedPoints->EnablePickRefiner(1);
        }
        connect( this->pickObserver, SIGNAL( NewPointAdded(vtkReal*) ), this, SLOT( AddNewPoint(vtkReal*) ) );
        if (this->InteractorStyle)
        {
            this->InteractorStyle->AddObserver(vtkCommand::LeftButtonPressEvent, this->mousePickCommand);
            this->InteractorStyle->AddObserver(vtkCommand::MouseMoveEvent, this->mousePickCommand);
            this->InteractorStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent,  this->mousePickCommand);
        }
    }
    this->interactor->SetPicker( this->picker );
    this->renderer->AddActor2D( this->textActor );
    this->InitDone = TRUE;
    if (this->pickActor)
        this->pickActor->PickableOn();
    return TRUE;
}

void PickerObject::DeInit()
{
    if (this->savePicker)
    {
        if (this->interactor)
            this->interactor->SetPicker(this->savePicker);
        this->savePicker->UnRegister(this);
        this->savePicker = 0;
    }
    this->selectedPoints->ShowCrosshair(0);
    this->pickActor->PickableOff();
}

void PickerObject::SetRenderer(vtkRenderer * ren)
{
    if( this->renderer == ren )
        return;
    if( this->renderer )
        this->renderer->UnRegister( this );
    this->renderer = ren;
    if( this->renderer )
        this->renderer->Register( this );
}
void PickerObject::SetInteractor(vtkRenderWindowInteractor * iren)
{
    if( this->interactor == iren )
        return;
    if( this->interactor )
        this->interactor->UnRegister( this );
    this->interactor = iren;
    if( this->interactor )
        this->interactor->Register( this );
}

void PickerObject::EnableInteraction(int yes)
{
    if (this->selectedPoints)
    {
        this->selectedPoints->GetPointList()->EnableInteraction(yes);
    }   
    if (this->InteractorStyle)
    {
        if (yes)
        {
            this->InteractorStyle->AddObserver(vtkCommand::LeftButtonPressEvent, this->mousePickCommand);
            this->InteractorStyle->AddObserver(vtkCommand::MouseMoveEvent, this->mousePickCommand);
            this->InteractorStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, this->mousePickCommand);
        }
        else
        {
            this->InteractorStyle->RemoveObserver(this->mousePickCommand);
        }
    }
    if (this->picker && this->pickObserver)
    {
        if (yes)
            this->picker->AddObserver( vtkCommand::EndPickEvent, this->pickObserver );
        else
            this->picker->RemoveObserver( this->pickObserver );
    }

}

void PickerObject::DisablePickerOnPlanes()
{
    this->selectedPoints->EnablePickRefiner(0);
}

// picking operations
bool PickerObject::LoadPointsFromTagFile(QString fileName)
{
//    this->PrintListOfPoints("In LoadPointsFromTagFile", this->selectedPoints->GetPointList());
    this->SaveSelectedPoints();
    if (this->ReadTagFile( fileName ))
        return true;
    return false;
}

void PickerObject::DisplayPoints( )
{
    this->SaveSelectedPoints();
    this->RemoveAllPoints();
    this->RestoreSelectedPoints();
    this->selectedView->NotifyNeedRender();
}

void PickerObject::HighlightPoint(int index)
{
    if (index >= 0 && index < this->selectedPoints->GetNumberOfPoints() )
    {
        vtkReal pt[3];
        vtkSphereWithTextPointWidget *point;
        int i = this->selectedPoints->GetHighlightedPointIndex();
        if (i > -1)
        {
            point = this->selectedPoints->GetPoint(i);
            point->ExternalHighlight(0);
        }
        this->CurrentPointIndex = index;
        point = this->selectedPoints->GetPoint(this->CurrentPointIndex);
        point->ExternalHighlight(1);
        this->GetPointCoordinates(CurrentPointIndex, pt);
        this->ShowPointCoordinatesIn3DView(CurrentPointIndex+1, pt[0], pt[1], pt[2]);
        this->selectedView->NotifyNeedRender();
        POINT_PROPERTIES p;
        p.index = this->CurrentPointIndex;
        p.x = pt[0];
        p.y = pt[1];
        p.z = pt[2];
        emit PointSelected(&p);
    }
}

void PickerObject::RefreshObject( )
{
    switch (this->PickerType)
    {
    case SURFACE:
        if (this->ImageObj)
            this->ImageObj->RefreshObject();
        break;
    case OBJ:
    default:
        break;
    }
}

void PickerObject::AddNewPoint(vtkReal *pt)
{
    char num[8];
    this->CurrentPointIndex = this->selectedPoints->GetNumberOfPoints();
    vtkSphereWithTextPointWidget *newPoint = vtkSphereWithTextPointWidget::New();
    if (this->ImageObj)
    {
        newPoint->SetInput(this->ImageObj->GetImage());
        if (!this->Manager->GetRegistrationFlag())
            newPoint->SetTransform(this->ImageObj->GetCurrentTransform());
        newPoint->PlaceWidget();
    }
    else
    {
        vtkReal bounds[6];
        this->PolyDataObj->GetPolyData()->GetBounds(bounds);
        newPoint->PlaceWidget(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
    }
    newPoint->SetInteractor(this->interactor);
    newPoint->SetRadius(this->PointRadius);
    newPoint->SetLabelScale(this->TagLabelSize);
    newPoint->SetHotSpotSize(this->PointRadius+0.25*this->PointRadius);
    newPoint->SetPosition(pt[0], pt[1], pt[2]);
    newPoint->AddObserver(vtkCommand::UserEvent+IGNS_MOVE_3D_POINT, this->movePointCommand);
    newPoint->AddObserver(vtkCommand::UserEvent+IGNS_HIGHLIGHT_POINT, this->movePointCommand);
    newPoint->SetPointIndex(this->CurrentPointIndex);
    sprintf(num, "%i", this->CurrentPointIndex+1);
    newPoint->SetLabel(num);
    newPoint->SetEnabled(1);
    newPoint->ShowLabel(true);
    this->selectedPoints->AddPoint(newPoint);
    newPoint->Delete();
    POINT_PROPERTIES p;
    p.index = this->CurrentPointIndex;
    p.x = pt[0];
    p.y = pt[1];
    p.z = pt[2];
    emit PointAdded(&p);
    this->HighlightPoint(this->CurrentPointIndex);
    this->ShowPointCoordinatesIn3DView(this->CurrentPointIndex+1, pt[0], pt[1], pt[2]);
}

void PickerObject::RemovePoint()
{
    this->HidePointCoordinatesIn3DView();
    this->DisplayPoints();
    this->selectedPoints->ShowCrosshair(0);
    this->selectedPoints->GetPointList()->SeNumericLabels();
    this->RefreshObject();
    emit UpdatePoints();
}

void PickerObject::SaveSelectedPoints( )
{
    this->tempPoints->GetPointList()->Reset();
    this->tempPoints->CopyPointList(this->selectedPoints->GetPointList());
}

void PickerObject::RestoreSelectedPoints( )
{
    this->selectedPoints->GetPointList()->Reset();
    this->selectedPoints->CopyPointList(this->tempPoints->GetPointList());
    this->selectedPoints->GetPointList()->EnableAllPointsOn();
    // put back the observers
    int i, n = this->selectedPoints->GetNumberOfPoints();
    vtkSphereWithTextPointWidget *point;
    for (i = 0; i < n; i++)
    {
        point = this->selectedPoints->GetPoint(i);
        point->SetRadius(this->PointRadius);
        point->SetLabelScale(this->TagLabelSize);
        point->SetHotSpotSize(this->PointRadius+0.25*this->PointRadius);
        point->AddObserver(vtkCommand::UserEvent+IGNS_MOVE_3D_POINT, this->movePointCommand);
        point->AddObserver(vtkCommand::UserEvent+IGNS_HIGHLIGHT_POINT, this->movePointCommand);
        point->Update();
    }        
}

void PickerObject::RemoveAllPoints( )
{
    this->selectedPoints->GetPointList()->EnableAllPointsOff();
    // remove the observers
    int i, n = this->selectedPoints->GetNumberOfPoints();
    vtkSphereWithTextPointWidget *point;
    for (i = 0; i < n; i++)
    {
        point = this->selectedPoints->GetPoint(i);
        point->RemoveObserver(vtkCommand::UserEvent+IGNS_MOVE_3D_POINT); 
        point->RemoveObserver(vtkCommand::UserEvent+IGNS_HIGHLIGHT_POINT); 
    }        
    this->selectedPoints->GetPointList()->Reset();
}

int PickerObject::SavePickedPoints(QString fileName)
{
    int n;
    n = this->selectedPoints->GetNumberOfPoints();
    if (n > 0)
        this->WriteTagFile(fileName);
    return n;
}

int PickerObject::GetNumberOfPoints()
{
    return this->selectedPoints->GetNumberOfPoints();
}

void PickerObject::SavePointsForUndo()
{
    if (!this->savedPoints)
        this->savedPoints = PointsObject::New();
    else 
        this->savedPoints->GetPointList()->Reset();
    this->savedPoints->CopyPointList(this->selectedPoints->GetPointList());
}

void PickerObject::UndoPicking()
{
    this->selectedPoints->GetPointList()->Reset();
    if (this->savedPoints)
    {
        this->selectedPoints->CopyPointList(this->savedPoints->GetPointList());
        this->selectedPoints->GetPointList()->EnableAllPointsOn();
    }
}

QString PickerObject::GetPointCoordsString(int idx)
{
    vtkReal pt[3];
    vtkSphereWithTextPointWidget *point = this->selectedPoints->GetPoint( idx );
    point->GetPosition(pt);
    QString tmp = QString("(%1, %2, %3)") 
                    .arg(pt[0])
                    .arg(pt[1])
                    .arg(pt[2]);
    return tmp;
}

vtkReal* PickerObject::GetPointCoordinates(int idx)
{
    static vtkReal pt[3];
    vtkSphereWithTextPointWidget *point = this->selectedPoints->GetPoint( idx );
    point->GetPosition(pt);
    return pt;
}

void PickerObject::GetPointCoordinates(int idx, vtkReal *pt)
{
    if (pt)
    {
        vtkSphereWithTextPointWidget *point = this->selectedPoints->GetPoint( idx );
        point->GetPosition(pt);
    }
}

void PickerObject::SetPointCoordinates(int idx, vtkReal *pt)
{
    if (pt)
    {
        vtkSphereWithTextPointWidget *point = this->selectedPoints->GetPoint( idx );
        point->SetPosition( pt );
    }
}

void PickerObject::UpdatePointList()
{
    emit UpdatePoints();
}

void PickerObject::SetPointRadius(vtkReal radius)
{
    vtkReal radiusDif = radius - this->PointRadius;
    this->TagLabelSize += radiusDif;
    this->PointRadius = radius;     
    if (this->PointRadius < 0)
        this->PointRadius = -this->PointRadius;
    if (this->ImageObj)
    {
        vtkReal spacing[3];
        this->ImageObj->GetImage()->GetSpacing(spacing);
        if (this->PointRadius < spacing[0])
            this->PointRadius =  spacing[0];
    }
    this->DisplayPoints();
}

void PickerObject::RemoveCrosshairFromViews()
{
    this->selectedPoints->ShowCrosshair(0);
}

void PickerObject::SetPlanesPosition(int idx, vtkReal posX, vtkReal posY, vtkReal posZ)
{
    this->selectedPoints->SetImagePlanesPosition(idx, posX, posY, posZ);
}

void PickerObject::OverrideMouseActions()
{
    this->selectedPoints->OverrideMouseActions();
}

void PickerObject::RestoreMouseActions()
{
    this->selectedPoints->RestoreMouseActions();
}

void PickerObject::WriteTagFile(QString filename)
{
    //Points do not have names, just numbers
    vtkPoints *pts = vtkPoints::New();
    vtkReal pt[3];
    int i, n = this->selectedPoints->GetNumberOfPoints();
    std::vector<std::string> pointNames;
    char c[3];
    for (i = 1; i < n+1; i++)
    {
        sprintf(c, "%i", i);
        pointNames.push_back( std::string( ( (const char*)c) ));
        vtkSphereWithTextPointWidget *point = this->selectedPoints->GetPoint( i-1 );
        point->GetPosition(pt);
        pts->InsertNextPoint(pt);
    }
    vtkTagWriter * writer = vtkTagWriter::New();
    writer->SetFileName( filename );
    writer->SetPointNames( pointNames );
    writer->AddVolume( pts, (const char*)this->objectName );
    writer->SetReferenceDataFile( (const char*)this->objectName );
    QDateTime saveTime = QDateTime::currentDateTime();
    writer->SetTimeStamp(saveTime.toString(Qt::ISODate));
    writer->Write();
    writer->Delete();
    pts->Delete();
}

bool PickerObject::ReadTagFile( QString & filename )
{
    vtkTagReader * reader = vtkTagReader::New();
    if( reader->CanReadFile( filename ) )
    {    
        reader->SetFileName( filename );
        reader->Update();
        vtkPoints *pts = reader->GetVolume( 0 );
        int i, n = pts->GetNumberOfPoints();
        for (i = 0; i < n; i++)
        {
            this->AddNewPoint(pts->GetPoint(i));
        }
        reader->Delete();
        return true;
    }
    reader->Delete();
    return false;
}

void PickerObject::PreDisplaySetup()
{
}

void PickerObject::Setup( View * view, vtkAssembly * placementHint ) 
{
}

void PickerObject::Update()
{
    this->selectedPoints->Update();
}

void PickerObject::Release( View * view ) 
{
    if (view == this->selectedView)
    {
        this->RemoveAllPoints();
        this->renderer->RemoveActor2D( this->textActor );
        this->selectedView->NotifyNeedRender();
    }
}

QWidget * PickerObject::CreateSettingsDialog( QWidget * parent ) 
{ 
    PickerObjectSettingsDialog * res = new PickerObjectSettingsDialog( parent, "Picker Settings Dialog", Qt::WDestructiveClose );
    res->SetSceneManager( this->Manager );
    return res;
}

void PickerObject::PickerInteraction( unsigned long event )
{
    if (!this->InteractorStyle)
        return;
    switch( event )
    {
    case vtkCommand::LeftButtonPressEvent:
        this->MouseMotion = 0;
        this->InteractorStyle->OnLeftButtonDown();
        break;
    case vtkCommand::LeftButtonReleaseEvent:
        if (this->MouseMotion == 0)
        {
            int *pick = this->interactor->GetEventPosition();
            this->picker->Pick((vtkReal)pick[0], (vtkReal)pick[1], 0.0, this->renderer);
        }
        this->InteractorStyle->OnLeftButtonUp(); 
        break;
    case vtkCommand::MouseMoveEvent:
        this->MouseMotion = 1;
        this->InteractorStyle->OnMouseMove();
        break;
    }
}

void PickerObject::ShowPointCoordinatesIn3DView(int id, vtkReal x, vtkReal y, vtkReal z)
{
    char text[64];
    sprintf( text, "(%i: %5.5f, %5.5f, %5.5f)", id, x, y, z );
    this->textMapper->SetInput( text );
    this->textActor->SetPosition( 10, 10 );
    this->textActor->VisibilityOn();
    this->selectedView->NotifyNeedRender();
}


void PickerObject::HidePointCoordinatesIn3DView()
{
    this->textMapper->SetInput( "" );
    this->textActor->SetPosition( 10, 10 );
    this->textActor->VisibilityOn();
    this->selectedView->NotifyNeedRender();
}

void PickerObject::MovePoint(POINT_PROPERTIES *pt)
{
    emit PointMoved(pt);
}

void PickerObject::PrintListOfPoints(QString title, vtkListOfSphereWithTextPointWidgets *list)
{
    vtkIndent indent;
    if (list == 0)
        list = this->selectedPoints->GetPointList();
    cout << "########### " << title << endl;
    list->PrintSelf(cout, indent);
    cout << "########### end\n";
}

void PickerObject::HideShow()
{
    if(ObjectHidden)
    {
        this->selectedPoints->GetPointList()->EnableAllPointsOff();
    }
    else
    {
        this->selectedPoints->GetPointList()->EnableAllPointsOn();
    }
}

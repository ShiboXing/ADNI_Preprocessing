#include <iostream>
#include "scenemanager.h"
#include "quadviewwindow.h"
#include "objectlistdialog.h"
#include "simpleobjectlistdialog.h"
#include "scenemanagersettingsdialog.h"

#include "MINCReader.h"
#include "vtkMNIOBJReader.h"
#include "vtkTagReader.h"
#include "vtkInteractorStyleImage.h"
#include "vtkTransform.h"
#include "vtkMultiImagePlaneWidget.h"
#include "vtkImageData.h"
#include "vtkPolyDataReader.h"
#include "vtkDataObjectReader.h"
#include "vtkStructuredPointsReader.h"
#include "vtkStructuredGridReader.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkRectilinearGridReader.h"
#include "vtkErrorCode.h"
#include "view.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "imageobject.h"
#include "polydataobject.h"
#include "ignsmsg.h"
#include "usertransformations.h"
#include "reslicer.h"
#include "pointsobject.h"
#include "version.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <string>
#include <qmessagebox.h>

ObjectSerializationMacro( SceneManager );

SceneManager::SceneManager()
{       
    this->CurrentObjectName = "";
    this->CurrentView = -1;
    this->ViewBackgroundColor[0] = 0;
    this->ViewBackgroundColor[1] = 0;
    this->ViewBackgroundColor[2] = 0;
    this->MincAndObj = 0;
    this->PickObj = 0;
    this->UsedTransforms = 0;
    this->ReferenceDataObject = 0;
    this->DataObjectsCount = 0;
    this->MRIDataObjectsCount = 0;
    this->ResliceObjectsCount = 0;
    this->ImageReslicer = 0;
    this->GlobalOpacity = 1.0;
    this->ViewMode = VIEW_SELECTED;
    m_expertMode = 0;
    this->SupportedSceneSaveVersion = IGNS_SCENE_SAVE_VERSION;
    this->SceneSaveVersionFromFile = IGNS_SCENE_SAVE_VERSION;
}

SceneManager::~SceneManager()
{
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        (*it)->Delete();
    }

    for( ObjectList::iterator it2 = Objects.begin(); it2 != Objects.end(); ++it2 )
    {
        (*it2)->Delete();
    }
    if (this->MincAndObj)
        this->MincAndObj->Delete();
    if (this->PickObj)
        this->PickObj->Delete();
    if (this->ImageReslicer)
        this->ImageReslicer->Delete();
}

void SceneManager::Serialize( Serializer * ser )
{
    ::Serialize( ser, "ViewMode", this->ViewMode );
    SceneObject *preop = this->GetObject(PREOP_ROOT_OBJECT_NAME);
//    ::Serialize( ser, "PreoperativeRootObject", preop );
    int i, nChildren, objType = 0;
    if(!ser->IsReader())
    {
        nChildren = preop->GetNumberOfChildren();
    }
    ::Serialize( ser, "NumberOfPreoperativeObjects", nChildren );
    for (i = 0; i < nChildren; i++)
    {
        if(!ser->IsReader())
        {
            SceneObject *obj = preop->GetChild(i);
            objType = obj->GetObjectType(); 
            if (objType & MINC_TYPE)
            {
                QString childNo = QString( "PreopObject_%1" ).arg(i);
                ser->BeginSection(childNo);
                ::Serialize( ser, "ObjectType", objType);
                ImageObject *iObj = (ImageObject*)obj;
                ::Serialize( ser, "ImageObject", iObj );
            }
            else if (objType & OBJ_TYPE)
            {
                QString childNo = QString( "PreopObject_%1" ).arg(i);
                ser->BeginSection(childNo);
                ::Serialize( ser, "ObjectType", objType);
                PolyDataObject *pObj = (PolyDataObject*)obj;
                ::Serialize( ser, "PolyDataObject", pObj );
            }
            else if (objType & POINTS_OBJ_TYPE)
            {
                QString childNo = QString( "PreopObject_%1" ).arg(i);
                ser->BeginSection(childNo);
                ::Serialize( ser, "ObjectType", objType);
                PointsObject *pt = (PointsObject*)obj;
                ::Serialize( ser, "PointsObject", pt );
            }
            // no other object may be save yet
        }
        else
        {
            QString childNo = QString( "PreopObject_%1" ).arg(i);
            ser->BeginSection(childNo);
            ::Serialize( ser, "ObjectType", objType);
            if (objType & MINC_TYPE)
            {
                ImageObject *tmpObj = ImageObject::New();
                ::Serialize( ser, "ImageObject", tmpObj );
                int type = tmpObj->GetObjectType();
                bool hidden = tmpObj->GetObjectHidden();
                QString fileName(tmpObj->GetFullFileName());
                QString displayName(tmpObj->GetName());
                int viewOutline = tmpObj->GetViewOutline();
                int viewPlanes[3];
                for (int i = 0; i < 3; i++)
                    viewPlanes[i] = tmpObj->GetViewPlane(i);
                int view3DPlanes = tmpObj->GetView3DPlanes();
                int viewSurface = tmpObj->GetViewSurface();
                int lutIndex = tmpObj->GetLutIndex();
                double opacity[3];
                tmpObj->GetAllPlanesOpacity(opacity);
                double windowLevel[2];
                tmpObj->GetUsedWindowLevel(windowLevel);
                double surfaceContour = tmpObj->GetSurfaceContourValue();
                double surfaceOpacity = tmpObj->GetSurfaceOpacityValue();
                int gaussianSmoothing = tmpObj->GetGaussianSmoothingFlag();
                double radiusFactor = tmpObj->GetRadiusFactor();
                double standardDeviation = tmpObj->GetStandardDeviation();
                tmpObj->Delete();
                if (this->OpenFile( fileName,
                                preop,
                                objType,
                                displayName ))
                {
                    ImageObject *newObj = reinterpret_cast<ImageObject *>(this->GetObject(displayName));
                    newObj->SetViewOutline(viewOutline);
                    newObj->SetViewSurface(viewSurface);
                    if (viewSurface)
                    {
                        newObj->SetSurfaceContourValue(surfaceContour);
                        newObj->SetSurfaceOpacityValue(surfaceOpacity);
                        if (gaussianSmoothing)
                        {
                            newObj->SetGaussianSmoothingFlag(gaussianSmoothing);
                            newObj->SetRadiusFactor(radiusFactor);
                            newObj->SetStandardDeviation(standardDeviation);
                        }
                    }
                    newObj->SetView3DPlanes(view3DPlanes);
                    newObj->ChooseColorTable(lutIndex);
                    for (int i = 0; i < 3; i++)
                        newObj->SetViewPlane(i, viewPlanes[i]);
                    if (opacity[0] == opacity[1] && opacity[1] == opacity[2])
                        newObj->SetAllPlanesOpacity(opacity[0]);
                    else    
                    {
                        for (int i = 0; i < 3; i++)
                            newObj->SetOnePlaneOpacity(i, opacity[i]);
                    }
                    newObj->SetUsedWindowLevel((int)windowLevel[0], (int)windowLevel[1]);
                    newObj->SetSurfaceContourValue(surfaceContour);
                    newObj->SetSurfaceOpacityValue(surfaceOpacity);
                    newObj->UpdateSettingsWidget();
                    newObj->SetObjectHidden(hidden);
                    if (hidden)
                        newObj->HideShow();
                }
            }
            else if (objType & OBJ_TYPE)
            {
                PolyDataObject *tmpObj = PolyDataObject::New();
                ::Serialize( ser, "PolyDataObject", tmpObj );
                int type = tmpObj->GetObjectType();
                bool hidden = tmpObj->GetObjectHidden();
                QString fileName(tmpObj->GetFullFileName());
                QString displayName(tmpObj->GetName());
                int renderingMode = tmpObj->GetRenderingMode();
                int scalarsVisible = tmpObj->GetScalarsVisible();
                double opacity = tmpObj->GetOpacity();
                double objectColor[3];
                tmpObj->GetColor(objectColor, 0);
                int DTITracks = tmpObj->GetDTITracks();
                double DTITubeRadius = tmpObj->GetDTITubeRadius();
                int DTITubeResolution = tmpObj->GetDTITubeResolution();
                tmpObj->Delete();
                if (!DTITracks || (m_expertMode && m_expertMode->GetExpertLevel() > 1))
                {
                    if (this->OpenFile( fileName,
                                    preop,
                                    objType,
                                    displayName ))
                    {
                        PolyDataObject *newObj = reinterpret_cast<PolyDataObject *>(this->GetObject(displayName));
                        newObj->SetRenderingMode(renderingMode);
                        newObj->SetScalarsVisible(scalarsVisible);
                        newObj->SetOpacity(opacity);
                        newObj->SetColor(objectColor);
                        newObj->SetObjectHidden(hidden);
                        if (DTITracks)
                        {
                            newObj->SetDTITracks(DTITracks);
                            newObj->SetDTITubeRadius(DTITubeRadius);
                            newObj->SetDTITubeResolution(DTITubeResolution);
                        }
                        if (hidden)
                            newObj->HideShow();
                        newObj->UpdateSettingsWidget();
                    }
                }
            }
            else if (objType & POINTS_OBJ_TYPE)
            {
                PointsObject *tmpObj = PointsObject::New();
                tmpObj->SetManager(this);
                ::Serialize( ser, "PointsObject", tmpObj );
                int type = tmpObj->GetObjectType();
                bool hidden = tmpObj->GetObjectHidden();
                double pointRadius = tmpObj->GetPointRadius();
                double labelSize = tmpObj->GetLabelSize();
                int showLabel = tmpObj->GetShowLabel();
                vtkReal enabledColor[3] = {0.0, 0.0, 1.0};
                vtkReal disabledColor[3] = {0.75, 0.5, 0.0};
                vtkReal selectedColor[3] = {0.0, 1.0, 0.0};
                vtkReal opacity = 1.0;
                int newVersion = QString::compare(this->SceneSaveVersionFromFile, "1.0");
                if (newVersion > 0)
                {
                    tmpObj->GetEnabledColor(enabledColor);
                    tmpObj->GetDisabledColor(disabledColor);
                    tmpObj->GetSelectedColor(selectedColor);
                    opacity = tmpObj->GetOpacity();
                }
                QString fileName(tmpObj->GetFullFileName());
                QString displayName(tmpObj->GetName());
                tmpObj->Delete();
                if (this->OpenFile( fileName,
                                preop,
                                objType,
                                displayName ))
                {
                    PointsObject *newObj = reinterpret_cast<PointsObject *>(this->GetObject(displayName));
                    newObj->SetObjectHidden(hidden);
                    newObj->SetPointRadius(pointRadius);
                    newObj->SetLabelSize(labelSize);
                    newObj->SetShowLabel(showLabel);
                    if (newVersion > 0)
                    {
                        newObj->SetEnabledColor(enabledColor);
                        newObj->SetDisabledColor(disabledColor);
                        newObj->SetSelectedColor(selectedColor);
                        newObj->SetOpacity(opacity);
                    }
                    if (hidden)
                        newObj->HideShow();
                    newObj->UpdateSettingsWidget();
                }
            }
        }
        ser->EndSection();
    }
    if(ser->IsReader())
    {
        nChildren = preop->GetNumberOfChildren();
        // connect PointsObjects to images
        for (i = 0; i < nChildren; i++)
        {
            SceneObject *obj = preop->GetChild(i);
            objType = obj->GetObjectType();
            if (objType & POINTS_OBJ_TYPE)
            {
                PointsObject *pts = reinterpret_cast<PointsObject *>(obj);
                if (pts->GetImageObjectName() != "none")
                {
                    obj = this->GetObject(pts->GetImageObjectName());
                    if (obj && (obj->GetObjectType() & MINC_TYPE))
                        pts->SetImageObject(reinterpret_cast<ImageObject *>(this->GetObject(pts->GetImageObjectName())));
                    else
                        pts->RemoveImageObject();
                }
            }
        }
    }
}

int SceneManager::OpenFile( QString filename, SceneObject * attachTo, int fileType, const QString & dataObjectName, const QString & transformName )
{
    if( QFile::exists( filename ) )
    {
        MINCReader * reader = MINCReader::New();
        if( reader->CanReadFile( filename ) )
        {
            reader->Delete();
            OpenMincFile( filename, attachTo, fileType, dataObjectName, transformName );
            return 1;
        }
        reader->Delete();

        vtkMNIOBJReader * readerObj = vtkMNIOBJReader::New();
        if( readerObj->CanReadFile( filename ) )
        {
            readerObj->Delete();
            OpenObjFile( filename, attachTo, fileType, dataObjectName, transformName );
            return 1;
        }
        readerObj->Delete();

        // try tag file
        vtkTagReader * readerTag = vtkTagReader::New();
        if( readerTag->CanReadFile( filename ) )
        {
            readerTag->Delete();
            OpenTagFile( filename, attachTo, fileType, dataObjectName, transformName );
            return 1;
        }
        readerTag->Delete();
        
        // finally try vtk
        if (m_expertMode && m_expertMode->GetExpertLevel() > 1)
            return OpenVTKFile( filename, attachTo, fileType, dataObjectName, transformName );
    }
    return 0;
}


void SceneManager::OpenMincFile( QString filename, SceneObject * attachTo, int fileType, const QString & dataObjectName, const QString & transformName )
{
    if( QFile::exists( filename ) )
    {
        ImageObject * image = ImageObject::New();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "#### OpenMincFile: ImageObject ref. count. =" << image->GetReferenceCount() << endl);
#endif
        MINCReader * reader = MINCReader::New();
        reader->SetFileName( filename );
        reader->SetDataType(VTK_UNSIGNED_SHORT);
        reader->Update();
        vtkMINCImageAttributes2 *attributes = reader->GetImageAttributes();
        image->SetImage(reader->GetOutput());
        image->SetMINCImageAttributes(attributes);
        image->SetDirectionCosines(reader->GetDirectionCosines());
        vtkMatrix4x4 *transformMatrix = reader->GetTransformationMatrix();
        reader->Delete();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "Reader Transform :" << "\n" <<
        transformMatrix->GetElement(0, 0) << ", " <<
        transformMatrix->GetElement(0, 1) << ", " <<
        transformMatrix->GetElement(0, 2) << ", " <<
        transformMatrix->GetElement(1, 0) << ", " <<
        transformMatrix->GetElement(1, 1) << ", " <<
        transformMatrix->GetElement(1, 2) << ", " <<
        transformMatrix->GetElement(2, 0) << ", " <<
        transformMatrix->GetElement(2, 1) << ", " <<
        transformMatrix->GetElement(2, 2) << ", " <<
        endl);
#endif
        QFileInfo info( filename );
        image->SetDataFileName( info.fileName() );
        image->SetFullFileName(info.absFilePath());
        if (!dataObjectName.isEmpty())
            image->SetName( dataObjectName );
        else
            image->SetName( info.fileName() );
        image->SetObjectType(MINC_TYPE | fileType);
        image->SetManager(this);

        this->AddObject( image, attachTo );
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "#### OpenMincFile: ImageObject ref. count. =" << image->GetReferenceCount() << endl);
#endif
        if (fileType & INTRAOP_TYPE)
        {
            if (UsedTransforms)
            {
                vtkTransform *t = UsedTransforms->GetUserTransform(transformName);
                int index = UsedTransforms->GetTransformIndex(transformName);
                image->SetIntraopFinalAppliedTransformMatrix(transformMatrix);
                if (t && index > 1)
                {
                    if (UsedTransforms->GetTransformType(index) == ABSOLUTE_TRANSFORM)
                        image->SetIntraopFinalAppliedTransformMatrix(t->GetMatrix(), index);
                    else
                    {
                        image->GetIntraopFinalAppliedTransform()->Concatenate(t);
                    }
                }
                image->SetTransformName(transformName);
            }
        }
        if (this->ViewMode == VIEW_SYNCHRONIZED)
        {
            this->AddImageInSynchronizedView(image);
        }
    }
}

void SceneManager::OpenObjFile( QString filename, SceneObject * attachTo, int fileType, const QString & dataObjectName, const QString & transformName )
{
    if( QFile::exists( filename ) )
    {
        vtkMNIOBJReader * reader = vtkMNIOBJReader::New();
        reader->SetFileName( filename );
        reader->Update();

        PolyDataObject * object = PolyDataObject::New();
        object->SetPolyData( reader->GetOutput() );
        object->SetProperty( reader->GetProperty() );
        
        if (fileType & REFERENCE_DATA_TYPE)
            fileType &= ~REFERENCE_DATA_TYPE;
        QFileInfo info( filename );
        object->SetDataFileName( info.fileName() );
        object->SetFullFileName(info.absFilePath());
        if (!dataObjectName.isEmpty())
            object->SetName( dataObjectName );
        else
            object->SetName( info.fileName() );
        object->SetObjectType(OBJ_TYPE | fileType);
        object->SetManager(this);
        if (fileType & INTRAOP_TYPE)
        {
            if (UsedTransforms)
                object->SetTransform(UsedTransforms->GetUserTransform(transformName));
            object->SetTransformName(transformName);
        }

        this->AddObject( object, attachTo );

        reader->Delete();
    }
}

int SceneManager::OpenVTKFile( QString filename, SceneObject * attachTo, int fileType, const QString & dataObjectName, const QString & transformName )
{
    int ok = 0, tracks = 0, bin = 0;
    if( QFile::exists( filename ) )
    {
        vtkDataObjectReader * reader = vtkDataObjectReader::New();
        
        reader->SetFileName( filename );
        reader->Update();

        if (reader->GetErrorCode() == vtkErrorCode::NoError)
        {
            bin = reader->GetFileType();
            if (!reader->IsFilePolyData())
            {
                QMessageBox::warning( 0, "Warning", "Currently IBIS supports only DTI tracts.\nOther files may not be displayed correctly.", 1, 0 );
            }
            if (reader->IsFilePolyData())
            {
                vtkPolyDataReader * polyReader = vtkPolyDataReader::New();
                polyReader->SetFileName( filename );
                polyReader->Update();
                PolyDataObject * object = PolyDataObject::New();
                char *header = polyReader->GetHeader();
                if (strstr(header, "tracts"))
                {
                    tracks = 1;
                }
                object->SetPolyData( polyReader->GetOutput() );
                if (tracks)
                {
                    object->SetDTITracks(tracks);
                    object->PrepareDTITracks();
                }
                if (fileType & REFERENCE_DATA_TYPE)
                    fileType &= ~REFERENCE_DATA_TYPE;
                QFileInfo info( filename );
                object->SetDataFileName( info.fileName() );
                object->SetFullFileName(info.absFilePath());
                if (!dataObjectName.isEmpty())
                    object->SetName( dataObjectName );
                else
                    object->SetName( info.fileName() );
                object->SetObjectType(OBJ_TYPE | fileType);
                object->SetManager(this);
                if (fileType & INTRAOP_TYPE)
                {
                    if (UsedTransforms)
                        object->SetTransform(UsedTransforms->GetUserTransform(transformName));
                    object->SetTransformName(transformName);
                }

                this->AddObject( object, attachTo );
                ok = 1;
                polyReader->Delete();
            }
            else if (reader->IsFileRectilinearGrid())
            {
                vtkRectilinearGridReader *rGridReader = vtkRectilinearGridReader::New();
                rGridReader->Delete();
            }
            else if (reader->IsFileStructuredGrid())
            {
                vtkStructuredGridReader *sGridReader = vtkStructuredGridReader::New();
                sGridReader->Delete();
            }
            else if (reader->IsFileUnstructuredGrid())
            {
                vtkUnstructuredGridReader *uGridReader = vtkUnstructuredGridReader::New();
                uGridReader->Delete();
            }
            else if (reader->IsFileStructuredPoints())
            {
                vtkStructuredPointsReader *pReader = vtkStructuredPointsReader::New();
                ImageObject * image = ImageObject::New();
                image->SetImage( (vtkImageData *)pReader->GetOutput() );
                QFileInfo info( filename );
                image->SetDataFileName( info.fileName() );
                if (!dataObjectName.isEmpty())
                    image->SetName( dataObjectName );
                else
                    image->SetName( info.fileName() );
                image->SetObjectType(MINC_TYPE | fileType);
                image->SetManager(this);
        
                if (fileType & INTRAOP_TYPE)
                {
                    if (UsedTransforms)
                        image->SetTransform(UsedTransforms->GetUserTransform(transformName));
                    image->SetTransformName(transformName);
                }
                switch (this->ViewMode)
                {
                case VIEW_SYNCHRONIZED:
                    this->AddObject( image, attachTo );
                    this->AddImageInSynchronizedView(image);
                    break;
                case VIEW_ALL_IN_2D:
                case VIEW_SELECTED:
                default:
                    this->AddObject( image, attachTo );
                    break;
                }
                ok = 1;
                pReader->Delete();
            }
            else
            {
                QMessageBox::warning( 0, "Warning", "Unsupported file format", 1, 0 );
            }
        }
        reader->Delete();
    }
    return ok;
}

void SceneManager::OpenTagFile( QString filename, SceneObject * attachTo, int fileType, const QString & dataObjectName, const QString & transformName )
{
    if( QFile::exists( filename ) )
    {
        vtkTagReader * reader = vtkTagReader::New();
        reader->SetFileName( filename );
        reader->Update();
        vtkPoints *pts = reader->GetVolume( 0 );
        int i, n = pts->GetNumberOfPoints();
        QString displayName;
        if (!(dataObjectName.isNull() || dataObjectName.isEmpty()))
            displayName = dataObjectName;
        else
            displayName = reader->GetVolumeNames()[0];          
        if (fileType & REFERENCE_DATA_TYPE)
            fileType &= ~REFERENCE_DATA_TYPE;
        PointsObject *pointsObject = this->CreatePointsObject(displayName, attachTo);
        if (pointsObject)
        {
            QFileInfo info( filename );
            // name is set in CreatePointsObject, it is displayName, modified if same as existing object name
            pointsObject->SetDataFileName( info.fileName() );
            pointsObject->SetFullFileName(info.absFilePath());
            pointsObject->SetObjectType(POINTS_OBJ_TYPE | fileType);
            if (fileType & INTRAOP_TYPE)
            {
                if (UsedTransforms)
                    pointsObject->SetTransform(UsedTransforms->GetUserTransform(transformName));
                pointsObject->SetTransformName(transformName);
            }
            for (i = 0; i < n; i++)
                pointsObject->AddPoint(reader->GetPointNames()[i], pts->GetPoint(i));
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "#### OpenTagFile: ref. count. =" << pointsObject->GetReferenceCount() << endl);
#endif
        }
        reader->Delete();
    }
}

QWidget * SceneManager::CreateQuadViewWindow( QWidget * parent )
{
    QuadViewWindow * res = 0;
    res = new QuadViewWindow( parent, "Visualizer", Qt::WDestructiveClose );
    res->SetSceneManager( this );
    return res;
}

QWidget * SceneManager::CreateObjectListDialog( QWidget * parent )
{
    ObjectListDialog * res = 0;
    res = new ObjectListDialog( parent, "Object list", Qt::WDestructiveClose );
    res->SetSceneManager( this );
    connect( this, SIGNAL( ObjectAdded() ), res, SLOT( SceneChanged() ) );
    connect( this, SIGNAL( ObjectRemoved() ), res, SLOT( SceneChanged() ) );
    return res;
}

QWidget * SceneManager::CreateSimpleObjectListDialog( QWidget * parent )
{
    SimpleObjectListDialog * res = 0;
    res = new SimpleObjectListDialog( parent, "Object list", Qt::WDestructiveClose );
    res->SetSceneManager( this );
    connect( this, SIGNAL( ObjectAdded() ), res, SLOT( SceneChanged() ) );
    connect( this, SIGNAL( ObjectRemoved() ), res, SLOT( SceneChanged() ) );
    return res;
}

QWidget * SceneManager::CreateSettingsDialog( QWidget * parent )
{
    SceneManagerSettingsDialog * res = new SceneManagerSettingsDialog( parent, "BackgroundSettings", Qt::WDestructiveClose | Qt::WStyle_Tool | Qt::WStyle_StaysOnTop);
    res->SetSceneManager( this );
    return res;
}

View * SceneManager::CreateView( int type, QString name )
{
    View * res = this->GetView( type );

    if( res )
        return res;

    res = View::New();
    res->SetName( name );
    res->SetManager( this );
    res->SetType( type );
    this->Views.push_back( res );
    return res;
}

View * SceneManager::GetView( int type )
{
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        if( (*it)->GetType() == type )
        {
            return (*it);
        }
    }
    return NULL;
}

View * SceneManager::GetView( const QString & name )
{
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        if( (*it)->GetName() == name )
        {
            return (*it);
        }
    }
    return NULL;
}

void SceneManager::SetViewBackgroundColor( vtkReal * color )
{
    this->ViewBackgroundColor[0] = color[0];
    this->ViewBackgroundColor[1] = color[1];
    this->ViewBackgroundColor[2] = color[2];
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        (*it)->SetBackgroundColor( color );
    }
}

void SceneManager::UpdateBackgroundColor( )
{
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        (*it)->SetBackgroundColor( this->ViewBackgroundColor );
    }
}

void SceneManager::AddObject( SceneObject * object, SceneObject * attachTo )
{
    if( object )
    {
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        object->SetLogger(this->GetLogger());
#endif        
        object->SetManager(this);
        if( !attachTo )
        {
            object->Register( this );
            this->Objects.push_back( object );
            this->SetupInAllViews( object );
        }
        else
        {
            attachTo->AddChild( object );
        }
        if (object->GetObjectType() & REFERENCE_DATA_TYPE)
        {
            this->ReferenceDataObject = reinterpret_cast<ImageObject*>(object);
        }
        object->PreDisplaySetup();
        if ((object->GetObjectType() & MINC_TYPE)
            || (object->GetObjectType() & OBJ_TYPE)
            || (object->GetObjectType() & POINTS_OBJ_TYPE))
        {
            this->DataObjectsCount++;
        }
        connect( object, SIGNAL( NameChanged() ), this, SLOT( ObjectNameChangedSlot() ) );
        connect( object, SIGNAL( TransformSet() ), this, SLOT( TransformChangedSlot() ) );
        emit ObjectAdded();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "AddObject - added " << object->GetName() << endl);
#endif        
        if (!(object->GetObjectType() & PICKER_TYPE) &&
            !(object->GetObjectType() & POINTER_TYPE) &&
            !(object->GetObjectType() & USPROBE_TYPE))
        {
            this->SetCurrentObject(object->GetName());
            emit ObjectNameChanged();
        }
    }
}

void SceneManager::AddReconstructedVolumeObject( ImageObject * image, SceneObject * attachTo )
{
//    image->SetTransformFromData( UsedTransforms->GetUserTransform(0) );
    if (UsedTransforms)
        image->SetTransform(UsedTransforms->GetUserTransform(0));
    image->SetTransformName(UsedTransforms->GetTransformName(0));
    this->AddObject( image, attachTo );
    if (this->ViewMode == VIEW_SYNCHRONIZED)
        this->AddImageInSynchronizedView(image);
}

void SceneManager::RemoveObject( SceneObject * object , bool viewChange)
{
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    igstkLogMacro( DEBUG, "#### RemoveObject: " << object->GetName() << ", ref. count. =" << object->GetReferenceCount() << endl);
#endif
    if (object->GetObjectType() & MINC_TYPE)
    {
        ImageObject *image = reinterpret_cast<ImageObject *>(object);
        ImageObject *associatedObject = image->GetAssociatedObject();
        if (associatedObject)
        {
            associatedObject->SetAssociatedObject(0);
            image->SetAssociatedObject(0);
            image->SetObjectReplacedInSynchronizedView(FALSE);
            associatedObject->SetObjectReplacedInSynchronizedView(FALSE);
            if (associatedObject->GetObjectType() & RESLICED_DATA_TYPE)
            {
                this->RemoveObject(associatedObject);
            }
        }
    }
    if (object->GetObjectType() & RESLICED_DATA_TYPE)
    {
        this->ResliceObjectsCount--;
        if (viewChange && this->ResliceObjectsCount == 0 )
            this->SetViewMode(VIEW_SELECTED);
    }
    
    if( this->CurrentObjectName == object->GetName() )
    {
        this->CurrentObjectName = "";
    }
    
    if ((object->GetObjectType() & MINC_TYPE) || (object->GetObjectType() & OBJ_TYPE))
        this->DataObjectsCount--;
    // Release from all views
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        object->Release( (*it) );
    }
    
    this->DisAssociateMincWithObj(object);
    // Detach from parent
    if( object->GetParent() )
    {
        object->DetachFromParent();
    }
    else
    {
        ObjectList::iterator it = find( this->Objects.begin(), this->Objects.end(), object );
        if( it != this->Objects.end() )
        {
            this->Objects.erase( it );
        }
    }
    
    object->disconnect( this, SLOT( ObjectNameChangedSlot() ) );
    if (object == this->PickObj)
    {
        this->GetRidOfCrossHair();
        this->PickObj->Delete();
        this->PickObj = 0;
        object = 0;
    }
    
    if (object)
    {
        if (this->HasToDestroyPicker(object))
        {
            this->RemoveObject(this->PickObj);
        }
    }
    emit ObjectRemoved();
}

SceneObject * SceneManager::GetObject( QString objectName )
{
    ObjectList::iterator it = this->Objects.begin();
    for( ; it != this->Objects.end(); ++it )
    {
        if( (*it)->GetName() == objectName )
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

SceneObject * SceneManager::GetCurrentObject( )
{
    if (!this->CurrentObjectName.isNull() && !this->CurrentObjectName.isEmpty())
        return this->GetObject( this->CurrentObjectName );
    return 0;
}

void SceneManager::SetCurrentObject( QString  name )
{
    if (this->CurrentObjectName == name)
        return;
    this->CurrentObjectName = name;
    SceneObject *current = this->GetCurrentObject();
    if (current->GetObjectType() & MINC_TYPE)
    {
        if (this->ViewMode == VIEW_SELECTED)
            this->ViewCurrentlySelectedObjectOnlyIn2dViews();
        else if (this->ViewMode == VIEW_ALL_IN_2D)
            this->HideShowAllImageObjectsOtherThanCurrentObjectIn2DViews(0, true);
        else
        {
        //This is a hack for blending planes in synchronized view to get rid of black blended in.
            ImageObject *img = reinterpret_cast<ImageObject *>(current);
            int index = img->GetLutIndex();
            if(index == 0)
                img->ChooseColorTable(1);
            else
                img->ChooseColorTable(0);
            img->ChooseColorTable(index);
        }
    }
}

SceneObject * SceneManager::GetUniqueTypedObject(int type)
{
    SceneObject *obj = 0, *firstFound = 0;
    int i, n = 0, k = 0;
    ObjectList::iterator it = this->Objects.begin();
    for( ; it != this->Objects.end(); ++it )
    {
        obj = (*it);
        if ((obj->GetObjectType() & type) && !obj->GetObjectManagedBySystem())
        {
            if (n == 0)
                firstFound = obj;
            n++;
        }
        else
        {
            SceneObject *o;
            k = obj->GetNumberOfChildren();
            for (i = 0; i < k; i++)
            {
                o = obj->GetChild(i);
                if ( n == 0 && (o->GetObjectType()& type) && !o->GetObjectManagedBySystem())
                {
                    if (n == 0)
                        firstFound = o;
                    n++;
                }
            }
        }
    }
    if (n == 1)
        return firstFound;
    return 0;
}

void SceneManager::SetReferenceDataObject( ImageObject *  referenceObject)
{
    if (referenceObject)
    {
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "referenceObject candidate: " << referenceObject->GetName() << " type: " << referenceObject->GetObjectType() << endl);
#endif        
        if (this->ReferenceDataObject)
        {
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
            igstkLogMacro( DEBUG, "ReferenceDataObject exists: " << this->ReferenceDataObject->GetName() << " type: " << this->ReferenceDataObject->GetObjectType() << endl);
#endif            
            this->ReferenceDataObject->SetObjectType(this->ReferenceDataObject->GetObjectType() & ~(int)(REFERENCE_DATA_TYPE));
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
            igstkLogMacro( DEBUG, "ReferenceDataObject type changed: " << this->ReferenceDataObject->GetObjectType() << endl);
#endif            
        }
        this->ReferenceDataObject = referenceObject;
        this->ReferenceDataObject->SetObjectType(this->ReferenceDataObject->GetObjectType() | REFERENCE_DATA_TYPE);
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "Current ReferenceDataObject: " << this->ReferenceDataObject->GetName() << " type: " << this->ReferenceDataObject->GetObjectType() << endl);
#endif        
    }
}
        
ImageObject * SceneManager::GetReferenceDataObject( )
{
    return reinterpret_cast<ImageObject *>(this->GetUniqueTypedObject(REFERENCE_DATA_TYPE));
}


void SceneManager::PreDisplaySetup()
{
    for( ObjectList::iterator it = this->Objects.begin(); it != this->Objects.end(); ++it )
    {
        (*it)->PreDisplaySetup();
    }

    this->ResetAllCameras();
}

void SceneManager::ResetAllCameras()
{
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        (*it)->ResetCamera();
    }
}

void SceneManager::ZoomAllCameras(double factor)
{
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        (*it)->ZoomCamera(factor);
    }
}

void SceneManager::ReleaseAllViews()
{
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        (*it)->ReleaseView();
    }
}

void SceneManager::SetupInAllViews( SceneObject * object )
{
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        object->Setup( (*it), 0 );
    }
}


void SceneManager::SetStandardView(STANDARDVIEW type)
{
    View * v3d = GetView( THREED_VIEW_TYPE );
    if (v3d)
    {
        vtkCamera *camera = v3d->GetRenderer()->GetActiveCamera();
        vtkReal x, y, z;
        double s = camera->GetParallelScale();
        switch (type)
        {
        case SV_FRONT:
            x = 0.0;
            y = 1.0;
            z = 0.0;
            break;
        case SV_BACK:
            x = 0.0;
            y = -1.0;
            z = 0.0;
            break;
        case SV_LEFT:
            x = -1.0;
            y = 0.0;
            z = 0.0;
            break;
        case SV_RIGHT:
            x = 1.0;
            y = 0.0;
            z = 0.0;
            break;
        case SV_TOP:
            x = 0.0;
            y = 0.0;
            z = 1.0;
            break;
        case SV_BOTTOM:
            x = 0.0;
            y = 0.0;
            z = -1.0;
            break;
        default:
            return;
        }
        camera->SetFocalPoint(0.0, 0.0, 0.0);
        camera->SetPosition(x, y, z);
        if (x == 0.0)
        {
            camera->SetViewUp(0.0, 1.0, 0.0);
        }
        else if (y == 0.0)
        {
            camera->SetViewUp(0.0, 0.0, 1.0);
        }
        else
        {
            camera->SetViewUp(1.0, 0.0, 0.0);
        }
        v3d->ResetCamera();
        SceneObject *obj = this->GetObject(PREOP_ROOT_OBJECT_NAME);
        if (obj)
        {
            vtkTransform *tr = obj->GetTransform();
            camera->ApplyTransform(tr);
        }
        v3d->ResetCamera();
        camera->SetParallelScale(s);
        v3d->NotifyNeedRender();
    }
}

void SceneManager::ObjectNameChangedSlot()
{
    emit ObjectNameChanged();
}

SceneObject * SceneManager::FindImportedObject( )
{
    SceneObject *obj, *o;
    int i, k;
    ObjectList::iterator it = this->Objects.begin();
    for(; it != this->Objects.end(); ++it)
    {
        obj = (*it);
        if( obj->GetObjectIsImported() )
            return obj;
        k = obj->GetNumberOfChildren();
        for (i = 0; i < k; i++)
        {
            o = obj->GetChild(i);
            if( o->GetObjectIsImported() )
                return o;
        }
    }
    return 0;
}

void SceneManager::AssociateMincWithObj(ImageObject* iObj, PolyDataObject *pObj)
{
    this->MincAndObj = MincWithObj::New();
    this->MincAndObj->SetImageObj(iObj);
    this->MincAndObj->SetPolyDataObj(pObj);
    iObj->SetMincAndObj(this->MincAndObj);
    pObj->SetMincAndObj(this->MincAndObj);
}

void SceneManager::DisAssociateMincWithObj(SceneObject* obj)
{
    MincWithObj *mobj = obj->GetMincAndObj();
    if (mobj)
    {
        obj->RemoveMincAndObj();
        if (obj->GetObjectType() & MINC_TYPE)
        {
            PolyDataObject *pObj = mobj->GetPolyDataObj();
            pObj->RemoveMincAndObj();
        }
        else
        {
            ImageObject *iObj = mobj->GetImageObj();
            iObj->RemoveMincAndObj();
        }
        this->MincAndObj->Delete();
        this->MincAndObj = 0;
    }
}

PickerObject *SceneManager::MakePickerObject(ImageObject *obj)
{
    SceneObject * attachTo = obj->GetParent();
    this->PickObj = PickerObject::New();
    this->PickObj->SetImageObject(obj);
    this->PickObj->SetPickerType(SURFACE);
    this->PickObj->SetManager(this);
    View * v3d = GetView( THREED_VIEW_TYPE );
    if (v3d)
        this->PickObj->SetInteractorStyle(v3d->GetInteractorStyle());
    if (!this->PickObj->Init())
        return 0;
    this->AddObject( this->PickObj, attachTo );
    return this->PickObj;
}

PickerObject *SceneManager::MakePickerObject(PolyDataObject *obj)
{
    SceneObject * attachTo = obj->GetParent();
    this->PickObj = PickerObject::New();
    this->PickObj->SetPolyDataObj(obj);
    this->PickObj->SetPickerType(OBJ);
    this->PickObj->SetManager(this);
    if (this->MincAndObj && obj == this->MincAndObj->GetPolyDataObj())
    {
        ImageObject *obj = this->MincAndObj->GetImageObj();
        this->PickObj->SetImageObject(obj);
    }
    View * v3d = GetView( THREED_VIEW_TYPE );
    if (v3d)
        this->PickObj->SetInteractorStyle(v3d->GetInteractorStyle());
    if (!this->PickObj->Init())
        return 0;
    this->AddObject( this->PickObj, attachTo );
    return this->PickObj;
}

void SceneManager::DestroyPickerObject()
{
    if (this->PickObj)
    {
        this->RemoveObject(this->PickObj);
    }
}

void SceneManager::GetRidOfCrossHair()
{
    if (this->PickObj)
    {
        this->PickObj->DisablePickerOnPlanes();
    }
}

bool SceneManager::HasToDestroyPicker(SceneObject* object)
{
    if (this->PickObj)
    {
        if (this->PickObj->GetPickerType() == SURFACE && (SceneObject*)this->PickObj->GetImageObj() == object)
        {
            return TRUE;
        }
        else if (this->PickObj->GetPickerType() == OBJ && (SceneObject*)this->PickObj->GetPolyDataObj() == object)
        {
            return TRUE;
        }
    }
    return FALSE;
}
    
void SceneManager::SetInitialCameraPosition(int viewType)
{
    View * v = this->GetView(viewType);
    if (v)
    {
        vtkCamera *camera;
        camera = v->GetRenderer()->GetActiveCamera();
        switch( viewType )
        {
            case SAGITTAL_VIEW_TYPE:
                camera->SetPosition( -1, 0, 0 ); // nose to the left
                camera->SetFocalPoint( 0, 0, 0 );
                camera->SetViewUp( 0, 0, 1 );
                break;
            case CORONAL_VIEW_TYPE:
                camera->SetPosition( 0, 1, 0 ); // left on right
                camera->SetFocalPoint( 0, 0, 0 );
                camera->SetViewUp( 0, 0, 1 );
                break;
            case TRANSVERSE_VIEW_TYPE: // axial
                camera->SetPosition( 0, 0, -1 ); //left on right
                camera->SetFocalPoint( 0, 0, 0 );
                camera->SetViewUp( 0, 1, 0 ); //nose up
                break;
            case THREED_VIEW_TYPE:
                camera->SetPosition( 1, 0, 0 );
                camera->SetFocalPoint( 0, 0, 0 );
                camera->SetViewUp( 0, 0, 1 );
                break;
        }
    }
}

void SceneManager::SetInitialPositionForAllCameras()
{
    vtkCamera *camera;
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        camera = (*it)->GetRenderer()->GetActiveCamera();
        switch( (*it)->GetType() )
        {
        case SAGITTAL_VIEW_TYPE:
            SetInitialCameraPosition(SAGITTAL_VIEW_TYPE);
            break;
        case CORONAL_VIEW_TYPE:
            SetInitialCameraPosition(CORONAL_VIEW_TYPE);
            break;
        case TRANSVERSE_VIEW_TYPE: // axial
            SetInitialCameraPosition(TRANSVERSE_VIEW_TYPE);
            break;
        case THREED_VIEW_TYPE:
            SetInitialCameraPosition(THREED_VIEW_TYPE);
            break;
        }
    }
}

void SceneManager::SetCameraPositionInAllViews( )
{
    vtkReal center[3] = {0.0, 0.0, 0.0};
    vtkCamera *camera;
    if (this->ReferenceDataObject)
    {
        this->ReferenceDataObject->GetImage()->GetCenter(center);
    }
    SceneObject *rootObj = this->GetObject(PREOP_ROOT_OBJECT_NAME);
    vtkTransform *tr = rootObj->GetTransform();
    this->SetInitialPositionForAllCameras();
    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
    {
        camera = (*it)->GetRenderer()->GetActiveCamera();
        camera->ApplyTransform(tr);
        if ( (*it)->GetType() == THREED_VIEW_TYPE)
            camera->SetFocalPoint(center);
    }
    this->ResetAllCameras();
}

void SceneManager::SetCameraPositionForObject(ImageObject *obj)
{
    if (obj)
    {
        for (int i = 0; i < 3; i++)
        {
            this->MoveCamera(obj, i);
        }
    }
}

void SceneManager::MoveCamera(ImageObject *imageObj, int planeIndex)
{
    if (!imageObj || planeIndex < 0 || planeIndex > 3)
        return;
    vtkIndent indent;
    vtkMultiImagePlaneWidget *widget = imageObj->GetPlane(planeIndex);
    if (!widget)
        return;
    int viewType = widget->GetPlaneType();
    View * v = this->GetView(viewType);
    if (v)
    {
        vtkCamera *camera;
        camera = v->GetRenderer()->GetActiveCamera();
        double s = camera->GetParallelScale();
        SetInitialCameraPosition(viewType);
        vtkTransform *tr = vtkTransform::New();
        tr->Identity();
        vtkTransform *currentTransform = imageObj->GetCurrentTransform();
        if (currentTransform)
            tr->DeepCopy(currentTransform);
        vtkMatrix4x4 *resliceAxes = widget->GetResliceAxes();
        if(!resliceAxes)
        {
            tr->Delete();
            return;
        }
        tr->Concatenate(resliceAxes);
        if (viewType == SAGITTAL_VIEW_TYPE)
        {
            tr->RotateX(-90);
            tr->RotateZ(-90);
        }
        else if (viewType == CORONAL_VIEW_TYPE)
        {
            tr->RotateX(90);
            tr->RotateY(90);
        }
        camera->ApplyTransform(tr);
        tr->Delete();
        v->ResetCamera();
        camera->SetParallelScale(s);
        v->GetRenderer()->ResetCameraClippingRange();
        v->NotifyNeedRender();
    }
}

void SceneManager::CreateLookupTable(const QString tableName, vtkReal range[2], vtkLookupTable *lut, bool alpha)
{
    lut->SetTableRange(range[0],range[1]);
    if ( tableName == "Hot" )
    {
        lut->SetHueRange( 0.1, 0.1 );
        lut->SetSaturationRange( 1, 1 );
        lut->SetValueRange( 0.2, 1 );
    }
    else if ( tableName == "Spectrum" )
    {
        lut->SetHueRange( 0, 1 );
        lut->SetSaturationRange( 1, 1 );
        lut->SetValueRange( 0, 1 ); 
        lut->SetRampToSQRT();
    }
    else if ( tableName == "Red" )
    {
        lut->SetHueRange( 0, 0);
        lut->SetSaturationRange( 1, 1 );
        lut->SetValueRange( 0, 1 );
    }
    else if ( tableName == "Green" )
    {
        lut->SetHueRange( 0.3, 0.3 );
        lut->SetSaturationRange( 1, 1 );
        lut->SetValueRange( 0, 1 );
    }
    else if ( tableName == "Blue" )
    {
        lut->SetHueRange( 0.6, 0.6 );
        lut->SetSaturationRange( 1, 1 );
        lut->SetValueRange( 0, 1 ); 
    }
    else if ( tableName == "Pink" )
    {
        lut->SetHueRange( 0.9, 0.9 );
        lut->SetSaturationRange( 1, 1 );
        lut->SetValueRange( 0, 1 ); 
    }
    else if ( tableName == "Yellow" )
    {
        lut->SetHueRange( 0.15, 0.15 );
        lut->SetSaturationRange( 1, 1 );
        lut->SetValueRange( 0, 1 ); 
    }
    else if ( tableName == "Display" )
    {
        lut->SetHueRange( 0, 1 );
        lut->SetSaturationRange( 0, 1 );
        lut->SetValueRange( 0, 1 ); 
    }
    else // Gray
    {
        lut->SetHueRange( 1, 1 );
        lut->SetSaturationRange( 0, 0 );
        lut->SetValueRange( 0, 1 );
    }
    if (alpha)
        lut->SetAlphaRange( 0, 1 ); 
    lut->Build();
}

int SceneManager::GetNumberOfTemplateLokupTables() const
{
    return 7;
}

const QString SceneManager::GetTemplateLookupTableName(int index)
{
    switch (index)
    {
        case 0:
            return "Gray";
        case 1:
            return "Hot";
        case 2:
            return "Spectrum";
        case 3:
            return "Red";
        case 4:
            return "Green";
        case 5:
            return "Blue";
        case 6:
            return "Display";
    }
    return "Gray";
}

void SceneManager::ListObjectsByName(char *header)
{
    if (header)
        cout << "######## " << header << endl;
    cout << "Number of Objects: " << GetNumberOfObjects() << endl;
    SceneObject *obj, *o;
    int i, k, n = 0;
    ObjectList::iterator it = this->Objects.begin();
    for(; it != this->Objects.end(); ++it)
    {
        obj = (*it);
        cout << "# " << ++n << ": " <<  obj->GetName() << endl;
        k = obj->GetNumberOfChildren();
        for (i = 0; i < k; i++)
        {
            o = obj->GetChild(i);
            cout << "# " << ++n << ": " <<  o->GetName() << endl;
        }
    }
}

void SceneManager::SetUserTransforms( UserTransformations * tr )
{
    this->UsedTransforms = tr;
}

UserTransformations * SceneManager::GetUserTransforms( )
{
    return this->UsedTransforms;
}

vtkTransform * SceneManager::GetRegistrationTransform()
{
    if (this->UsedTransforms)
        return this->UsedTransforms->GetRegistrationTransform();
    return 0;
}

bool SceneManager::GetRegistrationFlag()
{
    if (this->UsedTransforms)
        return this->UsedTransforms->GetRegistrationFlag();
    return false;
}

void SceneManager::TransformChangedSlot()
{
    if (this->GetViewMode() == VIEW_SYNCHRONIZED)
    {
        SceneObject *currentObj = this->GetCurrentObject();
        ImageObject *image, *associatedObject; 
        if ((currentObj->GetObjectType() & MINC_TYPE)  &&
            (currentObj->GetObjectType() & INTRAOP_TYPE)) // currently in the program it may only be intra op image, preop are all tramsformed together
        {
            image = reinterpret_cast<ImageObject *>(currentObj);
            associatedObject = image->GetAssociatedObject();
            if (associatedObject)
            {
                image->SetAssociatedObject(0);
                image->SetObjectReplacedInSynchronizedView(FALSE);
                if (associatedObject->GetObjectType() & RESLICED_DATA_TYPE)
                {
                    this->RemoveObject(associatedObject, false);
                }
            }
            this->AddImageInSynchronizedView(image);
            associatedObject = image->GetAssociatedObject();
            associatedObject->MovePlanes(this->ReferenceDataObject);
            this->SetCurrentObject(image->GetName());
        }
        else if (currentObj->GetObjectType() & PREOP_ROOT_TYPE) // currently it must be preop root
        {
            int i, n = currentObj->GetNumberOfChildren();
            SceneObject *childObj;
            for (i = 0; i < n; i++)
            {
                childObj = currentObj->GetChild(i);
                if ( (childObj->GetObjectType()& MINC_TYPE) &&
                     !(childObj->GetObjectType()& REFERENCE_DATA_TYPE) &&
                     !(childObj->GetObjectType()& RESLICED_DATA_TYPE) &&
                     !childObj->GetObjectManagedBySystem())
                {
                    image = reinterpret_cast<ImageObject *>(childObj);
                    associatedObject = image->GetAssociatedObject();
                    if (associatedObject)
                    {
                        image->SetAssociatedObject(0);
                        image->SetObjectReplacedInSynchronizedView(FALSE);
                        if (associatedObject->GetObjectType() & RESLICED_DATA_TYPE)
                        {
                            this->RemoveObject(associatedObject, false);
                        }
                    }
                    this->AddImageInSynchronizedView(image);
                    associatedObject = image->GetAssociatedObject();
                    associatedObject->MovePlanes(this->ReferenceDataObject);
                }
            }
            this->SetCurrentObject(currentObj->GetName());
        }
    }
    emit ObjectTransformChanged();
}

vtkRenderer *SceneManager::GetViewRenderer(int viewType)
{
    View *v = this->GetView(viewType);
    return v->GetRenderer();
}

void SceneManager::MoveAllPlanes(vtkMultiImagePlaneWidget *modelPlane)
{
    SceneObject *obj = 0;
    int i, k = 0;
    int planeIndex = modelPlane->GetPlaneType();
    int type;
    vtkMultiImagePlaneWidget *plane;
    vtkReal origin[3], point1[3], point2[3];
    modelPlane->GetOrigin(origin);
    modelPlane->GetPoint1(point1);
    modelPlane->GetPoint2(point2);
    ObjectList::iterator it = this->Objects.begin();
    for( ; it != this->Objects.end(); ++it )
    {
        obj = (*it);
        type = obj->GetObjectType();
        if ((type & MINC_TYPE) && !obj->GetObjectManagedBySystem())
        {
            plane = ((ImageObject*)obj)->GetPlane(planeIndex);
            if (plane != modelPlane)
            {
                plane->SetOrigin(origin);
                plane->SetPoint1(point1);
                plane->SetPoint2(point2);
                plane->UpdatePlacement();
            }
        }
        else
        {
            SceneObject *o;
            k = obj->GetNumberOfChildren();
            for (i = 0; i < k; i++)
            {
                o = obj->GetChild(i);
                if ( (o->GetObjectType()& MINC_TYPE) && !o->GetObjectManagedBySystem())
                {
                    plane = ((ImageObject*)o)->GetPlane(planeIndex);
                    if (plane != modelPlane)
                    {
                        plane->SetOrigin(origin);
                        plane->SetPoint1(point1);
                        plane->SetPoint2(point2);
                        plane->UpdatePlacement();
                    }
                }
            }
        }
    }
}

void SceneManager::HideShowAllImageObjectsOtherThanCurrentObjectIn2DViews(SceneObject *currentObj, bool show)
{
    SceneObject *obj = 0;
    ImageObject *image;
    int i, k = 0;
    ObjectList::iterator it = this->Objects.begin();
    for( ; it != this->Objects.end(); ++it )
    {
        obj = (*it);
        if (!obj->GetObjectManagedBySystem() && (obj != currentObj) && (obj->GetObjectType() & MINC_TYPE) )
        {
            image = reinterpret_cast<ImageObject*>(obj);
            for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
            {
                if ( (*it)->GetType() != THREED_VIEW_TYPE)
                    image->HideShowIn2DOnly(show, (*it));
            }
        }
        else
        {
            SceneObject *o;
            k = obj->GetNumberOfChildren();
            for (i = 0; i < k; i++)
            {
                o = obj->GetChild(i);
                if (!o->GetObjectManagedBySystem() && (o != currentObj) && (o->GetObjectType() & MINC_TYPE) )
                {
                    image = reinterpret_cast<ImageObject*>(o);
                    for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
                    {
                        if ( (*it)->GetType() != THREED_VIEW_TYPE)
                            image->HideShowIn2DOnly(show, (*it));
                    }
                }
            }
        }
    }
    SceneObject *current = this->GetCurrentObject();
    if (current && (current->GetObjectType() & MINC_TYPE))
        this->SetCameraPositionForObject(reinterpret_cast<ImageObject*>(current));
}

void SceneManager::ObjectSelectedIn3D(vtkMultiImagePlaneWidget *plane)
{
    SceneObject *obj = 0;
    ImageObject *image, *selectedImageObj = 0;
    vtkImageData *selected = reinterpret_cast<vtkImageData*>(plane->GetInput());
    vtkImageData *tmp = 0;
    int i, k = 0;
    bool found = false;
    ObjectList::iterator it = this->Objects.begin();
    for( ; it != this->Objects.end() && !found; ++it )
    {
        obj = (*it);
        if (!obj->GetObjectManagedBySystem() && (obj->GetObjectType() & MINC_TYPE) )
        {
            image = reinterpret_cast<ImageObject*>(obj);
            tmp = image->GetImage();
            if (tmp == selected)
            {
                found = true;
                selectedImageObj = image;
            }
        }
        else
        {
            SceneObject *o;
            k = obj->GetNumberOfChildren();
            for (i = 0; i < k && !found; i++)
            {
                o = obj->GetChild(i);
                if (!o->GetObjectManagedBySystem() && (o->GetObjectType() & MINC_TYPE) )
                {
                    image = reinterpret_cast<ImageObject*>(o);
                    tmp = image->GetImage();
                    if (tmp == selected)
                    {
                        found = true;
                        selectedImageObj = image;
                    }
                }
            }
        }
    }
    if (found)
    {
        this->SetCurrentObject(selectedImageObj->GetName());
        emit ObjectNameChanged();
    }
}

void SceneManager::SetViewMode(int mode)
{
    if (this->ViewMode != mode)
    {
        this->AllImagesResetPlanes();
        int oldMode = this->ViewMode;
        if (this->ViewMode == VIEW_SYNCHRONIZED)
            this->SetSynchronizeOff();
        if (this->ViewMode == VIEW_SELECTED)
            this->HideShowAllImageObjectsOtherThanCurrentObjectIn2DViews(0, true);
        this->ViewMode = mode;
        switch (mode)
        {
        case VIEW_SYNCHRONIZED:
            if (!this->SetSynchronizeOn())
            {
                this->ViewMode = oldMode;
                return;
            }
            break;
        case VIEW_ALL_IN_2D:
            break;
        case VIEW_SELECTED:
        default:
            this->ViewMode = VIEW_SELECTED;
            this->ViewCurrentlySelectedObjectOnlyIn2dViews();
            break;
        }
    }
}

void SceneManager::ViewCurrentlySelectedObjectOnlyIn2dViews()
{
    SceneObject *obj = this->GetCurrentObject();
    if (obj && obj->GetObjectType() & MINC_TYPE)
    {
        ImageObject *image = reinterpret_cast<ImageObject*>(obj);
        this->HideShowAllImageObjectsOtherThanCurrentObjectIn2DViews(obj, false);
        for( ViewList::iterator it = Views.begin(); it != Views.end(); ++it )
        {
            if ( (*it)->GetType() != THREED_VIEW_TYPE)
            {
                image->HideShowIn2DOnly(true, (*it));
                (*it)->GetRenderer()->ResetCameraClippingRange();
            }
        }
        this->SetCameraPositionForObject(image);
    }
}

bool SceneManager::SetSynchronizeOn()
{
    if (!this->ReferenceDataObject)
    {
        if (this->MRIDataObjectsCount > 0)
            QMessageBox::warning( 0, "Warning", "Reference Image not selected.", 1, 0 );
        else
            QMessageBox::warning( 0, "Warning", "Reference Image not loaded.", 1, 0 );
        return false;
    }
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    vtkTransform *refTransform = this->ReferenceDataObject->GetCurrentTransform();
    if (refTransform)
    {
        vtkMatrix4x4 *mat = refTransform->GetMatrix();
        igstkLogMacro( DEBUG, "reference object transform before:" << "\n" <<
        mat->Element[0][0] << ", " << 
        mat->Element[0][1] << ", " <<
        mat->Element[0][2] << ", " << 
        mat->Element[0][3] << "\n" << 
        mat->Element[1][0] << ", " << 
        mat->Element[1][1] << ", " <<
        mat->Element[1][2] << ", " <<
        mat->Element[1][3] << "\n" <<
        mat->Element[2][0] << ", " << 
        mat->Element[2][1] << ", " <<
        mat->Element[2][2] << ", " <<
        mat->Element[2][3] << 
        endl);
    }
#endif    
    if (this->ImageReslicer)
        this->ImageReslicer->Delete();
    this->ImageReslicer = Reslicer::New(); 
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    this->ImageReslicer->SetLogger(this->GetLogger());
#endif    
    vtkImageData *baseForReslice = this->ImageReslicer->MakeBaseForReslice(this->ReferenceDataObject);
    if (!baseForReslice)
    {
        QMessageBox::warning( 0, "Warning", "Setting synchronized view mode failed.", 1, 0 );
        return false;
    }   
    SceneObject *currentObj = this->GetCurrentObject();
    ImageObject *image = 0;
    SceneObject *obj = 0;
    ObjectList::iterator it = this->Objects.begin();
    int i, k = 0;
    for( ; it != this->Objects.end(); ++it )
    {
        obj = (*it);
        if (!obj->GetObjectManagedBySystem()
            && (obj->GetObjectType() & MINC_TYPE)
            && !(obj->GetObjectType() & RESLICED_DATA_TYPE)
            && obj != this->ReferenceDataObject)
        {
            image = reinterpret_cast<ImageObject*>(obj);
            this->AddImageInSynchronizedView(image);
        }
        else
        {
            SceneObject *o;
            k = obj->GetNumberOfChildren();
            for (i = 0; i < k; i++)
            {
                o = obj->GetChild(i);
                if (!o->GetObjectManagedBySystem()
                    && (o->GetObjectType() & MINC_TYPE)
                    && !(o->GetObjectType() & RESLICED_DATA_TYPE)
                    && o != this->ReferenceDataObject )
                {
                    image = reinterpret_cast<ImageObject*>(o);
                    this->AddImageInSynchronizedView(image);
                }
            }
        }
    }
    this->AllImagesResetPlanes();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    refTransform = this->ReferenceDataObject->GetCurrentTransform();
    if (refTransform)
    {
        vtkMatrix4x4 *mat = refTransform->GetMatrix();
        igstkLogMacro( DEBUG, "reference object transform after:" << "\n" <<
        mat->Element[0][0] << ", " << 
        mat->Element[0][1] << ", " <<
        mat->Element[0][2] << ", " << 
        mat->Element[0][3] << "\n" << 
        mat->Element[1][0] << ", " << 
        mat->Element[1][1] << ", " <<
        mat->Element[1][2] << ", " <<
        mat->Element[1][3] << "\n" <<
        mat->Element[2][0] << ", " << 
        mat->Element[2][1] << ", " <<
        mat->Element[2][2] << ", " <<
        mat->Element[2][3] << 
        endl);
    }
#endif    
    this->SetCurrentObject(currentObj->GetName());
    emit ObjectNameChanged();
    return true;
}

void SceneManager::SetSynchronizeOff()
{
    SceneObject *obj = 0;
    ObjectList::iterator it = this->Objects.begin();
    for( ; it != this->Objects.end(); ++it )
    {
        obj = (*it);
        if (!obj->GetObjectManagedBySystem() && (obj->GetObjectType() & RESLICED_DATA_TYPE))
        {
            this->RemoveObject( obj );
        }
        else
        {
            SceneObject *o;
            while ((o = obj->GetChildOfType(RESLICED_DATA_TYPE)))
            {
                if (!o->GetObjectManagedBySystem())
                {
                    this->RemoveObject( o );
                }
            }
        }
    }
    this->AllImagesResetPlanes();
}

void SceneManager::AddImageInSynchronizedView(ImageObject *image)
{
    if (this->ImageReslicer)
    {
        int tableIndex = 0;
        tableIndex = image->GetLutIndex();
        vtkImageData *reslice = vtkImageData::New();
        this->ImageReslicer->ResliceImageData(image, reslice);
        image->SetObjectHidden(TRUE);
        
        
        ImageObject *imageReslice = ImageObject::New();
        imageReslice->SetObjectType(MINC_TYPE | RESLICED_DATA_TYPE);
        imageReslice->SetImage(reslice);
        imageReslice->SetName("reslice_"+image->GetName() );
        this->AddObject( imageReslice, this->GetObject(PREOP_ROOT_OBJECT_NAME) );
        imageReslice->ChooseColorTable(tableIndex); 
        double opacity[3];
        image->GetAllPlanesOpacity(opacity);
        if (opacity[0] == opacity[1] && opacity[0] == opacity[2])
        {
            imageReslice->SetAllPlanesOpacity(opacity[0]);
        }
        else
        {
            imageReslice->SetOnePlaneOpacity(0, opacity[0]);
            imageReslice->SetOnePlaneOpacity(1, opacity[1]);
            imageReslice->SetOnePlaneOpacity(2, opacity[2]);
        }
        double level[2];
        image->GetUsedWindowLevel(level);
        vtkReal range[2], range1[2], scale;
        image->GetImageScalarRange(range);
        imageReslice->GetImageScalarRange(range1);
        scale = (range1[1]-range1[0])/(range[1]-range[0]);
        imageReslice->SetUsedWindowLevel(level[0]*scale, level[1]*scale);
        imageReslice->SetAssociatedObject(image);
        image->SetAssociatedObject(imageReslice);
        image->SetObjectReplacedInSynchronizedView(TRUE);
        this->ResliceObjectsCount++;
        
        image->HideShow();
        imageReslice->SetObjectHidden(FALSE);
        imageReslice->HideShow();
        
        emit ObjectNameChanged();
    }
}

void SceneManager::AllImagesResetPlanes()
{
    SceneObject *obj = 0;
    ImageObject *image = 0;
    ObjectList::iterator it = this->Objects.begin();
    int i, k = 0;
    for( ; it != this->Objects.end(); ++it )
    {
        obj = (*it);
        if (!obj->GetObjectManagedBySystem() &&
            (!(obj->GetObjectType() & RESLICED_DATA_TYPE)) &&
            (!(obj->GetObjectType() & OBJ_TYPE)))
        {
            image = reinterpret_cast<ImageObject*>(obj);
            image->ResetPlanes();
        }
        else
        {
            SceneObject *o;
            k = obj->GetNumberOfChildren();
            for (i = 0; i < k; i++)
            {
                o = obj->GetChild(i);
                if ((o->GetObjectType() & MINC_TYPE) &&
                    !o->GetObjectManagedBySystem() &&
                    (!(o->GetObjectType() & RESLICED_DATA_TYPE)) )
                {
                    image = reinterpret_cast<ImageObject*>(o);
                    image->ResetPlanes();
                }
            }
        }
    }
    if (this->ReferenceDataObject)
        this->ReferenceDataObject->ResetPlanes();
}

void SceneManager::SetAllObjectsOpacity(double opacity)
{
    SceneObject *obj = 0;
    ImageObject *image = 0;
    PolyDataObject *polyObj;
    
    this->GlobalOpacity = opacity;
    ObjectList::iterator it = this->Objects.begin();
    int i, k = 0;
    for( ; it != this->Objects.end(); ++it )
    {
        obj = (*it);
        if (!obj->GetObjectManagedBySystem())
        {
            if (!(obj->GetObjectType() & OBJ_TYPE))
            {
                image = reinterpret_cast<ImageObject*>(obj);
                image->SetAllPlanesOpacity(opacity);
            }
            else
            {
                polyObj = reinterpret_cast<PolyDataObject*>(obj);
                polyObj->SetOpacity(opacity);
            }
        }
        else
        {
            SceneObject *o;
            k = obj->GetNumberOfChildren();
            for (i = 0; i < k; i++)
            {
                o = obj->GetChild(i);
                if (!o->GetObjectManagedBySystem())
                {
                    if (!(o->GetObjectType() & OBJ_TYPE))
                    {
                        image = reinterpret_cast<ImageObject*>(o);
                        image->SetAllPlanesOpacity(opacity);
                    }
                    else
                    {
                        polyObj = reinterpret_cast<PolyDataObject*>(o);
                        polyObj->SetOpacity(opacity);
                    }
                }
            }
        }
    }
}

PointsObject * SceneManager::CreatePointsObject(const QString & name, SceneObject * attachTo)
{
    QString newName(name);
    PointsObject  *obj = 0;
    SceneObject *tmpObj = this->GetObject(name);
    if (tmpObj)
    {
        if (tmpObj->GetObjectType() & POINTS_OBJ_TYPE)
        {
            obj = reinterpret_cast<PointsObject*>(this->GetObject(name));
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
            igstkLogMacro( DEBUG, "#### CreatePointsObject: Object already exists, ref. count. =" << obj->GetReferenceCount() << endl);
#endif
        }
        else
        {
            newName.append("_tag");
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
            igstkLogMacro( DEBUG, "#### CreatePointsObject: Object already exists but, it is not a PointsObject, new name: " << newName << endl);
#endif
        }
    }
    if (!obj)
    {
        obj = PointsObject::New();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
        igstkLogMacro( DEBUG, "#### CreatePointsObject: ref. count. =" << obj->GetReferenceCount() << endl);
#endif
        obj->SetName(newName);
        SceneObject *tmpObj = this->GetCurrentObject();
        if (tmpObj && tmpObj->GetObjectType() & MINC_TYPE)
        {
            ImageObject *iObj = reinterpret_cast<ImageObject*>(tmpObj);
            if (iObj)
                obj->SetImageObject(iObj);
        }
        this->AddObject( obj, attachTo );
        View * v3d = GetView( THREED_VIEW_TYPE );
        if (v3d)
        {
            obj->SetInteractor(v3d->GetInteractor());
        }
    }
    return obj;
}

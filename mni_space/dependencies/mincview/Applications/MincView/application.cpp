#include "application.h"
#include "serializer.h"
#include "scenemanager.h"
#include "polydataobject.h"
#include "vtkAxes.h"
#include <qdir.h>
#include "ignsconfig.h"


ObjectSerializationMacro( ApplicationGeneralSettings );


ApplicationGeneralSettings::ApplicationGeneralSettings()
{
    LastVisitedDirectory = (const char*)(QDir::homeDirPath());
}


void ApplicationGeneralSettings::Serialize( Serializer * serializer )
{
    ::Serialize( serializer, "LastVisitedDirectory", LastVisitedDirectory );
}


Application * Application::m_uniqueInstance = NULL;


int Application::Init()
{
    if( !m_uniqueInstance )
    {
        static Application staticInstance;
        m_uniqueInstance = &staticInstance;
        return 1;
    }
    return 0;
}


Application & Application::GetInstance()
{
    return *m_uniqueInstance;
}

SceneManager * Application::GetSceneManager()
{
    return GetInstance().m_sceneManager;
}

ApplicationGeneralSettings * Application::GetSettings()
{
    return GetInstance().m_settings;
}

void Application::ReadApplicationConfig( const char * filename )
{
    QString name;
    if( !filename )
    {
        name = QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY;
        QDir configDir( name );
        if( !configDir.exists( ) )
        {
            configDir.mkdir( name );
        }
        name = name + "/" + IGNS_MINC_VIEW_CONFIG;
    }
    else 
        name = filename;

    // Try to read previous config
    if( QFile::exists( name ) )
    {
        SerializerReader reader;
        reader.SetFilename( name );
        reader.Start();
        Serialize( &reader, "GeneralSettings", m_settings );
        if (reader.BeginSection("BackgroundColor"))
        {
            vtkReal color[3];
            Serialize( &reader, "Color", color, 3 );
            m_sceneManager->SetViewBackgroundColor(color);
            reader.EndSection();
        }
        reader.Finish();
    }
}


void Application::WriteApplicationConfig( const char * filename )
{
    QString name;
    if( !filename )
    {
        name = QDir::homeDirPath() + "/" + IGNS_CONFIGURATION_SUBDIRECTORY;
        QDir configDir( name );
        if( !configDir.exists( ) )
        {
            configDir.mkdir( name );
        }
        name = name + "/" + IGNS_MINC_VIEW_CONFIG;
    }
    else 
        name = filename;
    
    // try to write new config
    SerializerWriter writer;
    writer.SetFilename( name );
    writer.Start();
    ::Serialize( &writer, "GeneralSettings", m_settings );
    writer.BeginSection("BackgroundColor");
    vtkReal *color =m_sceneManager->GetViewBackgroundColor();
    ::Serialize( &writer, "Color", color, 3 );
    writer.EndSection();
    writer.Finish();
}


Application::Application()
{
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    // prepare logger
    m_logger = LoggerType::New();
    m_logOutput = LogOutputType::New();
    m_logFile.open("logMincView.txt");
    if( !m_logFile.fail() )
    {
        m_logOutput->SetStream( m_logFile );
    }
    else
    {
        std::cerr << "Problem opening Log file, using cerr instead "
        << std::endl;
        m_logOutput->SetStream( std::cerr );
    }
    m_logger->AddLogOutput( m_logOutput );
    m_logger->SetPriorityLevel( LoggerType::DEBUG );

    igstkLogMacro2( m_logger, DEBUG, "MincView log\n");
#endif    
    // Create the object that will manage the 3D scene in the visualizer
    m_settings = new ApplicationGeneralSettings;
    m_sceneManager = SceneManager::New();
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    m_sceneManager->SetLogger(m_logger);
#endif    
    
    ReadApplicationConfig( 0 );

    
    // and add axes at its origin
    vtkAxes * axesSource = vtkAxes::New();
    axesSource->SetScaleFactor( 150 );
    axesSource->SymmetricOn();
    axesSource->ComputeNormalsOff();
    PolyDataObject * axesObject = PolyDataObject::New();
    axesObject->SetPolyData( axesSource->GetOutput() );
    axesObject->SetName("Axes");
    axesObject->SetObjectType(SYSTEM_TYPE);
    m_sceneManager->AddObject( axesObject );
    axesSource->Delete();
    axesObject->Delete();
}


Application::~Application()
{
    // Write the last application configuration
    GetInstance().WriteApplicationConfig( 0 );

    m_sceneManager->Delete();
    delete m_settings;
}

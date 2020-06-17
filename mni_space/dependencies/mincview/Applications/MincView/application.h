#ifndef __Application_h_
#define __Application_h_

#include "serializer.h"

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
#include "itkStdStreamLogOutput.h"
#include "igstkObject.h"
#include "igstkMacros.h"
#endif
// forward declarations
class SceneManager;

// Structure that holds general settings for this application.
struct ApplicationGeneralSettings
{
    ApplicationGeneralSettings();
    void Serialize( Serializer * serializer );
    
    QString LastVisitedDirectory;
};

ObjectSerializationHeaderMacro( ApplicationGeneralSettings );

//===============================================================================
// Main application class (Singleton)
//===============================================================================
class Application
{

public:

    ~Application();
    
#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    typedef igstk::Object::LoggerType    LoggerType;
    typedef itk::StdStreamLogOutput      LogOutputType;
#endif    
    static int Init();
    static Application & GetInstance();
    static SceneManager * GetSceneManager();
    static ApplicationGeneralSettings * GetSettings();

    void ReadApplicationConfig( const char * filename );
    void WriteApplicationConfig( const char * filename );

    void OpenMincFile( const char * filename );
    void OpenObjFile( const char * filename );

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    LoggerType * GetLogger() {return m_logger;}
#endif    
private:

    Application();

    static Application   * m_uniqueInstance;

    SceneManager    * m_sceneManager;
    ApplicationGeneralSettings * m_settings;

#if defined(USE_IGSTK_LOG) && !defined(WIN32) && !defined(_WIN32)
    LoggerType::Pointer         m_logger;
    LogOutputType::Pointer      m_logOutput;
    std::ofstream               m_logFile;
#endif    
};


#endif

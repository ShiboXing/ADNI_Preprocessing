#include <qapplication.h>
#include "application.h"
#include "mainwindow.h"
#include "scenemanager.h"

int main( int argc, char** argv )
{
    // Initialize application core
    Application::Init();
    
    // Read files on the command line if there are
    if( argc > 1 )
    {
        for( int i = 1; i < argc; i++ )
        {
            Application::GetSceneManager()->OpenMincFile( argv[i] );
        }
    }

    // Create GUI app and main window
    QApplication a( argc, argv );
    MainWindow * mw = new MainWindow( 0, "MincView", Qt::WDestructiveClose );
    mw->show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    // Start main loop
    return a.exec();
}


#ifndef __QtUtilities_h_
#define __QtUtilities_h_

#include <qapplication.h>
#include <qwidgetlist.h>

inline QWidget * GetWidgetByName( QString widgetName )
{
    QWidgetList  * list = QApplication::allWidgets();
    QWidgetListIt it( *list );
    QWidget * w = 0;
    while ( ( w = it.current() ) != 0 ) 
    {
        ++it;
        if( w->name() == widgetName )
            break;
    }
    delete list;
    
    return w;
}

#endif
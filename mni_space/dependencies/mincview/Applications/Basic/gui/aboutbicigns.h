#ifndef TAG_ABOUTBICIGNS_H
#define TAG_ABOUTBICIGNS_H

#include "aboutbicignsbase.h"
#include <qstring.h>


class AboutBICIgns : public AboutBICIgnsBase
{
Q_OBJECT

public:

    AboutBICIgns( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    ~AboutBICIgns();
        
    void Initialize(QString appName);

};


#endif  // TAG_ABOUTBICIGNS_H

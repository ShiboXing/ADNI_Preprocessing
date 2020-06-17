
#include <qlabel.h>
#include <qdatetime.h>
#include "version.h"
#include "aboutbicigns.h"

AboutBICIgns::AboutBICIgns(QWidget * parent, const char * name, WFlags f) :
AboutBICIgnsBase(parent, name, f)
{
}

AboutBICIgns::~AboutBICIgns()
{
}

void AboutBICIgns::Initialize(QString appName)
{
    if (appName.isNull() || appName.isEmpty())
        applicationLabel->setText("Interactive Brain Imaging System");
    else    
        applicationLabel->setText(appName);
    versionLabel->setText(IGNS_VERSION);
    dateLabel->setText(IGNS_BUILD_DATE);
}



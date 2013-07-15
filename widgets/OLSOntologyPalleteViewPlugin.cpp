#include "OLSOntologyPalleteView.h"
#include "OLSOntologyPalleteViewPlugin.h"

#include <QtPlugin>

OLSOntologyPalleteViewPlugin::OLSOntologyPalleteViewPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void OLSOntologyPalleteViewPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;
    
    // Add extension registrations, etc. here
    
    m_initialized = true;
}

bool OLSOntologyPalleteViewPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *OLSOntologyPalleteViewPlugin::createWidget(QWidget *parent)
{
    return new OLSOntologyPalleteView(parent);
}

QString OLSOntologyPalleteViewPlugin::name() const
{
    return QLatin1String("OLSOntologyPalleteView");
}

QString OLSOntologyPalleteViewPlugin::group() const
{
    return QLatin1String("");
}

QIcon OLSOntologyPalleteViewPlugin::icon() const
{
    return QIcon();
}

QString OLSOntologyPalleteViewPlugin::toolTip() const
{
    return QLatin1String("");
}

QString OLSOntologyPalleteViewPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool OLSOntologyPalleteViewPlugin::isContainer() const
{
    return false;
}

QString OLSOntologyPalleteViewPlugin::domXml() const
{
    return QLatin1String("<widget class=\"OLSOntologyPalleteView\" name=\"OLSOntologyPalleteView\">\n</widget>\n");
}

QString OLSOntologyPalleteViewPlugin::includeFile() const
{
    return QLatin1String("OLSOntologyPalleteView.h");
}


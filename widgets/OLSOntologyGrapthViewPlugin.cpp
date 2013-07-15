#include "OLSOntologyGrapthView.h"
#include "OLSOntologyGrapthViewPlugin.h"

#include <QtPlugin>

OLSOntologyGrapthViewPlugin::OLSOntologyGrapthViewPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void OLSOntologyGrapthViewPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;
    
    // Add extension registrations, etc. here
    
    m_initialized = true;
}

bool OLSOntologyGrapthViewPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *OLSOntologyGrapthViewPlugin::createWidget(QWidget *parent)
{
    return new OLSOntologyGrapthView(parent);
}

QString OLSOntologyGrapthViewPlugin::name() const
{
    return QLatin1String("OLSOntologyGrapthView");
}

QString OLSOntologyGrapthViewPlugin::group() const
{
    return QLatin1String("");
}

QIcon OLSOntologyGrapthViewPlugin::icon() const
{
    return QIcon();
}

QString OLSOntologyGrapthViewPlugin::toolTip() const
{
    return QLatin1String("");
}

QString OLSOntologyGrapthViewPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool OLSOntologyGrapthViewPlugin::isContainer() const
{
    return false;
}

QString OLSOntologyGrapthViewPlugin::domXml() const
{
    return QLatin1String("<widget class=\"OLSOntologyGrapthView\" name=\"OLSOntologyGrapthView\">\n</widget>\n");
}

QString OLSOntologyGrapthViewPlugin::includeFile() const
{
    return QLatin1String("OLSOntologyGrapthView.h");
}


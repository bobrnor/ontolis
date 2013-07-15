#include "OLSOntologyTreeView.h"
#include "OLSOntologyTreeViewPlugin.h"

#include <QtPlugin>

OLSOntologyTreeViewPlugin::OLSOntologyTreeViewPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void OLSOntologyTreeViewPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;
    
    // Add extension registrations, etc. here
    
    m_initialized = true;
}

bool OLSOntologyTreeViewPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *OLSOntologyTreeViewPlugin::createWidget(QWidget *parent)
{
    return new OLSOntologyTreeView(parent);
}

QString OLSOntologyTreeViewPlugin::name() const
{
    return QLatin1String("OLSOntologyTreeView");
}

QString OLSOntologyTreeViewPlugin::group() const
{
    return QLatin1String("");
}

QIcon OLSOntologyTreeViewPlugin::icon() const
{
    return QIcon();
}

QString OLSOntologyTreeViewPlugin::toolTip() const
{
    return QLatin1String("");
}

QString OLSOntologyTreeViewPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool OLSOntologyTreeViewPlugin::isContainer() const
{
    return false;
}

QString OLSOntologyTreeViewPlugin::domXml() const
{
    return QLatin1String("<widget class=\"OLSOntologyTreeView\" name=\"OLSOntologyTreeView\">\n</widget>\n");
}

QString OLSOntologyTreeViewPlugin::includeFile() const
{
    return QLatin1String("OLSOntologyTreeView.h");
}


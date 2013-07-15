#include "OLSOntologyGrapthViewPlugin.h"
#include "OLSOntologyTreeViewPlugin.h"
#include "OLSOntologyPalleteViewPlugin.h"
#include "OLSWidgets.h"

OLSWidgets::OLSWidgets(QObject *parent)
    : QObject(parent)
{
    m_widgets.append(new OLSOntologyGrapthViewPlugin(this));
    m_widgets.append(new OLSOntologyTreeViewPlugin(this));
    m_widgets.append(new OLSOntologyPalleteViewPlugin(this));
    
}

QList<QDesignerCustomWidgetInterface*> OLSWidgets::customWidgets() const
{
    return m_widgets;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(olswidgetsplugin, OLSWidgets)
#endif // QT_VERSION < 0x050000

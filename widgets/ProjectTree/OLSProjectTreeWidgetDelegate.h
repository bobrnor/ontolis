#ifndef OLSPROJECTTREEWIDGETDELEGATE_H
#define OLSPROJECTTREEWIDGETDELEGATE_H

#include <QMap>
#include <QVariant>

class OLSProjectTreeWidgetDelegate {

  public:
    virtual bool processDrop(QTreeWidgetItem *parentItem, const QMap<int, QVariant> &dropData) = 0;
};

#endif // OLSPROJECTTREEWIDGETDELEGATE_H

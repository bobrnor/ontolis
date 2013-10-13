#ifndef OLSPROJECTTREEWIDGET_H
#define OLSPROJECTTREEWIDGET_H

#include <QTreeWidget>

#include "OLSProjectTreeWidgetDelegate.h"

class OLSProjectTreeWidget : public QTreeWidget {

    Q_OBJECT

  private:
    OLSProjectTreeWidgetDelegate *m_delegate;

  protected:
    bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);

  public:
    explicit OLSProjectTreeWidget(QWidget *parent = 0);

    void setDelegate(OLSProjectTreeWidgetDelegate *delegate);
    OLSProjectTreeWidgetDelegate *delegate() const;
};

#endif // OLSPROJECTTREEWIDGET_H

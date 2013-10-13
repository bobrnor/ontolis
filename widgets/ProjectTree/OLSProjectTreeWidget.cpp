#include "OLSProjectTreeWidget.h"

#include <QDebug>
#include <QDropEvent>
#include <QMimeData>

OLSProjectTreeWidget::OLSProjectTreeWidget(QWidget *parent) :  QTreeWidget(parent) {
}

bool OLSProjectTreeWidget::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action) {

  QByteArray itemData = data->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&itemData, QIODevice::ReadOnly);

  // magic
  int r, c;
  QMap<int, QVariant> v;
  stream >> r >> c >> v;

  qDebug() << v;

  if (parent != NULL && m_delegate != NULL) {
    bool result = m_delegate->processDrop(parent, v);
    if (result) {
      return QTreeWidget::dropMimeData(parent, index, data, action);
    }
  }
  return false;
}

void OLSProjectTreeWidget::setDelegate(OLSProjectTreeWidgetDelegate *delegate) {

  m_delegate = delegate;
}

OLSProjectTreeWidgetDelegate *OLSProjectTreeWidget::delegate() const {

  return m_delegate;
}

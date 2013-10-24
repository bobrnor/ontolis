#include "OLSOntologyPalleteWidget.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QMimeData>

OLSOntologyPalleteWidget::OLSOntologyPalleteWidget(QWidget *parent) : QListWidget(parent) {

  setIconSize(QSize(110, 110));
  setSelectionMode(QAbstractItemView::SingleSelection);
  setViewMode(QListView::IconMode);

  QStringList names;
  QStringList shapeNames;
  QStringList picNames;
  names << "Rect" << "Rounded rect";
  shapeNames << "rect" << "rounded_rect";
  picNames << "rect.png" << "rounded-rect.png";

  for (int i = 0; i < names.size(); ++i) {
    QString name = names[i];
    QString shapeName = shapeNames[i];
    QString picName = picNames[i];

    QVariantMap guiAttrs;
    QVariantMap attrs;
    attrs["shape_name"] = shapeName;
    guiAttrs["gui-attributes"] = attrs;

    QListWidgetItem *item = new QListWidgetItem(name, this);
    item->setIcon(QPixmap("/Users/bobrnor/Dropbox/PSU/Projects/OntoEditor/pics/" + picName));
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    item->setData(Qt::UserRole, guiAttrs);
  }
}

QStringList OLSOntologyPalleteWidget::mimeTypes() const {

  return QStringList("application/node-attrs");
}

QMimeData *OLSOntologyPalleteWidget::mimeData(const QList<QListWidgetItem *> items) const {

  QMimeData *data = QListWidget::mimeData(items);
  QVariant attrs = items[0]->data(Qt::UserRole);
  data->setData("application/node-attrs", QJsonDocument::fromVariant(attrs).toJson());
  return data;
}

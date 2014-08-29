#ifndef OLSONTOLOGYGRAPHNODEITEM_H
#define OLSONTOLOGYGRAPHNODEITEM_H

#include <QGraphicsRectItem>

#include "OLSOntologyGraphItemDataKey.h"
#include "OLSOntologyGraphItemType.h"
#include "OLSOntologyGraphElement.h"

class OLSOntologyGraphRelationItem;

class OLSOntologyGraphNodeItem : public QObject, public QGraphicsRectItem, public OLSOntologyGraphElement {
    Q_OBJECT

  private:
    QList<OLSOntologyGraphRelationItem *> m_relations;
    QColor m_backgroundColor;
    QColor m_textColor;
    QString m_shapeName;
    QImage m_image;

  public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void attributesChanged();

  public:
    explicit OLSOntologyGraphNodeItem(QGraphicsItem *parent = 0);
    ~OLSOntologyGraphNodeItem();

    void appendRelation(OLSOntologyGraphRelationItem *relation);
    void removeRelation(OLSOntologyGraphRelationItem *relation);

    void removeAllRelations();

    void setName(const QString &name);

    QString attributesAsText() const;
    QVariantMap attributes() const;
    void setAttributesFromData(const QByteArray &data);
    void setAttributes(const QVariantMap &attributes);

  signals:
    void nodeItemPositionChangedSignal(long id, const QPointF &newPosition);
};

#endif // OLSONTOLOGYGRAPHNODEITEM_H

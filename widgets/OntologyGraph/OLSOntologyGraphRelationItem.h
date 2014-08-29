#ifndef OLSONTOLOGYGRAPHRELATIONITEM_H
#define OLSONTOLOGYGRAPHRELATIONITEM_H

#include <QGraphicsLineItem>

#include "OLSOntologyGraphItemDataKey.h"
#include "OLSOntologyGraphItemType.h"
#include "OLSOntologyGraphElement.h"

class OLSOntologyGraphNodeItem;

class OLSOntologyGraphRelationItem : public QGraphicsLineItem, public OLSOntologyGraphElement {
  private:
    OLSOntologyGraphNodeItem *m_sourceNode;
    OLSOntologyGraphNodeItem *m_destinationNode;

    double m_width;
    double m_lenght;
    QVector<qreal> m_dashPattern;

  public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    void attributesChanged();

  public:
    explicit OLSOntologyGraphRelationItem(QGraphicsItem *parent = 0);
    ~OLSOntologyGraphRelationItem();

    QRectF boundingRect() const;

    virtual void adjust();

    virtual void removeFromNodes();

    virtual void setSourceNode(OLSOntologyGraphNodeItem *node);
    virtual OLSOntologyGraphNodeItem *sourceNode() const;

    virtual void setDestinationNode(OLSOntologyGraphNodeItem *node);
    virtual OLSOntologyGraphNodeItem *destinationNode() const;

    QString attributesAsText() const;
    QVariantMap attributes() const;
    void setAttributesFromData(const QByteArray &data);
    void setAttributes(const QVariantMap &attributes);
};

#endif // OLSONTOLOGYGRAPHRELATIONITEM_H

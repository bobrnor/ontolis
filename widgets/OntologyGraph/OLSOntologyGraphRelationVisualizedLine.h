#ifndef OLSONTOLOGYGRAPHRELATIONVISUALIZEDLINE_H
#define OLSONTOLOGYGRAPHRELATIONVISUALIZEDLINE_H

#include "OLSOntologyGraphRelationItem.h"

class OLSOntologyGraphNodeItem;

class OLSOntologyGraphRelationVisualizedLine : public OLSOntologyGraphRelationItem {
  private:
    OLSOntologyGraphNodeItem *m_sourceNode;
    QPointF m_endPoint;
    bool m_endPointEmpty;

  protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  public:
    explicit OLSOntologyGraphRelationVisualizedLine(QGraphicsItem *parent = 0);

    void adjust();
    void removeFromNodes();

    void setSourceNode(OLSOntologyGraphNodeItem *node);
    OLSOntologyGraphNodeItem *sourceNode() const;

    void setEndPoint(QPointF point);
    QPointF endPoint() const;    
};

#endif // OLSONTOLOGYGRAPHRELATIONVISUALIZEDLINE_H

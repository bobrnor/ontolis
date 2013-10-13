#include "OLSOntologyGraphRelationVisualizedLine.h"

#include <QPen>
#include <QPainter>
#include <QDebug>

#include "OLSOntologyGraphNodeItem.h"

OLSOntologyGraphRelationVisualizedLine::OLSOntologyGraphRelationVisualizedLine(QGraphicsItem *parent) :
  OLSOntologyGraphRelationItem(parent), m_sourceNode(NULL) {

  setData(OLSOntologyGraphItemDataKey::kType, OLSOntologyGraphItemType::kOther);
  setPen(QPen(Qt::black, 2));
  m_endPointEmpty = true;
}

void OLSOntologyGraphRelationVisualizedLine::setSourceNode(OLSOntologyGraphNodeItem *node) {

  m_sourceNode = node;
  if (m_sourceNode != NULL) {
    m_sourceNode->appendRelation(this);
  }
  adjust();
}

OLSOntologyGraphNodeItem *OLSOntologyGraphRelationVisualizedLine::sourceNode() const {

  return m_sourceNode;
}

void OLSOntologyGraphRelationVisualizedLine::setEndPoint(QPointF point) {

  m_endPoint = point;
  m_endPointEmpty = false;
  adjust();
}

QPointF OLSOntologyGraphRelationVisualizedLine::endPoint() const {

  return m_endPoint;
}

void OLSOntologyGraphRelationVisualizedLine::adjust() {

  if (m_sourceNode != NULL && !m_endPointEmpty) {
    setLine(QLineF(m_sourceNode->scenePos(), m_endPoint));
  }
}

void OLSOntologyGraphRelationVisualizedLine::removeFromNodes() {

  if (m_sourceNode != NULL) {
    m_sourceNode->removeRelation(this);
    m_sourceNode = NULL;
  }
}

void OLSOntologyGraphRelationVisualizedLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

  QPen pen = this->pen();

  QVector<qreal> dashPattern;
  dashPattern.append(2.0);
  dashPattern.append(2.0);
  pen.setDashPattern(dashPattern);
  pen.setColor(Qt::lightGray);

  painter->setPen(pen);
  painter->drawLine(line());
}

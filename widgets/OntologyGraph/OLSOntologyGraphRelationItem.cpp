#include "OLSOntologyGraphRelationItem.h"

#include <QPen>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsScene>

#include <math.h>

#include "OLSOntologyGraphNodeItem.h"

OLSOntologyGraphRelationItem::OLSOntologyGraphRelationItem(QGraphicsItem *parent) :
  QGraphicsLineItem(parent), OLSOntologyGraphElement(), m_sourceNode(NULL), m_destinationNode(NULL) {

  setPen(QPen(Qt::black, 1));
  setZValue(-1);
  m_width = 1.0;
  m_lenght = MAXFLOAT;

  setFlag(ItemIsSelectable);

  setData(OLSOntologyGraphItemDataKey::kType, OLSOntologyGraphItemType::kRelation);
}

OLSOntologyGraphRelationItem::~OLSOntologyGraphRelationItem() {

  qDebug() << "Relation destructor";
}

QRectF OLSOntologyGraphRelationItem::boundingRect() const {

  QRectF boundingRect = QGraphicsLineItem::boundingRect();

  QFontMetrics metrics = QFontMetrics(QFont());
  QRect textRect = metrics.boundingRect(m_name);

  boundingRect.setX(boundingRect.x() - 10.0);
  boundingRect.setY(boundingRect.y() - 10.0);
  boundingRect.setWidth(boundingRect.width() + 10.0);
  boundingRect.setHeight(boundingRect.height() + 10.0);

  if (boundingRect.width() / 2 < textRect.width()) {
    boundingRect.setWidth(textRect.width() * 2);
  }

  if (boundingRect.height() / 2 < textRect.height()) {
    boundingRect.setHeight(textRect.height() * 2);
  }

  return boundingRect;
}

void OLSOntologyGraphRelationItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {

  if (event->button() != Qt::LeftButton) {
    event->accept();
    return;
  }
  QGraphicsLineItem::mousePressEvent(event);
}

void OLSOntologyGraphRelationItem::setSourceNode(OLSOntologyGraphNodeItem *node) {

  m_sourceNode = node;
  if (m_sourceNode != NULL) {
    m_sourceNode->appendRelation(this);
  }
  adjust();
}

OLSOntologyGraphNodeItem *OLSOntologyGraphRelationItem::sourceNode() const {

  return m_sourceNode;
}

void OLSOntologyGraphRelationItem::setDestinationNode(OLSOntologyGraphNodeItem *node) {

  m_destinationNode = node;
  if (m_destinationNode != NULL) {
    m_destinationNode->appendRelation(this);
  }
  adjust();
}

OLSOntologyGraphNodeItem *OLSOntologyGraphRelationItem::destinationNode() const {

  return m_destinationNode;
}

void OLSOntologyGraphRelationItem::removeFromNodes() {

  if (m_sourceNode != NULL) {
    m_sourceNode->removeRelation(this);
    m_sourceNode = NULL;
  }

  if (m_destinationNode != NULL) {
    m_destinationNode->removeRelation(this);
    m_destinationNode = NULL;
  }
}

void OLSOntologyGraphRelationItem::adjust() {

  if (m_sourceNode != NULL && m_destinationNode != NULL) {
      QPointF point = m_destinationNode->pos();
      if (abs(m_destinationNode->pos().x() - m_sourceNode->pos().x()) > m_lenght) {
          double diff = abs(m_destinationNode->pos().x() - m_sourceNode->pos().x()) - m_lenght;
          if (m_destinationNode->pos().x() > m_sourceNode->pos().x()) {
              point.setX(m_destinationNode->pos().x() - diff);
          }
          else {
              point.setX(m_destinationNode->pos().x() + diff);
          }
      }
      if (abs(m_destinationNode->pos().y() - m_sourceNode->pos().y()) > m_lenght) {
          double diff = abs(m_destinationNode->pos().y() - m_sourceNode->pos().y()) - m_lenght;
          if (m_destinationNode->pos().y() > m_sourceNode->pos().y()) {
              point.setY(m_destinationNode->pos().y() - diff);
          }
          else {
              point.setY(m_destinationNode->pos().y() + diff);
          }
      }

      if (point != m_destinationNode->pos()) {
          qDebug() << point;
          m_destinationNode->setPos(point);
      }
      else {
          setLine(QLineF(m_sourceNode->pos(), m_destinationNode->pos()));
      }
  }

  if (this->scene() != NULL) {
    this->scene()->invalidate();
  }
}

void OLSOntologyGraphRelationItem::attributesChanged() {

  OLSOntologyRelationData *data = relatedDataController()->getRelationById(m_id);
  relatedDataController()->relationAttributesChanged(m_id, data->attributes);

  m_dashPattern.clear();

  QVariantMap category = m_dataController->getCategory(data->name);

  if (data->attributes.keys().contains("gui-attributes") || !category.isEmpty()) {
    QVariantMap guiAttributes = data->attributes.value("gui-attributes").toMap();

    if (!category.isEmpty()) {
        guiAttributes = category;
    }

    if (guiAttributes.contains("line_width")) {
      QString lineWidth = guiAttributes["line_width"].toString();
      m_width = lineWidth.toDouble();
    }

    if (guiAttributes.contains("line_length")) {
      QString lineWidth = guiAttributes["line_length"].toString();
      m_lenght = lineWidth.toDouble();
      adjust();
    }

    if (guiAttributes.contains("dash")) {
      QString dash = guiAttributes["dash"].toString();
      if (dash == "solid") {
//        m_dashPattern.append(1.0);
      }
      else if (dash == "dash") {
        m_dashPattern.append(3.0);
        m_dashPattern.append(2.0);
      }
      else if (dash == "dot") {
        m_dashPattern.append(1.0);
        m_dashPattern.append(2.0);
      }
    }
  }
}

void OLSOntologyGraphRelationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

    if (m_width > 0) {
        qreal Pi = 3.14;
        QPointF sourcePos(line().p1());
        QPointF destinationPos(line().p2());
        QPointF middlePos = (sourcePos + destinationPos) / 2.0;

        double angle = ::acos(line().dx() / line().length());
        if (line().dy() >= 0)
          angle = (Pi * 2) - angle;
        QPointF arrowP1 = middlePos - QPointF(sin(angle + Pi / 3) * 10,
                                                cos(angle + Pi / 3) * 10);
        QPointF arrowP2 = middlePos - QPointF(sin(angle + Pi - Pi / 3) * 10,
                                                cos(angle + Pi - Pi / 3) * 10);

        QPainterPath path;
        path.moveTo(sourcePos);
        path.lineTo(destinationPos);
        path.moveTo(middlePos);
        path.lineTo(arrowP1);
        path.moveTo(middlePos);
        path.lineTo(arrowP2);

        QPen pen = this->pen();
        pen.setWidthF(m_width);
        pen.setDashPattern(m_dashPattern);

        if (isSelected()) {
          QVector<qreal> dashPattern;
          dashPattern.append(2.0);
          dashPattern.append(2.0);
          pen.setDashPattern(dashPattern);
          pen.setColor(Qt::blue);
        }
        else if (m_dataController->isRelationChanged(m_id)) {
      //    QVector<qreal> dashPattern;
      //    dashPattern.append(2.0);
      //    dashPattern.append(2.0);
      //    pen.setDashPattern(dashPattern);
          pen.setColor(Qt::red);
        }

        painter->setPen(pen);
        painter->drawPath(path);
        painter->drawText(middlePos, m_name);
    }
}

QString OLSOntologyGraphRelationItem::attributesAsText() const {

  OLSOntologyRelationData *relationData = relatedDataController()->getRelationById(m_id);
  return relationData->attributesAsText();
}

QVariantMap OLSOntologyGraphRelationItem::attributes() const {

  OLSOntologyRelationData *relationData = relatedDataController()->getRelationById(m_id);
  return relationData->attributes;
}

void OLSOntologyGraphRelationItem::setAttributesFromData(const QByteArray &data) {

  OLSOntologyRelationData *relationData = relatedDataController()->getRelationById(m_id);
  relationData->setAttributesFromData(data);
  attributesChanged();
}

void OLSOntologyGraphRelationItem::setAttributes(const QVariantMap &attributes) {

  OLSOntologyRelationData *relationData = relatedDataController()->getRelationById(m_id);
  relationData->attributes = attributes;
  attributesChanged();
}

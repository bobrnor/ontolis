#include "OLSOntologyGraphView.h"

OLSOntologyGraphView::OLSOntologyGraphView(QWidget *parent)
  : QGraphicsView(parent) {

}

OLSOntologyGraphView::OLSOntologyGraphView(QGraphicsScene *scene, QWidget *parent)
  : QGraphicsView(scene, parent) {

}

void OLSOntologyGraphView::mouseMoveEvent(QMouseEvent *event) {

  if (hasMouseTracking()) {
    emit mousePositionChangedSignal(event->pos());
  }
  QGraphicsView::mouseMoveEvent(event);
}

void OLSOntologyGraphView::dragEnterEvent(QDragEnterEvent *event) {

}

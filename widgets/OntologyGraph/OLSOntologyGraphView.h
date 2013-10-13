#ifndef OLSONTOLOGYGRAPHICSVIEW_H
#define OLSONTOLOGYGRAPHICSVIEW_H

#include <QMouseEvent>
#include <QGraphicsView>

class OLSOntologyGraphView : public QGraphicsView {
    Q_OBJECT

  protected:
    void mouseMoveEvent(QMouseEvent *event);

//    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);

  public:
    explicit OLSOntologyGraphView(QWidget *parent = 0);
    explicit OLSOntologyGraphView(QGraphicsScene *scene, QWidget *parent = 0);

  signals:
    void mousePositionChangedSignal(const QPoint &pos);
};

#endif // OLSONTOLOGYGRAPHICSVIEW_H

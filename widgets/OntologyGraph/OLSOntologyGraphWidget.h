#ifndef OLSONTOLOGYGRAPHWIDGET_H
#define OLSONTOLOGYGRAPHWIDGET_H

#include <QTextEdit>

#include "../Shared/OLSOntologyDataController.h"

#include "OLSOntologyGraphView.h"

class OLSOntologyGraphRelationVisualizedLine;
class OLSOntologyGraphNodeItem;

namespace Ui {
  class OLSOntologyGraphWidget;
}

class OLSOntologyGraphWidget : public QWidget {
    Q_OBJECT
    
  public:
    explicit OLSOntologyGraphWidget(QWidget *parent = 0);
    ~OLSOntologyGraphWidget();

    void setDataController(OLSOntologyDataController *dataController);
    OLSOntologyDataController *dataController() const;

    QImage makeScreenshot() const;

    void updateData();

  protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    
  private:
    Ui::OLSOntologyGraphWidget *ui;

    QTextEdit *m_sourceCodeViewer;
    OLSOntologyGraphView *m_ontologyView;
    QPointF m_lastRightClickScenePosition;
    OLSOntologyGraphRelationVisualizedLine *m_relationVisualizedLine;
    OLSOntologyDataController *m_dataController;

    bool m_editRelationMode;
    bool m_sourceCodeShown;

    void setEditRelationMode(bool on);
    void setRelation(OLSOntologyGraphNodeItem *sourceNode, OLSOntologyGraphNodeItem *destinationNode);
    OLSOntologyGraphNodeItem *addNode(QPointF scenePos);

  signals:
    void dataChangedSignal();

  public slots:
    void showContextMenuSlot(const QPoint &pos);

    void addNodeSlot();
    void setRelationSlot();
    void editNodeSlot();
    void editRelationSlot();
    void removeSelectedSlot();
    void editAttrsSlot();

    void setSolidDashPatternSlot();
    void setDashDashPatternSlot();
    void setDotDashPatternSlot();

    void sceneSelectionChangedSlot();

    void ontologyViewMousePositionChangedSlot(const QPoint &pos);

    void dataChangedSlot();

    void itemSelectedSlot(long id);
    void itemsSelectedSlot(const QSet<long> ids);

    void zoomInSlot();
    void zoomOutSlot();

    void nodeItemPositionChangedSlot(long id, const QPointF &newPosition);

    void showOntologySlot();
    void showSourceCodeSlot();
};

#endif // OLSONTOLOGYGRAPHWIDGET_H

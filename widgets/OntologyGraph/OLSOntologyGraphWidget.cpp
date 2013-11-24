#include "OLSOntologyGraphWidget.h"

#include "ui_OLSOntologyGraphWidget.h"
#include "ui_OLSOntologyGraphAttributeEditor.h"

#include <QMenu>
#include <QDebug>
#include <QInputDialog>

#include "OLSOntologyGraphNodeItem.h"
#include "OLSOntologyGraphRelationItem.h"
#include "OLSOntologyGraphRelationVisualizedLine.h"

OLSOntologyGraphWidget::OLSOntologyGraphWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::OLSOntologyGraphWidget) {

  ui->setupUi(this);

  setAcceptDrops(true);

  m_ontologyView = new OLSOntologyGraphView(this);
  m_ontologyView->setDragMode(QGraphicsView::RubberBandDrag);
  m_ontologyView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_ontologyView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  m_sourceCodeViewer = new QTextEdit(this);
  m_sourceCodeViewer->setReadOnly(true);
  m_sourceCodeViewer->setVisible(false);

  QGraphicsScene *scene = new QGraphicsScene(m_ontologyView);
  QBrush bgBrush = QBrush(Qt::Dense7Pattern);
  bgBrush.setColor(Qt::lightGray);
  scene->setBackgroundBrush(bgBrush);

  m_ontologyView->setScene(scene);

  connect(m_ontologyView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenuSlot(QPoint)));

  m_dataController = NULL;
  m_relationVisualizedLine = NULL;
  m_editRelationMode = false;
  m_sourceCodeShown = false;

  ui->verticalLayout->addWidget(m_ontologyView);
}

OLSOntologyGraphWidget::~OLSOntologyGraphWidget() {

  delete ui;
}

void OLSOntologyGraphWidget::setDataController(OLSOntologyDataController *dataController) {

  m_dataController = dataController;
}

OLSOntologyDataController *OLSOntologyGraphWidget::dataController() const {

  return m_dataController;
}

void OLSOntologyGraphWidget::showContextMenuSlot(const QPoint &pos) {

  QList<QGraphicsItem *> selectedItems = m_ontologyView->scene()->selectedItems();

  QMenu contextMenu;
  QAction *addNodeAction = contextMenu.addAction(tr("Add node..."));

  if (selectedItems.count() == 1) {
    QGraphicsItem *selectedItem = selectedItems.at(0);
    if (selectedItem->data(OLSOntologyGraphItemDataKey::kType).toInt() == OLSOntologyGraphItemType::kNode) {
      QAction *addRelationAction = contextMenu.addAction(tr("Set relation with..."));
      connect(addRelationAction, SIGNAL(triggered()), SLOT(setRelationSlot()));
    }
  }
  contextMenu.addSeparator();

  connect(addNodeAction, SIGNAL(triggered()), SLOT(addNodeSlot()));

  if (selectedItems.count() == 1) {
    QGraphicsItem *selectedItem = selectedItems.at(0);
    if (selectedItem->data(OLSOntologyGraphItemDataKey::kType).toInt() == OLSOntologyGraphItemType::kNode) {
      QAction *editNodeAction = contextMenu.addAction(tr("Edit node..."));
      connect(editNodeAction, SIGNAL(triggered()), SLOT(editNodeSlot()));
    }
    else if (selectedItem->data(OLSOntologyGraphItemDataKey::kType).toInt() == OLSOntologyGraphItemType::kRelation) {
      QAction *editRelationAction = contextMenu.addAction(tr("Edit relation..."));
      connect(editRelationAction, SIGNAL(triggered()), SLOT(editRelationSlot()));

      QMenu *dashMenu = contextMenu.addMenu("Dash");
      QAction *solidDashAction = dashMenu->addAction("Solid");
      QAction *dashDashAction = dashMenu->addAction("Dash");
      QAction *dotDashAction = dashMenu->addAction("Dot");

      connect(solidDashAction, SIGNAL(triggered()), SLOT(setSolidDashPatternSlot()));
      connect(dashDashAction, SIGNAL(triggered()), SLOT(setDashDashPatternSlot()));
      connect(dotDashAction, SIGNAL(triggered()), SLOT(setDotDashPatternSlot()));
    }
  }
  else {
    QAction *editAction = contextMenu.addAction(tr("Edit..."));
    editAction->setEnabled(false);

  }
  contextMenu.addSeparator();

  QAction *removeAction = contextMenu.addAction(tr("Remove..."));

  if (selectedItems.count() == 0) {
    removeAction->setEnabled(false);
  }
  else {
    connect(removeAction, SIGNAL(triggered()), SLOT(removeSelectedSlot()));
  }

  if (m_ontologyView->scene()->selectedItems().count() == 1) {
    contextMenu.addSeparator();
    QAction *editAttrsAction = contextMenu.addAction(tr("Edit attributes..."));
    connect(editAttrsAction, SIGNAL(triggered()), SLOT(editAttrsSlot()));
  }

  m_lastRightClickScenePosition = m_ontologyView->mapToScene(pos);
  QPoint globalPos = m_ontologyView->mapToGlobal(pos);
  contextMenu.exec(globalPos);
}

void OLSOntologyGraphWidget::setEditRelationMode(bool on) {

  if (on) {
    m_ontologyView->scene()->clearSelection();
    m_editRelationMode = true;
    m_ontologyView->setMouseTracking(true);
    m_ontologyView->setDragMode(QGraphicsView::NoDrag);
    connect(m_ontologyView, SIGNAL(mousePositionChangedSignal(QPoint)), SLOT(ontologyViewMousePositionChangedSlot(QPoint)));
    connect(m_ontologyView->scene(), SIGNAL(selectionChanged()), SLOT(sceneSelectionChangedSlot()));
  }
  else {
    m_editRelationMode = false;
    m_ontologyView->setMouseTracking(false);
    m_ontologyView->setDragMode(QGraphicsView::RubberBandDrag);
    disconnect(this, SLOT(ontologyViewMousePositionChangedSlot(QPoint)));

    if (m_relationVisualizedLine != NULL) {
      m_relationVisualizedLine->removeFromNodes();
      m_ontologyView->scene()->removeItem(m_relationVisualizedLine);
      delete m_relationVisualizedLine;
      m_relationVisualizedLine = NULL;
    }
  }
}

OLSOntologyGraphNodeItem *OLSOntologyGraphWidget::addNode(QPointF scenePos) {

  long newNodeId = -1;
  if (m_dataController != NULL) {
    newNodeId = m_dataController->nodeCreated();
    emit dataChangedSignal();
  }

  OLSOntologyGraphNodeItem *newNode = new OLSOntologyGraphNodeItem(NULL);
  newNode->setRelatedDataController(m_dataController);
  newNode->setId(newNodeId);
  newNode->setPos(scenePos);
  connect(newNode, SIGNAL(nodeItemPositionChangedSignal(long, QPointF)), SLOT(nodeItemPositionChangedSlot(long, QPointF)));
  m_ontologyView->scene()->addItem(newNode);
  return newNode;
}

void OLSOntologyGraphWidget::addNodeSlot() {

  QPointF scenePos = m_lastRightClickScenePosition;
  addNode(scenePos);
}

void OLSOntologyGraphWidget::setRelation(OLSOntologyGraphNodeItem *sourceNode, OLSOntologyGraphNodeItem *destinationNode) {

  long newRelationId = -1;
  if (m_dataController != NULL) {
    newRelationId = m_dataController->relationCreated(sourceNode->id(), destinationNode->id());
    emit dataChangedSignal();
  }

  OLSOntologyGraphRelationItem *relationItem = new OLSOntologyGraphRelationItem();
  relationItem->setRelatedDataController(m_dataController);
  relationItem->setId(newRelationId);
  relationItem->setSourceNode(sourceNode);
  relationItem->setDestinationNode(destinationNode);
  m_ontologyView->scene()->addItem(relationItem);
}

void OLSOntologyGraphWidget::updateData() {

  if (m_dataController == NULL) {
    this->setEnabled(false);
  }
  else {
    this->setEnabled(true);    

    QMap<long, OLSOntologyGraphNodeItem *> invalidatedNodesMap;
    QMap<long, OLSOntologyGraphRelationItem *> invalidatedRelationsMap;

    QList<QGraphicsItem *> items = m_ontologyView->scene()->items();
    foreach (QGraphicsItem *item, items) {
      if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kNode) {
        OLSOntologyGraphNodeItem *nodeItem = static_cast<OLSOntologyGraphNodeItem *>(item);
        invalidatedNodesMap.insert(nodeItem->id(), nodeItem);
      }
      else if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kRelation) {
        OLSOntologyGraphRelationItem *relationItem = static_cast<OLSOntologyGraphRelationItem *>(item);
        invalidatedRelationsMap.insert(relationItem->id(), relationItem);
      }
    }

    QMap<long, OLSOntologyGraphNodeItem *> existedNodes;
    int nodeCount = m_dataController->nodeCount();
    for (int i = 0; i < nodeCount; ++i) {
      OLSOntologyNodeData *nodeData = m_dataController->getNodeByIndex(i);
      if (invalidatedNodesMap.contains(nodeData->id)) {
        OLSOntologyGraphNodeItem *nodeItem = invalidatedNodesMap.value(nodeData->id);
        nodeItem->setRelatedDataController(m_dataController);
        nodeItem->setName(nodeData->name);
        invalidatedNodesMap.remove(nodeItem->id());
        existedNodes.insert(nodeItem->id(), nodeItem);
      }
      else {
        QPointF pos = m_dataController->nodePosition(nodeData->id);

        OLSOntologyGraphNodeItem *nodeItem = new OLSOntologyGraphNodeItem();
        nodeItem->setRelatedDataController(m_dataController);
        nodeItem->setId(nodeData->id);
        nodeItem->setName(nodeData->name);
        nodeItem->setPos(pos);
        m_ontologyView->scene()->addItem(nodeItem);
        existedNodes.insert(nodeItem->id(), nodeItem);
        connect(nodeItem,
                SIGNAL(nodeItemPositionChangedSignal(long, QPointF)),
                SLOT(nodeItemPositionChangedSlot(long, QPointF)));
      }
    }

    int relationCount = m_dataController->relationCount();
    for (int i = 0; i < relationCount; ++i) {
      OLSOntologyRelationData *relationData = m_dataController->getRelationByIndex(i);
      if (invalidatedRelationsMap.contains(relationData->id)) {
        OLSOntologyGraphRelationItem *relationItem = invalidatedRelationsMap.value(relationData->id);
        relationItem->setRelatedDataController(m_dataController);
        relationItem->setName(relationData->name);
        invalidatedRelationsMap.remove(relationItem->id());
      }
      else {
        OLSOntologyGraphRelationItem *relationItem = new OLSOntologyGraphRelationItem();
        relationItem->setRelatedDataController(m_dataController);
        relationItem->setId(relationData->id);
        relationItem->setName(relationData->name);

        OLSOntologyGraphNodeItem *sourceNode = existedNodes.value(relationData->sourceNodeId);
        OLSOntologyGraphNodeItem *destinationNode = existedNodes.value(relationData->destinationNodeId);

        relationItem->setSourceNode(sourceNode);
        relationItem->setDestinationNode(destinationNode);

        m_ontologyView->scene()->addItem(relationItem);
      }
    }

    foreach (OLSOntologyGraphNodeItem *invalidNode, invalidatedNodesMap.values()) {
      m_ontologyView->scene()->removeItem(invalidNode);
      delete invalidNode;
    }

    foreach (OLSOntologyGraphRelationItem *invalidRelation, invalidatedRelationsMap.values()) {
      m_ontologyView->scene()->removeItem(invalidRelation);
      delete invalidRelation;
    }
  }

  m_ontologyView->scene()->invalidate();
}

void OLSOntologyGraphWidget::setRelationSlot() {

  QList<QGraphicsItem *> selectedItems = m_ontologyView->scene()->selectedItems();
  if (selectedItems.count() == 1) {
    QGraphicsItem *selectedItem = selectedItems.at(0);
    if (selectedItem->data(OLSOntologyGraphItemDataKey::kType).toInt() == OLSOntologyGraphItemType::kNode) {
      OLSOntologyGraphNodeItem *sourceNodeItem = static_cast<OLSOntologyGraphNodeItem *>(selectedItem);

      if (m_relationVisualizedLine != NULL) {
        m_relationVisualizedLine->removeFromNodes();
        m_ontologyView->scene()->removeItem(m_relationVisualizedLine);
        delete m_relationVisualizedLine;
        m_relationVisualizedLine = NULL;
      }

      m_relationVisualizedLine = new OLSOntologyGraphRelationVisualizedLine();
      m_relationVisualizedLine->setSourceNode(sourceNodeItem);
      m_ontologyView->scene()->addItem(m_relationVisualizedLine);

      setEditRelationMode(true);
    }
  }
}

void OLSOntologyGraphWidget::editNodeSlot() {

  QList<QGraphicsItem *> selectedItems = m_ontologyView->scene()->selectedItems();
  if (selectedItems.count() == 1) {
    QGraphicsItem *selectedItem = selectedItems.at(0);
    if (selectedItem->data(OLSOntologyGraphItemDataKey::kType).toInt() == OLSOntologyGraphItemType::kNode) {
      OLSOntologyGraphNodeItem *nodeItem = static_cast<OLSOntologyGraphNodeItem *>(selectedItem);

      bool ok = false;
      QString newName = QInputDialog::getText(this,
                                              tr("Enter node name"),
                                              tr("Name: "),
                                              QLineEdit::Normal,
                                              nodeItem->name(),
                                              &ok);
      if (ok) {
        if (m_dataController != NULL) {
          m_dataController->nodeNameChanged(nodeItem->id(), newName);
          emit dataChangedSignal();
        }
        nodeItem->setName(newName);
        m_ontologyView->scene()->invalidate();
      }
    }
  }
}

void OLSOntologyGraphWidget::editRelationSlot() {

  QList<QGraphicsItem *> selectedItems = m_ontologyView->scene()->selectedItems();
  if (selectedItems.count() == 1) {
    QGraphicsItem *selectedItem = selectedItems.at(0);
    if (selectedItem->data(OLSOntologyGraphItemDataKey::kType).toInt() == OLSOntologyGraphItemType::kRelation) {
      OLSOntologyGraphRelationItem *relationItem = static_cast<OLSOntologyGraphRelationItem *>(selectedItem);

      bool ok = false;

      QStringList items;
      int relationsCount = m_dataController->relationCount();
      for (int i = 0; i < relationsCount; ++i) {
        OLSOntologyRelationData *relation = m_dataController->getRelationByIndex(i);
        if (!items.contains(relation->name)) {
          items.append(relation->name);
        }
      }

      QString newName = QInputDialog::getItem(this,
                                              tr("Enter node name"),
                                              tr("Name: "),
                                              items,
                                              0,
                                              true,
                                              &ok,
                                              0);

      if (ok) {
        if (m_dataController != NULL) {
          m_dataController->relationNameChanged(relationItem->id(), newName);
          emit dataChangedSignal();
        }
        relationItem->setName(newName);
        m_ontologyView->scene()->invalidate();
      }
    }
  }
}

void OLSOntologyGraphWidget::removeSelectedSlot() {

  QList<QGraphicsItem *> selectedItems = m_ontologyView->scene()->selectedItems();
  foreach (QGraphicsItem *item, selectedItems) {
    if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kNode) {
      OLSOntologyGraphNodeItem *nodeItem = static_cast<OLSOntologyGraphNodeItem *>(item);
      nodeItem->removeAllRelations();
      if (m_dataController != NULL) {
        m_dataController->nodeRemoved(nodeItem->id());
      }
    }
    else if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kRelation) {
      OLSOntologyGraphRelationItem *relationItem = static_cast<OLSOntologyGraphRelationItem *>(item);
      relationItem->removeFromNodes();
      if (m_dataController != NULL) {
        m_dataController->relationRemoved(relationItem->id());
      }
    }
    m_ontologyView->scene()->removeItem(item);
    delete item;
  }
  updateData();
  emit dataChangedSignal();
}

void OLSOntologyGraphWidget::editAttrsSlot() {

  QGraphicsItem *selectedItem = m_ontologyView->scene()->selectedItems().at(0);
  OLSOntologyGraphElement *element = NULL;
  if (selectedItem->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kNode) {
    element = static_cast<OLSOntologyGraphElement *>(static_cast<OLSOntologyGraphNodeItem *>(selectedItem));
  }
  else if (selectedItem->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kRelation) {
    element = static_cast<OLSOntologyGraphElement *>(static_cast<OLSOntologyGraphRelationItem *>(selectedItem));
  }

  Ui::OLSOntologyGraphAttributeEditor editor = Ui::OLSOntologyGraphAttributeEditor();

  QDialog *dialog = new QDialog(this);
  editor.setupUi(dialog);
  editor.textEdit->appendPlainText(element->attributesAsText());

  if (dialog->exec() == QDialog::Accepted) {
    element->setAttributesFromData(editor.textEdit->toPlainText().toLocal8Bit());
  }
}

void OLSOntologyGraphWidget::setSolidDashPatternSlot() {

  QGraphicsItem *selectedItem = m_ontologyView->scene()->selectedItems().at(0);
  OLSOntologyGraphElement *element = static_cast<OLSOntologyGraphElement *>(static_cast<OLSOntologyGraphRelationItem *>(selectedItem));
  QVariantMap attributesMap = element->attributes();
  QVariantMap dashMap;
  dashMap["dash"] = "solid";
  attributesMap["gui-attributes"] = dashMap;
  element->setAttributes(attributesMap);
}

void OLSOntologyGraphWidget::setDashDashPatternSlot() {

  QGraphicsItem *selectedItem = m_ontologyView->scene()->selectedItems().at(0);
  OLSOntologyGraphElement *element = static_cast<OLSOntologyGraphElement *>(static_cast<OLSOntologyGraphRelationItem *>(selectedItem));
  QVariantMap attributesMap = element->attributes();
  QVariantMap dashMap;
  dashMap["dash"] = "dash";
  attributesMap["gui-attributes"] = dashMap;
  element->setAttributes(attributesMap);
}

void OLSOntologyGraphWidget::setDotDashPatternSlot() {

  QGraphicsItem *selectedItem = m_ontologyView->scene()->selectedItems().at(0);
  OLSOntologyGraphElement *element = static_cast<OLSOntologyGraphElement *>(static_cast<OLSOntologyGraphRelationItem *>(selectedItem));
  QVariantMap attributesMap = element->attributes();
  QVariantMap dashMap;
  dashMap["dash"] = "dot";
  attributesMap["gui-attributes"] = dashMap;
  element->setAttributes(attributesMap);
}

void OLSOntologyGraphWidget::sceneSelectionChangedSlot() {

  if (m_editRelationMode) {
    QList<QGraphicsItem *> selectedItems = m_ontologyView->scene()->selectedItems();
    if (selectedItems.count() == 1) {
      QGraphicsItem *selectedItem = selectedItems.at(0);
      if (selectedItem->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kNode) {
        setRelation(m_relationVisualizedLine->sourceNode(), static_cast<OLSOntologyGraphNodeItem *>(selectedItem));
        setEditRelationMode(false);
      }
      else {
        m_ontologyView->scene()->clearSelection();
      }
    }
  }
}

void OLSOntologyGraphWidget::ontologyViewMousePositionChangedSlot(const QPoint &pos) {

  if (m_relationVisualizedLine != NULL) {
    QPointF scenePos = m_ontologyView->mapToScene(pos);
    m_relationVisualizedLine->setEndPoint(scenePos);
  }
}

void OLSOntologyGraphWidget::dataChangedSlot() {

  updateData();
}

void OLSOntologyGraphWidget::itemSelectedSlot(long id) {

  m_ontologyView->scene()->clearSelection();

  foreach (QGraphicsItem *item, m_ontologyView->scene()->items()) {
    if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kNode) {
      OLSOntologyGraphNodeItem *nodeItem = static_cast<OLSOntologyGraphNodeItem *>(item);
      if (nodeItem->id() == id) {
        item->setSelected(true);
        m_ontologyView->centerOn(item);
      }
    }
    else if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kRelation) {
      OLSOntologyGraphRelationItem *relationItem = static_cast<OLSOntologyGraphRelationItem *>(item);
      if (relationItem->id() == id) {
        item->setSelected(true);
        m_ontologyView->centerOn(item);
      }
    }
  }
}

void OLSOntologyGraphWidget::itemsSelectedSlot(const QSet<long> ids) {

  m_ontologyView->scene()->clearSelection();

  bool alreadyCentered = false;
  foreach (QGraphicsItem *item, m_ontologyView->scene()->items()) {
    if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kNode) {
      OLSOntologyGraphNodeItem *nodeItem = static_cast<OLSOntologyGraphNodeItem *>(item);
      if (ids.contains(nodeItem->id())) {
        item->setSelected(true);
        if (!alreadyCentered) {
          m_ontologyView->centerOn(item);
          alreadyCentered = true;
        }
      }
    }
//    else if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kRelation) {
//      RelationItem *relationItem = static_cast<RelationItem *>(item);
//      if (relationItem->id() == id) {
//        item->setSelected(true);
//        m_ontologyView->centerOn(item);
//      }
//    }
  }
}

void OLSOntologyGraphWidget::zoomInSlot() {

  m_ontologyView->scale(1.2, 1.2);
}

void OLSOntologyGraphWidget::zoomOutSlot() {

  m_ontologyView->scale(0.8, 0.8);
}

void OLSOntologyGraphWidget::nodeItemPositionChangedSlot(long id, const QPointF &newPosition) {

  m_dataController->setNodePosition(id, newPosition);
}

QImage OLSOntologyGraphWidget::makeScreenshot() const {

  QImage image(m_ontologyView->scene()->width(), m_ontologyView->scene()->height(), QImage::Format_ARGB32_Premultiplied);
  QPainter painter(&image);
  m_ontologyView->scene()->render(&painter);
  return image;
}

void OLSOntologyGraphWidget::showOntologySlot() {

  if (m_sourceCodeShown) {
    m_sourceCodeShown = false;
    m_sourceCodeViewer->setVisible(false);
    ui->verticalLayout->removeWidget(m_sourceCodeViewer);
    ui->verticalLayout->addWidget(m_ontologyView);
    m_ontologyView->setVisible(true);
  }
}

void OLSOntologyGraphWidget::showSourceCodeSlot() {

  if (!m_sourceCodeShown) {
    m_sourceCodeShown = true;
    if (m_dataController != NULL) {
      qDebug() << m_dataController->sourceCode();
      m_sourceCodeViewer->setText(m_dataController->sourceCode());
    }

    m_ontologyView->setVisible(false);
    ui->verticalLayout->removeWidget(m_ontologyView);
    ui->verticalLayout->addWidget(m_sourceCodeViewer);
    m_sourceCodeViewer->setVisible(true);
  }
}

void OLSOntologyGraphWidget::dropEvent(QDropEvent *event) {

  QByteArray jsonData = event->mimeData()->data("application/node-attrs");
  QPointF globalPos = m_ontologyView->mapToScene(event->pos());
  OLSOntologyGraphNodeItem *newNode = addNode(globalPos);
  newNode->setAttributesFromData(jsonData);
}

void OLSOntologyGraphWidget::dragEnterEvent(QDragEnterEvent *event) {

  if (event->mimeData()->hasFormat("application/node-attrs")) {
    event->acceptProposedAction();
  }
  else {
    event->ignore();
  }
}

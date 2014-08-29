#include "OLSOntologyDataController.h"

#include <QVariantMap>

OLSOntologyDataController::OLSOntologyDataController() {

  m_lastId = 0;

  m_changedNodeIds = QSet<long>();
  m_changedRelationIds = QSet<long>();
}

OLSOntologyDataController::OLSOntologyDataController(const QVariant &json) {

  deserialize(json);

  m_changedNodeIds = QSet<long>();
  m_changedRelationIds = QSet<long>();
}

OLSOntologyDataController::OLSOntologyDataController(QList<OLSOntologyNodeData *> nodeList,
                                               QList<OLSOntologyRelationData *> relationList,
                                               QMap<long, QPointF> nodePositions,
                                               QSet<long> changedNodeIds,
                                               QSet<long> changedRelationIds) {

  m_lastId = -1;

  foreach (OLSOntologyNodeData *nodeData, nodeList) {
    m_nodesMap.insert(nodeData->id, nodeData);
    m_nodesList.append(nodeData);
  }

  foreach (OLSOntologyRelationData *relationData, relationList) {
    m_relationsMap.insert(relationData->id, relationData);
    m_relationsList.append(relationData);
    m_relationsMapByNodes.insert(QPair<long, long>(relationData->sourceNodeId, relationData->destinationNodeId), relationData);
  }

  m_nodePositions = nodePositions;
  m_changedNodeIds = changedNodeIds;
  m_changedRelationIds = changedRelationIds;

  qDebug() << "Snapshot created: " << m_changedNodeIds << "\n" << m_changedRelationIds;
}

void OLSOntologyDataController::setSourceCode(const QString &sourceCode) {

  m_sourceCode = sourceCode;
}

void OLSOntologyDataController::normalize() {

  double minX = INFINITY;
  double minY = INFINITY;

  foreach (long nodeId, m_nodePositions.keys()) {
    QPointF position = m_nodePositions.value(nodeId);
    minX = qMin(position.x(), minX);
    minY = qMin(position.y(), minY);
  }

  minX -= 200;
  minY -= 200;

  foreach (long nodeId, m_nodePositions.keys()) {
    QPointF position = m_nodePositions.value(nodeId);
    position.setX(position.x() - minX);
    position.setY(position.y() - minY);
    m_nodePositions.insert(nodeId, position);
  }
}

QVariant OLSOntologyDataController::serialize() {

  m_jsonMap["last_id"] = QVariant::fromValue(m_lastId);

  QVariantList nodes;
  foreach (OLSOntologyNodeData *nodeData, m_nodesList) {
    QVariantMap node;
    node["id"] = QVariant::fromValue(nodeData->id);
    node["name"] = nodeData->name;
    node["position_x"] = QVariant::fromValue(nodePosition(nodeData->id).x());
    node["position_y"] = QVariant::fromValue(nodePosition(nodeData->id).y());
    node["attributes"] = nodeData->attributes;
    nodes.append(node);
  }
  m_jsonMap["nodes"] = nodes;

  QVariantList relations;
  foreach (OLSOntologyRelationData *relationData, m_relationsList) {
    QVariantMap relation;
    relation["id"] = QVariant::fromValue(relationData->id);
    relation["name"] = relationData->name;
    relation["source_node_id"] = QVariant::fromValue(relationData->sourceNodeId);
    relation["destination_node_id"] = QVariant::fromValue(relationData->destinationNodeId);
    relation["attributes"] = relationData->attributes;
    relations.append(relation);
  }
  m_jsonMap["relations"] = relations;

  return m_jsonMap;
}

void OLSOntologyDataController::deserialize(const QVariant &json) {

  qDebug() << "Ontology data controller deserialization";

  m_jsonMap = json.toMap();

  qDebug() << m_jsonMap;

  m_lastId = m_jsonMap["last_id"].toLongLong();

  QVariantList nodes = m_jsonMap["nodes"].toList();
  foreach (QVariant node, nodes) {
    QVariantMap nodeMap = node.toMap();

    qDebug() << nodeMap;

    OLSOntologyNodeData *nodeData = new OLSOntologyNodeData();
    nodeData->id = nodeMap["id"].toLongLong();
    nodeData->name = nodeMap["name"].toString();
    if (nodeMap.contains("attributes")) {
      nodeData->attributes = nodeMap["attributes"].toMap();
    }

    if (nodeMap.contains("position_x") && nodeMap.contains("position_y")) {
      double x = nodeMap["position_x"].toDouble();
      double y = nodeMap["position_y"].toDouble();
      setNodePosition(nodeData->id, QPointF(x, y));
    }
    m_nodesMap.insert(nodeData->id, nodeData);
    m_nodesList.append(nodeData);
  }

  QVariantList relations = m_jsonMap["relations"].toList();
  foreach (QVariant relation, relations) {
    QVariantMap relationMap = relation.toMap();
    OLSOntologyRelationData *relationData = new OLSOntologyRelationData();
    relationData->id = relationMap["id"].toLongLong();
    relationData->name = relationMap["name"].toString();
    relationData->sourceNodeId = relationMap["source_node_id"].toLongLong();
    relationData->destinationNodeId = relationMap["destination_node_id"].toLongLong();
    if (relationMap.contains("attributes")) {
      relationData->attributes = relationMap["attributes"].toMap();
    }
    m_relationsMap.insert(relationData->id, relationData);
    m_relationsList.append(relationData);

    QPair<long, long> relationNodesPair(relationData->sourceNodeId, relationData->destinationNodeId);
    m_relationsMapByNodes.insert(relationNodesPair, relationData);

    OLSOntologyNodeData *sourceNode = m_nodesMap.value(relationData->sourceNodeId);
    sourceNode->relations.append(relationData->id);

    OLSOntologyNodeData *destinationNode = m_nodesMap.value(relationData->destinationNodeId);
    destinationNode->relations.append(relationData->id);
  }

  normalize();
  updateCategories();
}

void OLSOntologyDataController::updateCategories() {

    foreach (OLSOntologyRelationData *relation, m_relationsList) {
        if (!m_categories.keys().contains(relation->name)) {
            m_categories.insert(relation->name, QVariantMap());
        }
    }
}

// data source

int OLSOntologyDataController::nodeCount() {

  return m_nodesList.count();
}

int OLSOntologyDataController::relationCount() {

  return m_relationsList.count();
}

OLSOntologyNodeData *OLSOntologyDataController::getNodeByIndex(int index, bool asChanges) {

  OLSOntologyNodeData *nodeData = m_nodesList.at(index);
  if (asChanges) {
    m_changedNodeIds.insert(nodeData->id);
  }
  return nodeData;
}

OLSOntologyRelationData *OLSOntologyDataController::getRelationByIndex(int index, bool asChanges) {

  OLSOntologyRelationData *relationData = m_relationsList.at(index);
  if (asChanges) {
    m_changedRelationIds.insert(relationData->id);
  }
  return relationData;
}

OLSOntologyNodeData *OLSOntologyDataController::getNodeById(long id, bool asChanges) {

  if (asChanges) {
    m_changedNodeIds.insert(id);
  }
  return m_nodesMap.value(id);
}

OLSOntologyRelationData *OLSOntologyDataController::getRelationById(long id, bool asChanges) {

  if (asChanges) {
    m_changedRelationIds.insert(id);
  }
  return m_relationsMap.value(id);
}

OLSOntologyNodeData *OLSOntologyDataController::getNodeByPath(const QStringList &path) const {

  OLSOntologyNodeData *prevNode = NULL;
  foreach (QString nodeName, path) {
    OLSOntologyNodeData *node = findNode(nodeName, prevNode);
    Q_ASSERT(node != NULL);
    prevNode = node;
  }
  m_changedNodeIds.insert(prevNode->id);
  return prevNode;
}

OLSOntologyRelationData *OLSOntologyDataController::getRelationByNodes(long sourceNodeId, long destinationNodeId) {

  QPair<long, long> nodesPair(sourceNodeId, destinationNodeId);
  OLSOntologyRelationData *relationData = m_relationsMapByNodes.value(nodesPair);
  m_changedRelationIds.insert(relationData->id);
  return relationData;
}

OLSOntologyNodeData *OLSOntologyDataController::findNode(const QString &nodeName) const {

  foreach (OLSOntologyNodeData *nodeData, m_nodesList) {
    if (nodeData->name.compare(nodeName, Qt::CaseInsensitive) == 0) {
      m_changedNodeIds.insert(nodeData->id);
      return nodeData;
    }
  }
  return NULL;
}

OLSOntologyNodeData *OLSOntologyDataController::findNode(const QString &nodeName, OLSOntologyNodeData *startNode) const {

  if (startNode == NULL) {
    return findNode(nodeName);
  }

  foreach (long relationId, startNode->relations) {
    OLSOntologyRelationData *relation = m_relationsMap.value(relationId);
    if (relation->name.compare("transform", Qt::CaseInsensitive) != 0
        && relation->name.compare("type_transform", Qt::CaseInsensitive) != 0) {
      OLSOntologyNodeData *node = otherNode(relation, startNode);
      if (node->name.compare(nodeName, Qt::CaseInsensitive) == 0) {
        m_changedNodeIds.insert(node->id);
        return node;
      }
    }
  }

  qDebug() << QString("Can not find node `%1` from node `%2`").arg(nodeName, startNode->name);

  return NULL;
}

QList<QString> OLSOntologyDataController::categoryNames() const {

    return m_categories.keys();
}

QVariantMap OLSOntologyDataController::getCategory(const QString &categoryName) const {

    return m_categories.value(categoryName, QVariantMap());
}

void OLSOntologyDataController::setCategory(const QString &categoryName, QVariantMap category) {

    m_categories.insert(categoryName, category);
}

QStringList OLSOntologyDataController::pathToNode(long id) {

  m_changedNodeIds.insert(id);

  QStringList path;
  OLSOntologyNodeData *currentNode = NULL;
  OLSOntologyNodeData *nextNode = getNodeById(id);

  while (nextNode != NULL) {
    currentNode = nextNode;
    nextNode = NULL;
    path.push_front(currentNode->name);

    foreach (long relationId, currentNode->relations) {
      OLSOntologyRelationData *relation = getRelationById(relationId);
      if (relation->sourceNodeId == currentNode->id) {
        nextNode = getNodeById(relation->destinationNodeId);
        break;
      }
    }
  }

  return path;
}

OLSOntologyNodeData *OLSOntologyDataController::otherNode(OLSOntologyRelationData *relation, OLSOntologyNodeData *node) const {

  if (relation->sourceNodeId == node->id) {
    return m_nodesMap.value(relation->destinationNodeId);
  }
  else {
    return m_nodesMap.value(relation->sourceNodeId);
  }
}

QString OLSOntologyDataController::sourceCode() const {

  return m_sourceCode;
}

// delegate

long OLSOntologyDataController::nodeCreated() {

  m_lastId++;

  OLSOntologyNodeData *node = new OLSOntologyNodeData();
  node->id = m_lastId;
  node->name = "";

  m_nodesMap.insert(node->id, node);
  m_nodesList.append(node);

  m_changedNodeIds.insert(node->id);

  return node->id;
}

long OLSOntologyDataController::relationCreated(long sourceNodeId, long destinationNodeId) {

  m_lastId++;

  OLSOntologyRelationData *relation = new OLSOntologyRelationData();
  relation->id = m_lastId;
  relation->name = "";
  relation->sourceNodeId = sourceNodeId;
  relation->destinationNodeId = destinationNodeId;

  m_relationsMap.insert(relation->id, relation);
  m_relationsList.append(relation);

  QPair<long, long> relationNodesPair(relation->sourceNodeId, relation->destinationNodeId);
  m_relationsMapByNodes.insert(relationNodesPair, relation);

  OLSOntologyNodeData *sourceNode = m_nodesMap.value(relation->sourceNodeId);
  sourceNode->relations.append(relation->id);

  OLSOntologyNodeData *destinationNode = m_nodesMap.value(relation->destinationNodeId);
  destinationNode->relations.append(relation->id);

  m_changedRelationIds.insert(relation->id);

  updateCategories();

  return relation->id;
}

void OLSOntologyDataController::nodeNameChanged(long nodeId, const QString &name) {

  OLSOntologyNodeData *node = m_nodesMap.value(nodeId);
  node->name = name;

  m_changedNodeIds.insert(node->id);
}

void OLSOntologyDataController::relationNameChanged(long relationId, const QString &name) {

  OLSOntologyRelationData *relation = m_relationsMap.value(relationId);
  relation->name = name;

  m_changedRelationIds.insert(relation->id);

  updateCategories();
}

void OLSOntologyDataController::nodeAttributesChanged(long nodeId, const QVariantMap &attributes) {

  OLSOntologyNodeData *node = m_nodesMap.value(nodeId);
  node->attributes = attributes;

  m_changedNodeIds.insert(node->id);
}

void OLSOntologyDataController::relationAttributesChanged(long relationId, const QVariantMap &attributes) {

  OLSOntologyRelationData *relation = m_relationsMap.value(relationId);
  relation->attributes = attributes;

  m_changedRelationIds.insert(relation->id);
}

void OLSOntologyDataController::nodeRemoved(long nodeId) {

  OLSOntologyNodeData *node = m_nodesMap.value(nodeId);
  removeRelatedRelations(node);
  m_nodesMap.remove(nodeId);
  m_nodesList.removeAll(node);
  delete node;
}

void OLSOntologyDataController::relationRemoved(long relationId) {

  qDebug() << m_relationsMap;

  OLSOntologyRelationData *relation = m_relationsMap.value(relationId);

  if (relation != NULL) {
    m_relationsMap.remove(relationId);
    m_relationsList.removeAll(relation);

    QPair<long, long> relationNodesPair(relation->sourceNodeId, relation->destinationNodeId);
    m_relationsMapByNodes.remove(relationNodesPair);

    delete relation;
  }
}

void OLSOntologyDataController::removeRelatedRelations(OLSOntologyNodeData *nodeData) {

  QList<OLSOntologyRelationData *> relationsToRemove;
  foreach (OLSOntologyRelationData *relationData, m_relationsList) {
    if (relationData->sourceNodeId == nodeData->id || relationData->destinationNodeId == nodeData->id) {
      relationsToRemove.append(relationData);
    }
  }

  foreach (OLSOntologyRelationData *relationData, relationsToRemove) {
    m_relationsList.removeAll(relationData);
    m_relationsMap.remove(relationData->id);

    QPair<long, long> relationNodesPair(relationData->sourceNodeId, relationData->destinationNodeId);
    m_relationsMapByNodes.remove(relationNodesPair);

    delete relationData;
  }
}

QPointF OLSOntologyDataController::nodePosition(long nodeId) const {

  if (m_nodePositions.contains(nodeId)) {
    return m_nodePositions.value(nodeId);
  }
  else {
    return QPointF(0.0, 0.0);
  }
}

void OLSOntologyDataController::setNodePosition(long nodeId, const QPointF &position) {

  qDebug() << "Position changed!";
  m_nodePositions.insert(nodeId, position);
}

bool OLSOntologyDataController::isNodeChanged(long id) const {

  return m_changedNodeIds.contains(id);
}

bool OLSOntologyDataController::isRelationChanged(long id) const {

  return m_changedRelationIds.contains(id);
}

bool OLSOntologyDataController::hasChanges() const {

  return m_changedNodeIds.count() > 0 || m_changedRelationIds.count() > 0;
}

int OLSOntologyDataController::changesCount() const {

  return m_changedNodeIds.count() + m_changedRelationIds.count();
}

void OLSOntologyDataController::clearChanges() {

  m_changedNodeIds.clear();
  m_changedRelationIds.clear();
}

OLSOntologyDataController *OLSOntologyDataController::makeSnapshot() {

  OLSOntologyDataController *snapshot = new OLSOntologyDataController(m_nodesList,
                                                                m_relationsList,
                                                                m_nodePositions,
                                                                m_changedNodeIds,
                                                                m_changedRelationIds);

  return snapshot;
}

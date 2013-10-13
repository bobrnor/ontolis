#ifndef OLSONTOLOGYDATACONTROLLER_H
#define OLSONTOLOGYDATACONTROLLER_H

#include <QtGui>

#include "OLSOntologyNodeData.h"
#include "OLSOntologyRelationData.h"

class OLSOntologyDataController {
  private:
    long m_lastId;
    // mapping
    QMap<long, OLSOntologyNodeData *> m_nodesMap;
    QMap<long, OLSOntologyRelationData *> m_relationsMap;
    QMap<QPair<long, long>, OLSOntologyRelationData *> m_relationsMapByNodes;
    // listing
    QList<OLSOntologyNodeData *> m_nodesList;
    QList<OLSOntologyRelationData *> m_relationsList;
    QVariantMap m_jsonMap;

    QMap<long, QPointF> m_nodePositions;

    mutable QSet<long> m_changedNodeIds;
    mutable QSet<long> m_changedRelationIds;

    QString m_sourceCode;

    void removeRelatedRelations(OLSOntologyNodeData *nodeData);
    OLSOntologyNodeData *otherNode(OLSOntologyRelationData *relation, OLSOntologyNodeData *node) const;

    OLSOntologyDataController(QList<OLSOntologyNodeData *> nodeList,
                           QList<OLSOntologyRelationData *> relationList,
                           QMap<long, QPointF> nodePositions,
                           QSet<long> changedNodeIds,
                           QSet<long> changedRelationIds);

  public:
    OLSOntologyDataController();
    OLSOntologyDataController(const QVariant &json);

    void setSourceCode(const QString &sourceCode);

    void normalize();

    OLSOntologyDataController *makeSnapshot();

    // data source
    int nodeCount();
    int relationCount();

    OLSOntologyNodeData *getNodeByIndex(int index, bool asChanges = false);
    OLSOntologyRelationData *getRelationByIndex(int index, bool asChanges = false);

    OLSOntologyNodeData *getNodeById(long id, bool asChanges = false);
    OLSOntologyRelationData *getRelationById(long id, bool asChanges = false);

    OLSOntologyNodeData *getNodeByPath(const QStringList &path) const;

    OLSOntologyRelationData *getRelationByNodes(long sourceNodeId, long destinationNodeId);

    OLSOntologyNodeData *findNode(const QString &nodeName) const;
    OLSOntologyNodeData *findNode(const QString &nodeName, OLSOntologyNodeData *startNode) const;

    QStringList pathToNode(long id);

    QString sourceCode() const;

    bool isNodeChanged(long id) const;
    bool isRelationChanged(long id) const;
    bool hasChanges() const;
    int changesCount() const;
    void clearChanges();

    // delegate
    long nodeCreated();
    long relationCreated(long sourceNodeId, long destinationNodeId);
    void nodeNameChanged(long nodeId, const QString &name);
    void relationNameChanged(long relationId, const QString &name);
    void nodeAttributesChanged(long nodeId, const QVariantMap &attributes);
    void relationAttributesChanged(long relationId, const QVariantMap &attributes);
    void nodeRemoved(long nodeId);
    void relationRemoved(long relationId);

    QPointF nodePosition(long nodeId) const;
    void setNodePosition(long nodeId, const QPointF &position);

    QVariant serialize();
    void deserialize(const QVariant &json);
};

#endif // OLSONTOLOGYDATACONTROLLER_H

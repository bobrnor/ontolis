#ifndef OLSONTOLOGYTREEVIEWCONTROLLER_H
#define OLSONTOLOGYTREEVIEWCONTROLLER_H

#include <QtWidgets>

#include "../Shared/OLSOntologyDataController.h"

struct OLSOntologyTreeViewNodeData {
    OLSOntologyNodeData *nodeData;
    QMap<QString, QList<OLSOntologyTreeViewNodeData> > *childNodes;
};

class OLSOntologyTreeViewController : public QObject {
    Q_OBJECT

  private:
    QTreeView *m_objectsTreeView;
    QStandardItemModel *m_objectsModel;
    OLSOntologyDataController *m_dataController;

    QMap<long, OLSOntologyTreeViewNodeData> m_treeData;

    bool m_dragEnabled;

    void updateTreeDataBottomToTop();
    void updateTreeDataTopToBottom();
    void buildNodesTree(QStandardItem *rootItem, QList<OLSOntologyTreeViewNodeData> nodes, QSet<long> *seenNodeIds);
    void clearTreeView();

  public:
    OLSOntologyTreeViewController();
    ~OLSOntologyTreeViewController();

    QTreeView *treeView() const;

    void setDataController(OLSOntologyDataController *dataController);
    OLSOntologyDataController *dataController() const;

    void setDragEnabled(bool enabled);
    bool isDragEnabled() const;

    void updateData();

  signals:
    void dataChangedSignal();
    void itemSelectedSignal(long id);

  public slots:
    void dataChangedSlot();
    void itemSelectedSlot(const QModelIndex &index);
};

#endif // OLSONTOLOGYTREEVIEWCONTROLLER_H

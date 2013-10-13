#include "OLSProjectTreeViewController.h"

#include <QDebug>
#include <QInputDialog>
#include <QAction>

#include "OLSProject.h"
#include "OLSProjectFileCategory.h"

OLSProjectTreeViewController::OLSProjectTreeViewController() {

  m_treeWidget = new OLSProjectTreeWidget();
  m_treeWidget->setDelegate(this);
  m_treeWidget->setHeaderHidden(true);
  m_treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
  m_treeWidget->setAcceptDrops(true);
  m_treeWidget->setDropIndicatorShown(true);

  setupMenu();

  connect(m_treeWidget, SIGNAL(activated(QModelIndex)), SLOT(itemSelectedSlot(QModelIndex)));
}

OLSProjectTreeViewController::~OLSProjectTreeViewController() {

  delete m_treeWidget;
}

void OLSProjectTreeViewController::setupMenu() {

  QAction *addCategoryAction = new QAction("Add category", m_treeWidget);
  connect(addCategoryAction, SIGNAL(triggered()), SLOT(addCategorySlot()));
  m_treeWidget->addAction(addCategoryAction);
}

void OLSProjectTreeViewController::setProject(OLSProject *project) {

  m_project = project;
}

OLSProject *OLSProjectTreeViewController::project() const {

  return m_project;
}

QTreeWidget *OLSProjectTreeViewController::widget() const {

  return m_treeWidget;
}

void OLSProjectTreeViewController::updateData() {

  m_treeWidget->clear();

  if (m_project != NULL) {
    m_treeWidget->setColumnCount(1);

    for (int i = 0; i < m_project->filesCount(); ++i) {
      OLSProjectFile *file = m_project->getProjectFileByIndex(i);
      QString fileName = file->name();
      QTreeWidgetItem *item = new QTreeWidgetItem(m_treeWidget, QStringList(fileName));
      item->setData(0, Qt::UserRole, QVariant(fileName));
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

      if (file->categories().count() > 0) {
        foreach (OLSProjectFileCategory *category, file->categories()) {
          QTreeWidgetItem *categoryItem = new QTreeWidgetItem(item, QStringList(category->name()));
          categoryItem->setData(0, Qt::UserRole, QVariant(fileName));
          categoryItem->setData(0, Qt::UserRole + 1, QVariant(category->name()));
          categoryItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);

          if (category->relatedNodeIds().count() > 0) {
            foreach (long nodeId, category->relatedNodeIds()) {
              OLSOntologyNodeData *nodeData = file->ontologyController()->getNodeById(nodeId);
              Q_ASSERT(nodeData != NULL);
              QTreeWidgetItem *nodeItem = new QTreeWidgetItem(categoryItem, QStringList(nodeData->name));
//              categoryItem->setData(0, Qt::UserRole, QVariant(fileName));
//              categoryItem->setData(1, Qt::UserRole, QVariant(category->name()));
              nodeItem->setData(0, Qt::UserRole, QVariant((qlonglong)nodeData->id));
              nodeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            }
          }
        }
      }
    }
  }
}

bool OLSProjectTreeViewController::isCategoryIndex(const QModelIndex &index) {

  QMap<int, QVariant> itemData = m_treeWidget->model()->itemData(index);
  QString fileName = itemData.value(Qt::UserRole).toString();
  QString categoryName = itemData.value(Qt::UserRole + 1).toString();

  OLSProjectFile *projectFile = m_project->getProjectFileByName(fileName);
  if (projectFile != NULL) {
    OLSProjectFileCategory *category = projectFile->getCategoryByName(categoryName);
    if (category != NULL) {
      return true;
    }
  }

  return false;
}

bool OLSProjectTreeViewController::isFileIndex(const QModelIndex &index) {

  return index.parent() == QModelIndex();
}

bool OLSProjectTreeViewController::processDrop(QTreeWidgetItem *parentItem, const QMap<int, QVariant> &dropData) {

  QString fileName = parentItem->data(0, Qt::UserRole).toString();
  QString categoryName = parentItem->data(0, Qt::UserRole + 1).toString();
  long nodeId = dropData.value(Qt::UserRole).toLongLong();

  OLSProjectFile *projectFile = m_project->getProjectFileByName(fileName);
  if (projectFile != NULL) {
    OLSProjectFileCategory *category = projectFile->getCategoryByName(categoryName);
    if (category != NULL) {
      return category->addRelatedNodeId(nodeId);
    }
  }

  return false;
}

void OLSProjectTreeViewController::itemSelectedSlot(const QModelIndex &index) {

  if (isCategoryIndex(index)) {
    QString fileName = m_treeWidget->model()->data(index, Qt::UserRole).toString();
    QString categoryName = m_treeWidget->model()->data(index, Qt::UserRole + 1).toString();
    emit categorySelectedSignal(fileName, categoryName);
  }
  else if (isFileIndex(index)) {
    QString fileName = m_treeWidget->model()->data(index, Qt::UserRole).toString();
    emit currentFileChangedSignal(fileName);
  }
}

void OLSProjectTreeViewController::addCategorySlot() {

  QList<QTreeWidgetItem *> selectedItems = m_treeWidget->selectedItems();
  if (selectedItems.count() > 0) {
    QTreeWidgetItem *selectedItem = selectedItems.at(0);
    if (m_treeWidget->indexOfTopLevelItem(selectedItem) != -1) {
      QString fileName = selectedItem->data(0, Qt::UserRole).toString();
      OLSProjectFile *file = m_project->getProjectFileByName(fileName);

      bool ok = false;
      QString categoryName = QInputDialog::getText(m_treeWidget,
                                                   tr("Enter category name"),
                                                   tr("Name: "),
                                                   QLineEdit::Normal,
                                                   QString(),
                                                   &ok);

      if (ok) {
        OLSProjectFileCategory *category = new OLSProjectFileCategory(categoryName, file);
        file->addCategory(category);
        updateData();
      }
    }
  }
}

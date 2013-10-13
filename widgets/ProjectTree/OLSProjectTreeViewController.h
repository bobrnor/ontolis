#ifndef OLSOLSProjectTreeViewController_H
#define OLSOLSProjectTreeViewController_H

#include "OLSProjectFileCategory.h"
#include "OLSProjectTreeWidget.h"
#include "OLSProjectTreeWidgetDelegate.h"

class OLSProject;
class OLSProjectFileCategory;

class OLSProjectTreeViewController : public QObject, public OLSProjectTreeWidgetDelegate {

    Q_OBJECT

  private:
    OLSProjectTreeWidget *m_treeWidget;
    OLSProject *m_project;

    void setupMenu();

  public:
    OLSProjectTreeViewController();
    ~OLSProjectTreeViewController();

    void setProject(OLSProject *project);
    OLSProject *project() const;

    QTreeWidget *widget() const;

    void updateData();

    bool isCategoryIndex(const QModelIndex &index);
    bool isFileIndex(const QModelIndex &index);
    bool processDrop(QTreeWidgetItem *parentItem, const QMap<int, QVariant> &dropData);

  public slots:
    void itemSelectedSlot(const QModelIndex &index);
    void addCategorySlot();

  signals:
    void currentFileChangedSignal(const QString &fileName);
    void categorySelectedSignal(const QString &fileName, const QString &categoryName);
};

#endif // OLSOLSProjectTreeViewController_H

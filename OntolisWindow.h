#ifndef ONTOLISWINDOW_H
#define ONTOLISWINDOW_H

#include <QMainWindow>

#include "widgets/Shared/OLSOntologyDataController.h"
#include "widgets/OntologyGraph/OLSOntologyGraphWidget.h"
#include "widgets/OntologyTree/OLSOntologyTreeViewController.h"
#include "widgets/OntologyPallete/OLSOntologyPalleteWidget.h"
#include "widgets/ProjectTree/OLSProjectTreeViewController.h"
#include "widgets/ProjectTree/OLSProject.h"

namespace Ui {
class OntolisWindow;
}

class OntolisWindow : public QMainWindow {
    Q_OBJECT
    
  public:
    explicit OntolisWindow(QWidget *parent = 0);
    ~OntolisWindow();
    
  private:
    Ui::OntolisWindow *ui;

    OLSProject m_currentProject;

    QList<OLSOntologyGraphWidget *> m_openOntologyWidgets;

    QTreeView *m_logTreeView;

    OLSOntologyPalleteWidget *m_palleteWidget;

    OLSProjectTreeViewController *m_projectTreeViewController;
    OLSOntologyTreeViewController *m_ontologyTreeViewController;

    long m_currentSnapshotIndex;

    QShortcut *m_zoomInShortcut;
    QShortcut *m_zoomOutShortcut;
    QShortcut *m_removeShortcut;

    QMap<QString, QString> m_convertersMapping;
    QMap<QString, QString> m_transformatorsMapping;

    void setupMenu();
    void setupConverters();
    void setupTransformators();

    OLSOntologyGraphWidget *createNewOntologyWidget(OLSProjectFile *file);

    void clearConnections();
    void updateOntologyTreeData();

  private slots:
    void importSourceFileSlot();

    void createOntologyFileSlot();
    void openOntologyFileSlot();
    void saveOntologyFileSlot();

    void openProjectSlot();
    void saveProjectSlot();

    void exportFileSlot();
    void screenshotSlot();

    void transformSlot();

    void moveToStartSlot();
    void moveForwardSlot();
    void moveBackwardSlot();
    void moveToEndSlot();

    void currentTabChangedSlot(int index);
    void currentFileChangedSlot(const QString &fileName);
    void categorySelectedSlot(const QString &fileName, const QString &categoryName);

  signals:
    void itemsSelectedSignal(const QSet<long> ids);
};

#endif // ONTOLISWINDOW_H

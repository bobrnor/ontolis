#include "OntolisWindow.h"
#include "ui_OntolisWindow.h"

#include <QFile>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QTextStream>
#include <QTableWidget>

OntolisWindow::OntolisWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::OntolisWindow) {

  ui->setupUi(this);

  m_projectTreeViewController = new OLSProjectTreeViewController();
  m_projectTreeViewController->setProject(&m_currentProject);
  ui->projectTreeLayout->addWidget(m_projectTreeViewController->widget());

  m_ontologyTreeViewController = new OLSOntologyTreeViewController();
  ui->ontologyTreeLayout->addWidget(m_ontologyTreeViewController->treeView());

  m_logTreeView = new QTreeView();
  m_logTreeView->setHeaderHidden(true);
  m_logTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
//  m_logTreeView->setModel(m_transformationHelper->logModel());
  m_logTreeView->hide();

  m_palleteWidget = new OLSOntologyPalleteWidget();
  ui->palleteLayout->addWidget(m_palleteWidget);

  m_categoryWidget = new OntolisCategoryWidget();
  ui->categoryLayout->addWidget(m_categoryWidget);

  m_zoomInShortcut = new QShortcut(this);
  m_zoomInShortcut->setKey(QKeySequence("Ctrl+="));
  m_zoomInShortcut->setEnabled(true);

  m_zoomOutShortcut = new QShortcut(this);
  m_zoomOutShortcut->setKey(QKeySequence("Ctrl+-"));
  m_zoomOutShortcut->setEnabled(true);

  m_removeShortcut = new QShortcut(this);
  m_removeShortcut->setKey(QKeySequence::Delete);
  m_removeShortcut->setEnabled(true);

  QAction *toBeginAction = ui->toolBar->addAction(tr("Move to start"));
  connect(toBeginAction, SIGNAL(triggered()), SLOT(moveToStartSlot()));

  QAction *moveBackwardAction = ui->toolBar->addAction(tr("Move backward"));
  connect(moveBackwardAction, SIGNAL(triggered()), SLOT(moveBackwardSlot()));

  QAction *moveForwardAction = ui->toolBar->addAction(tr("Move forward"));
  connect(moveForwardAction, SIGNAL(triggered()), SLOT(moveForwardSlot()));

  QAction *toEndAction = ui->toolBar->addAction(tr("Move to the end"));
  connect(toEndAction, SIGNAL(triggered()), SLOT(moveToEndSlot()));

  m_currentSnapshotIndex = 0;

  connect(ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(currentTabChangedSlot(int)));
  connect(m_projectTreeViewController, SIGNAL(currentFileChangedSignal(QString)), SLOT(currentFileChangedSlot(QString)));
  connect(m_projectTreeViewController, SIGNAL(categorySelectedSignal(QString,QString)), SLOT(categorySelectedSlot(QString,QString)));

  setupMenu();
  setupConverters();
  setupTransformators();
}

OntolisWindow::~OntolisWindow() {

  delete ui;
}

OLSOntologyGraphWidget * OntolisWindow::createNewOntologyWidget(OLSProjectFile *file) {

  OLSOntologyGraphWidget *newWidget = new OLSOntologyGraphWidget(this);
  newWidget->setDataController(file->ontologyController());
  m_openOntologyWidgets.append(newWidget);
  ui->tabWidget->addTab(newWidget, file->name());

  return newWidget;
}

void OntolisWindow::clearConnections() {

  m_ontologyTreeViewController->disconnect();
  m_zoomInShortcut->disconnect();
  m_zoomOutShortcut->disconnect();
  m_removeShortcut->disconnect();
}

void OntolisWindow::updateOntologyTreeData() {

  int index = ui->tabWidget->currentIndex();
  OLSOntologyGraphWidget *widget = m_openOntologyWidgets[index];
  m_ontologyTreeViewController->setDataController(widget->dataController());
  m_ontologyTreeViewController->updateData();
}

void OntolisWindow::setupConverters() {

  QDir dir("/Users/bobrnor/Dropbox/PSU/Projects/ontolis/Scripts");

  qDebug() << dir.path();

  foreach (QString fileName, dir.entryList(QDir::Files)) {
    if (fileName.contains("converter")) {
      QString cmd;
#ifdef Q_OS_WIN32
      cmd.append("python ");
#endif
      cmd.append(dir.absoluteFilePath(fileName));
      cmd.append(" --method=supported_extensions");

      QProcess process;
      process.start(cmd, QIODevice::ReadOnly);
      process.waitForFinished();
      QString result(process.readAll());
      qDebug() << result;

      QStringList extensions = result.split(":");
      foreach (QString extension, extensions) {
        m_convertersMapping.insert(extension.trimmed(), dir.absoluteFilePath(fileName));
      }
    }
  }
}

void OntolisWindow::setupTransformators() {

    QDir dir("/Users/bobrnor/Dropbox/PSU/Projects/ontolis/Scripts");

    qDebug() << dir.path();

    foreach (QString fileName, dir.entryList(QDir::Files)) {
      if (fileName.contains("transformator")) {
        QString cmd;
  #ifdef Q_OS_WIN32
        cmd.append("python ");
  #endif
        cmd.append(dir.absoluteFilePath(fileName));
        cmd.append(" --method=supported_extensions");

        QProcess process;
        process.start(cmd, QIODevice::ReadOnly);
        process.waitForFinished();
        QString result(process.readAll());
        qDebug() << result;

        QStringList extensions = result.split(":");
        foreach (QString extension, extensions) {
          m_transformatorsMapping.insert(extension.trimmed(), dir.absoluteFilePath(fileName));
        }
      }
    }
}

void OntolisWindow::setupMenu() {

  QMenu *fileMenu = ui->menubar->addMenu(tr("File"));
  QAction *importSourceFileAction = fileMenu->addAction(tr("Import source file..."));
  fileMenu->addSeparator();
  QAction *createOntologyFileAction = fileMenu->addAction(tr("Create ontology..."));
  QAction *openOntologyFileAction = fileMenu->addAction(tr("Open ontology..."));
  QAction *saveOntologyFileAction = fileMenu->addAction(tr("Save ontology..."));
  QAction *saveOntologyAsFileAction = fileMenu->addAction(tr("Save ontology as..."));
  fileMenu->addSeparator();
  QAction *openProjectAction = fileMenu->addAction(tr("Open project..."));
  QAction *saveProjectAction = fileMenu->addAction(tr("Save project..."));
  fileMenu->addSeparator();
  QAction *exportFileAction = fileMenu->addAction(tr("Export file..."));
  fileMenu->addSeparator();
  QAction *screenshotAction = fileMenu->addAction(tr("Screenshot..."));

  QMenu *viewMenu = ui->menubar->addMenu(tr("View"));
  QAction *ontologyViewAction = viewMenu->addAction(tr("Show ontology"));
  QAction *sourceViewAction = viewMenu->addAction(tr("Show source code"));

  QMenu *transformationMenu = ui->menubar->addMenu(tr("Transformation"));
  QAction *transformAction = transformationMenu->addAction(tr("Transform"));

  connect(importSourceFileAction, SIGNAL(triggered()), SLOT(importSourceFileSlot()));

  connect(createOntologyFileAction, SIGNAL(triggered()), SLOT(createOntologyFileSlot()));
  connect(openOntologyFileAction, SIGNAL(triggered()), SLOT(openOntologyFileSlot()));
  connect(saveOntologyFileAction, SIGNAL(triggered()), SLOT(saveOntologyFileSlot()));
  connect(saveOntologyAsFileAction, SIGNAL(triggered()), SLOT(saveOntologyAsFileSlot()));

  connect(openProjectAction, SIGNAL(triggered()), SLOT(openProjectSlot()));
  connect(saveProjectAction, SIGNAL(triggered()), SLOT(saveProjectSlot()));

  connect(exportFileAction, SIGNAL(triggered()), SLOT(exportFileSlot()));

  connect(screenshotAction, SIGNAL(triggered()), SLOT(screenshotSlot()));

  connect(transformAction, SIGNAL(triggered()), SLOT(transformSlot()));

  connect(ontologyViewAction, SIGNAL(triggered()), SLOT(showOntologySlot()));
  connect(sourceViewAction, SIGNAL(triggered()), SLOT(showSourceCodeSlot()));
}

void OntolisWindow::importSourceFileSlot() {

  QString filePath = QFileDialog::getOpenFileName(this, tr("Open dialog"), QString(), "*");

  QFileInfo info(filePath);
  QString converterPath = m_convertersMapping["." + info.completeSuffix()];

  if (converterPath.length() > 0) {
    QString cmd;
#ifdef Q_OS_WIN32
      cmd.append("python ");
#endif
    cmd.append(converterPath);
    cmd.append(" --method=import");
    cmd.append(" --source-path=");
    cmd.append(filePath);

    QProcess process;
    process.start(cmd, QIODevice::ReadOnly);
    process.waitForFinished();
    QString result(process.readAllStandardOutput());

    OLSProjectFile *file = m_currentProject.createFile(result);
    if (file != NULL) {
      OLSOntologyGraphWidget *widget = createNewOntologyWidget(file);
      widget->dataChangedSlot();

      updateOntologyTreeData();
      m_projectTreeViewController->updateData();
    }
  }
}

void OntolisWindow::createOntologyFileSlot() {

  OLSProjectFile *file = m_currentProject.createFile();
  if (file != NULL) {
    OLSOntologyGraphWidget *widget = createNewOntologyWidget(file);
    widget->dataChangedSlot();

    updateOntologyTreeData();
    m_projectTreeViewController->updateData();
  }
}

void OntolisWindow::openOntologyFileSlot() {

  QString filePath = QFileDialog::getOpenFileName(this, tr("Open dialog"), QString(), "*");
  OLSProjectFile *file = m_currentProject.openFile(filePath);

  if (file != NULL) {
    OLSOntologyGraphWidget *widget = createNewOntologyWidget(file);
    widget->dataChangedSlot();

    updateOntologyTreeData();
    m_projectTreeViewController->updateData();
  }
}

void OntolisWindow::saveOntologyFileSlot() {

  int index = ui->tabWidget->currentIndex();
  OLSProjectFile *file = m_currentProject.getProjectFileByIndex(index);
  if (file->path().length() > 0) {
    m_currentProject.saveFile(file);
  }
  else {
    QString filePath = QFileDialog::getSaveFileName(this, tr("Open dialog"), QString(), "*");
    m_currentProject.saveFile(file, filePath);
  }

  ui->tabWidget->setTabText(index, file->name());
  updateOntologyTreeData();
  m_projectTreeViewController->updateData();
}

void OntolisWindow::saveOntologyAsFileSlot() {

  int index = ui->tabWidget->currentIndex();
  OLSProjectFile *file = m_currentProject.getProjectFileByIndex(index);
  QString filePath = QFileDialog::getSaveFileName(this, tr("Open dialog"), QString(), "*");
  m_currentProject.saveFile(file, filePath);

  ui->tabWidget->setTabText(index, file->name());
  updateOntologyTreeData();
  m_projectTreeViewController->updateData();
}

void OntolisWindow::openProjectSlot() {

  QString filePath = QFileDialog::getOpenFileName(this, tr("Open dialog"), QString(), "*.pjs");
  bool result = m_currentProject.openProject(filePath);

  if (result) {
    for (int i = 0; i < m_currentProject.filesCount(); ++i) {
      OLSProjectFile *file = m_currentProject.getProjectFileByIndex(i);

      OLSOntologyGraphWidget *widget = createNewOntologyWidget(file);
      widget->dataChangedSlot();
    }

    updateOntologyTreeData();
    m_projectTreeViewController->updateData();
  }
}

void OntolisWindow::saveProjectSlot() {

  QString filePath = QFileDialog::getSaveFileName(this, tr("Save dialog"), QString(), "*.pjs");
  m_currentProject.saveProject(filePath);
}

void OntolisWindow::exportFileSlot() {

  QString filePath = QFileDialog::getSaveFileName(this, tr("Open dialog"), QString(), "*");

  QFileInfo info(filePath);
  QString converterPath = m_convertersMapping["." + info.completeSuffix()];

  if (converterPath.length() > 0) {
    int index = ui->tabWidget->currentIndex();
    OLSProjectFile *file = m_currentProject.getProjectFileByIndex(index);

    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(true);
    tmpFile.open();
    QVariant json = file->ontologyController()->serialize();
    QByteArray data = QJsonDocument::fromVariant(json).toJson();
    tmpFile.write(data);
    tmpFile.flush();

    QString cmd;
#ifdef Q_OS_WIN32
      cmd.append("python ");
#endif
    cmd.append(converterPath);
    cmd.append(" --method=export");
    cmd.append(" --source-path=");
    cmd.append(tmpFile.fileName());

    qDebug() << cmd;

    QProcess process;
    process.start(cmd, QIODevice::ReadOnly);
    process.waitForFinished();
    QString result(process.readAll());

    qDebug() << result;

    QFile dstFile(filePath);
    dstFile.open(QIODevice::WriteOnly | QIODevice::Text);
    dstFile.write(result.toLocal8Bit());
  }
}

void OntolisWindow::screenshotSlot() {

  QString filePath = QFileDialog::getSaveFileName(this, tr("Save dialog"), QString(), "*.png");
  if (filePath != NULL) {
    if (ui->tabWidget->currentIndex() >= 0 && ui->tabWidget->currentIndex() < m_openOntologyWidgets.count()) {
      OLSOntologyGraphWidget *widget = m_openOntologyWidgets[ui->tabWidget->currentIndex()];
      QImage screenshot = widget->makeScreenshot();
      screenshot.save(filePath);
    }
  }
}

void OntolisWindow::currentTabChangedSlot(int index) {

  OLSOntologyGraphWidget *widget = m_openOntologyWidgets[index];

  clearConnections();

  m_ontologyTreeViewController->setDataController(widget->dataController());
  m_categoryWidget->setWidget(widget);

  connect(widget, SIGNAL(dataChangedSignal()), m_ontologyTreeViewController, SLOT(dataChangedSlot()));
  connect(m_ontologyTreeViewController, SIGNAL(dataChangedSignal()), widget, SLOT(dataChangedSlot()));

  connect(m_ontologyTreeViewController, SIGNAL(dataChangedSignal()), widget, SLOT(dataChangedSlot()));

  connect(m_ontologyTreeViewController, SIGNAL(itemSelectedSignal(long)), widget, SLOT(itemSelectedSlot(long)));

  connect(m_zoomInShortcut, SIGNAL(activated()), widget, SLOT(zoomInSlot()));
  connect(m_zoomOutShortcut, SIGNAL(activated()), widget, SLOT(zoomOutSlot()));
  connect(m_removeShortcut, SIGNAL(activated()), widget, SLOT(removeSelectedSlot()));

  connect(this, SIGNAL(itemsSelectedSignal(QSet<long>)), widget, SLOT(itemsSelectedSlot(QSet<long>)));

  m_ontologyTreeViewController->updateData();
  m_ontologyTreeViewController->setDragEnabled(true);
  widget->updateData();
}

void OntolisWindow::currentFileChangedSlot(const QString &fileName) {

  for (int i = 0; i < m_currentProject.filesCount(); ++i) {
    OLSProjectFile *file = m_currentProject.getProjectFileByIndex(i);
    if (file->name() == fileName) {
      ui->tabWidget->setCurrentIndex(i);
    }
  }
}

void OntolisWindow::categorySelectedSlot(const QString &fileName, const QString &categoryName) {

  OLSProjectFile *file = NULL;

  for (int i = 0; i < m_currentProject.filesCount(); ++i) {
    OLSProjectFile *projFile = m_currentProject.getProjectFileByIndex(i);
    if (projFile->name() == fileName) {
      file = projFile;
      break;
    }
  }

  if (file != NULL) {
    OLSProjectFileCategory *category = file->getCategoryByName(categoryName);
    if (category != NULL) {
      emit itemsSelectedSignal(category->relatedNodeIds());
    }
  }
}

void OntolisWindow::transformSlot() {

    int index = ui->tabWidget->currentIndex();
    OLSProjectFile *file = m_currentProject.getProjectFileByIndex(index);

    QString converterPath = "";
    if (file->ontologyController()->findNode("##.java")) {
        converterPath = m_transformatorsMapping[".java"];
    }
    else {
        converterPath = m_transformatorsMapping[".m"];
    }

    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(false);
    tmpFile.open();
    QVariant json = file->ontologyController()->serialize();
    QByteArray data = QJsonDocument::fromVariant(json).toJson();
    tmpFile.write(data);
    tmpFile.flush();

    QString cmd;
#ifdef Q_OS_WIN32
      cmd.append("python ");
#endif
    cmd.append(converterPath);
    cmd.append(" --method=transform");
    cmd.append(" --source-path=");
    cmd.append(tmpFile.fileName());

    qDebug() << cmd;

    QProcess process;
    process.start(cmd, QIODevice::ReadOnly);
    process.waitForFinished();
    QString result(process.readAll());

//    qDebug() << result;

    QTemporaryFile tmpDstFile;
    tmpDstFile.setAutoRemove(false);
    tmpDstFile.open();
    tmpDstFile.write(result.toLocal8Bit());
    tmpDstFile.flush();

    if (file->ontologyController()->findNode("##.java")) {
        converterPath = m_convertersMapping[".m"];
    }
    else {
        converterPath = m_convertersMapping[".java"];
    }

    cmd.clear();
#ifdef Q_OS_WIN32
      cmd.append("python ");
#endif
    cmd.append(converterPath);
    cmd.append(" --method=import");
    cmd.append(" --source-path=");
    cmd.append(tmpDstFile.fileName());

    qDebug() << cmd;

    QProcess otherProcess;
    otherProcess.start(cmd, QIODevice::ReadOnly);
    otherProcess.waitForFinished();
    QString _result(otherProcess.readAllStandardOutput());

    OLSProjectFile *_file = m_currentProject.createFile(_result);
    if (_file != NULL) {
      OLSOntologyGraphWidget *widget = createNewOntologyWidget(_file);
      widget->dataChangedSlot();

      updateOntologyTreeData();
      m_projectTreeViewController->updateData();
    }
}

void OntolisWindow::showOntologySlot() {

    int index = ui->tabWidget->currentIndex();
    if (index >= 0) {
        OLSOntologyGraphWidget *widget = m_openOntologyWidgets[index];
        widget->showOntologySlot();
    }
}

void OntolisWindow::showSourceCodeSlot() {

    int index = ui->tabWidget->currentIndex();
    if (index >= 0) {
        OLSOntologyGraphWidget *widget = m_openOntologyWidgets[index];
        widget->showSourceCodeSlot();
    }
}

void OntolisWindow::moveToStartSlot() {

  qDebug() << "Move to start";

//  ProjectFile *currentFile = m_currentProject.getProjectFileByName(m_currentFileName);

//  if (currentFile != NULL) {
//    if (m_currentSnapshotIndex > 0)  {
//      m_currentSnapshotIndex = 0;

//      Snapshot *snapshot = m_transformationHelper->snapshots().at(m_currentSnapshotIndex);

//      OntologyDataController *sourceDataController = snapshot->sourceOntologySnapshot();
//      OntologyDataController *destinationDataController = snapshot->destinationOntologySnapshot();
//      OntologyDataController *problemsDataController = snapshot->problemsOntologySnapshot();

//      m_sourceOntologyWidget->setDataController(sourceDataController);
//      m_sourceOntologyWidget->dataChangedSlot();

//      m_destinationOntologyWidget->setDataController(destinationDataController);
//      m_destinationOntologyWidget->dataChangedSlot();

//      m_problemsOntologyWidget->setDataController(problemsDataController);
//      m_problemsOntologyWidget->dataChangedSlot();

//      m_logTreeView->setModel(snapshot->logModelSnapshot());
//      m_logTreeView->expandAll();

//      if (sourceDataController->changesCount() > destinationDataController->changesCount()) {
//        if (sourceDataController->changesCount() > problemsDataController->changesCount()) {
//          ui->tabWidget->setCurrentIndex(0);
//        }
//        else {
//          ui->tabWidget->setCurrentIndex(4);
//        }
//      }
//      else {
//        if (destinationDataController->changesCount() > problemsDataController->changesCount()) {
//          ui->tabWidget->setCurrentIndex(1);
//        }
//        else {
//          ui->tabWidget->setCurrentIndex(4);
//        }
//      }
//    }
//  }
}

void OntolisWindow::moveForwardSlot() {

//  ProjectFile *currentFile = m_currentProject.getProjectFileByName(m_currentFileName);

//  if (currentFile != NULL) {
//    if (m_currentSnapshotIndex < m_transformationHelper->snapshots().count() - 1) {
//      m_currentSnapshotIndex++;

//      qDebug() << "Move to " << m_currentSnapshotIndex << " snapshot";

//      Snapshot *snapshot = m_transformationHelper->snapshots().at(m_currentSnapshotIndex);

//      OntologyDataController *sourceDataController = snapshot->sourceOntologySnapshot();
//      OntologyDataController *destinationDataController = snapshot->destinationOntologySnapshot();
//      OntologyDataController *problemsDataController = snapshot->problemsOntologySnapshot();

//      m_sourceOntologyWidget->setDataController(sourceDataController);
//      m_sourceOntologyWidget->dataChangedSlot();

//      m_destinationOntologyWidget->setDataController(destinationDataController);
//      m_destinationOntologyWidget->dataChangedSlot();

//      m_problemsOntologyWidget->setDataController(problemsDataController);
//      m_problemsOntologyWidget->dataChangedSlot();

//      m_logTreeView->setModel(snapshot->logModelSnapshot());
//      m_logTreeView->expandAll();

//      if (sourceDataController->changesCount() > destinationDataController->changesCount()) {
//        if (sourceDataController->changesCount() > problemsDataController->changesCount()) {
//          ui->tabWidget->setCurrentIndex(0);
//        }
//        else {
//          ui->tabWidget->setCurrentIndex(4);
//        }
//      }
//      else {
//        if (destinationDataController->changesCount() > problemsDataController->changesCount()) {
//          ui->tabWidget->setCurrentIndex(1);
//        }
//        else {
//          ui->tabWidget->setCurrentIndex(4);
//        }
//      }
//    }
//    else {
//      moveToEndSlot();
//    }
//  }
}

void OntolisWindow::moveBackwardSlot() {

//  ProjectFile *currentFile = m_currentProject.getProjectFileByName(m_currentFileName);

//  if (currentFile != NULL) {
//    if (m_currentSnapshotIndex > 0) {
//      m_currentSnapshotIndex--;

//      qDebug() << "Move to " << m_currentSnapshotIndex << " snapshot";

//      Snapshot *snapshot = m_transformationHelper->snapshots().at(m_currentSnapshotIndex);

//      OntologyDataController *sourceDataController = snapshot->sourceOntologySnapshot();
//      OntologyDataController *destinationDataController = snapshot->destinationOntologySnapshot();
//      OntologyDataController *problemsDataController = snapshot->problemsOntologySnapshot();

//      m_sourceOntologyWidget->setDataController(sourceDataController);
//      m_sourceOntologyWidget->dataChangedSlot();

//      m_destinationOntologyWidget->setDataController(destinationDataController);
//      m_destinationOntologyWidget->dataChangedSlot();

//      m_problemsOntologyWidget->setDataController(problemsDataController);
//      m_problemsOntologyWidget->dataChangedSlot();

//      m_logTreeView->setModel(snapshot->logModelSnapshot());
//      m_logTreeView->expandAll();

//      if (sourceDataController->changesCount() > destinationDataController->changesCount()) {
//        if (sourceDataController->changesCount() > problemsDataController->changesCount()) {
//          ui->tabWidget->setCurrentIndex(0);
//        }
//        else {
//          ui->tabWidget->setCurrentIndex(4);
//        }
//      }
//      else {
//        if (destinationDataController->changesCount() > problemsDataController->changesCount()) {
//          ui->tabWidget->setCurrentIndex(1);
//        }
//        else {
//          ui->tabWidget->setCurrentIndex(4);
//        }
//      }
//    }
//  }
}

void OntolisWindow::moveToEndSlot() {

  qDebug() << "Move to the end";

//  ProjectFile *currentFile = m_currentProject.getProjectFileByName(m_currentFileName);
//  if (m_currentFileName != NULL) {
//    m_currentSnapshotIndex = m_transformationHelper->snapshots().count();

//    m_sourceOntologyWidget->setDataController(currentFile->sourceOntologyController());
//    m_sourceOntologyWidget->dataChangedSlot();

//    m_destinationOntologyWidget->setDataController(currentFile->destinationOntologyController());
//    m_destinationOntologyWidget->dataChangedSlot();

//    m_problemsOntologyWidget->setDataController(m_currentProject.problemsOntologyController());
//    m_problemsOntologyWidget->dataChangedSlot();

//    m_logTreeView->setModel(m_transformationHelper->logModel());
//    m_logTreeView->expandAll();

//    ui->tabWidget->setCurrentIndex(1);
//  }
}

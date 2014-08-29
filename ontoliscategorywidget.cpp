#include "ontoliscategorywidget.h"
#include "ui_ontoliscategorywidget.h"

#include "widgets/OntologyGraph/OLSOntologyGraphNodeItem.h"
#include "widgets/OntologyGraph/OLSOntologyGraphRelationItem.h"

OntolisCategoryWidget::OntolisCategoryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OntolisCategoryWidget)
{
    ui->setupUi(this);

    m_widget = NULL;

    connect(ui->saveButton, SIGNAL(clicked()), SLOT(saveSlot()));
    connect(ui->cancelButton, SIGNAL(clicked()), SLOT(cancelSlot()));
    connect(ui->categoryComboBox, SIGNAL(currentIndexChanged(int)), SLOT(currentIndexChangedSlot(int)));
}

OntolisCategoryWidget::~OntolisCategoryWidget()
{
    delete ui;
}

void OntolisCategoryWidget::setWidget(OLSOntologyGraphWidget *widget) {

    if (m_widget) {
        disconnect(m_widget->m_ontologyView->scene());
    }

    m_widget = widget;
    connect(m_widget->m_ontologyView->scene(), SIGNAL(selectionChanged()), SLOT(selectionChanged()));
    update();
}

void OntolisCategoryWidget::update() {

    ui->categoryComboBox->clear();
    ui->categoryTextEdit->clear();
    foreach (QString categoryName, m_widget->dataController()->categoryNames()) {
        ui->categoryComboBox->addItem(categoryName);
    }
}

void OntolisCategoryWidget::saveSlot() {

    qDebug() << "saved";
    QJsonDocument doc = QJsonDocument::fromJson(ui->categoryTextEdit->toPlainText().toLocal8Bit());

    if (m_widget->m_ontologyView->scene()->selectedItems().length() > 0) {
        foreach (QGraphicsItem *item, m_widget->m_ontologyView->scene()->selectedItems()) {
            if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kNode) {
              OLSOntologyGraphNodeItem *nodeItem = static_cast<OLSOntologyGraphNodeItem *>(item);
              m_widget->dataController()->setCategory(nodeItem->name(), doc.object().toVariantMap());
              nodeItem->attributesChanged();
            }
            else if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kRelation) {
              OLSOntologyGraphRelationItem *relationItem = static_cast<OLSOntologyGraphRelationItem *>(item);
              QVariantMap map = QVariantMap();
              map.insert("gui-attributes", doc.object().toVariantMap());
              relationItem->setAttributes(map);
          }
        }
    }
    else {
        m_widget->dataController()->setCategory(ui->categoryComboBox->currentText(), doc.object().toVariantMap());
        foreach (QGraphicsItem *item, m_widget->m_ontologyView->scene()->items()) {
            if (item->data(OLSOntologyGraphItemDataKey::kType) == OLSOntologyGraphItemType::kRelation) {
              OLSOntologyGraphRelationItem *relationItem = static_cast<OLSOntologyGraphRelationItem *>(item);
              if (relationItem->name() == ui->categoryComboBox->currentText()) {
                  relationItem->attributesChanged();
                }
            }
        }
    }
}

void OntolisCategoryWidget::cancelSlot() {

    qDebug() << "canceled";
    currentIndexChangedSlot(ui->categoryComboBox->currentIndex());
}

void OntolisCategoryWidget::currentIndexChangedSlot(int index) {

    qDebug() << "changed";
    QVariantMap attrs = m_widget->dataController()->getCategory(ui->categoryComboBox->currentText());
    QJsonObject object = QJsonObject::fromVariantMap(attrs);
    QJsonDocument document = QJsonDocument(object);
    ui->categoryTextEdit->setPlainText(QString::fromUtf8(document.toJson()));
}

void OntolisCategoryWidget::selectionChanged() {

    if (m_widget->m_ontologyView->scene()->selectedItems().length() > 0) {
        ui->categoryComboBox->setEditText("");
    }
    else {
        update();
    }
}

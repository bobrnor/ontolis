#ifndef ONTOLISCATEGORYWIDGET_H
#define ONTOLISCATEGORYWIDGET_H

#include <QWidget>
#include "widgets/OntologyGraph/OLSOntologyGraphWidget.h"
#include "widgets/OntologyGraph/OLSOntologyGraphElement.h"

namespace Ui {
class OntolisCategoryWidget;
}

class OntolisCategoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OntolisCategoryWidget(QWidget *parent = 0);
    ~OntolisCategoryWidget();

    void setWidget(OLSOntologyGraphWidget *widget);

private:
    Ui::OntolisCategoryWidget *ui;

    OLSOntologyGraphWidget *m_widget;
    QSet<OLSOntologyGraphElement *> m_selectedElements;

    void update();

public slots:
    void saveSlot();
    void cancelSlot();
    void currentIndexChangedSlot(int index);
    void selectionChanged();
};

#endif // ONTOLISCATEGORYWIDGET_H

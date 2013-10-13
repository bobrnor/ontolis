#ifndef OLSONTOLOGYPALLETEWIDGET_H
#define OLSONTOLOGYPALLETEWIDGET_H

#include <QListWidget>

class OLSOntologyPalleteWidget : public QListWidget {

    Q_OBJECT
  public:
    explicit OLSOntologyPalleteWidget(QWidget *parent = 0);
    
  protected:
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QList<QListWidgetItem *> items) const;    
};

#endif // OLSONTOLOGYPALLETEWIDGET_H

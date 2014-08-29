#ifndef OLSONTOLOGYGRAPHELEMENT_H
#define OLSONTOLOGYGRAPHELEMENT_H

#include <QString>
#include <QMap>

#include "../Shared/OLSOntologyDataController.h"

class OLSOntologyGraphElement {
  public:
    long m_id;
    QString m_name;

    OLSOntologyDataController *m_dataController;

    virtual void attributesChanged() = 0;

  public:
    OLSOntologyGraphElement();

    void setId(long id);
    long id() const;

    virtual QString attributesAsText() const = 0;
    virtual QVariantMap attributes() const = 0;
    virtual void setAttributesFromData(const QByteArray &data) = 0;
    virtual void setAttributes(const QVariantMap &attributes) = 0;

    virtual void setName(const QString &name);
    virtual QString name() const;

    void setRelatedDataController(OLSOntologyDataController *dataController);
    OLSOntologyDataController *relatedDataController() const;
};

#endif // OLSONTOLOGYGRAPHELEMENT_H

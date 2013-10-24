#ifndef OLSONTOLOGYNODEDATA_H
#define OLSONTOLOGYNODEDATA_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QList>
#include <QVariantMap>

struct OLSOntologyNodeData {
    long id;
    QString name;
    QList<long> relations;
    QVariantMap attributes;

    QString attributesAsText() const {

      QJsonObject object = QJsonObject::fromVariantMap(attributes);
      QJsonDocument document = QJsonDocument(object);
      return QString::fromUtf8(document.toJson());
    }

    void setAttributesFromData(const QByteArray &data) {

      QJsonDocument document = QJsonDocument::fromJson(data);
      QJsonObject object = document.object();
      attributes = object.toVariantMap();
    }
};

#endif // OLSONTOLOGYNODEDATA_H

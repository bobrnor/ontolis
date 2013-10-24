#ifndef OLSONTOLOGYRELATIONDATA_H
#define OLSONTOLOGYRELATIONDATA_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVariantMap>

struct OLSOntologyRelationData {
    long id;
    long sourceNodeId;
    long destinationNodeId;
    QString name;
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

#endif // OLSONTOLOGYRELATIONDATA_H

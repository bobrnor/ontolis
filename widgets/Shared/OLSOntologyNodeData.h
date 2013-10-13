#ifndef OLSONTOLOGYNODEDATA_H
#define OLSONTOLOGYNODEDATA_H

#include <qjson/include/qjson/parser.h>
#include <qjson/include/qjson/serializer.h>

#include <QString>
#include <QList>
#include <QVariantMap>

struct OLSOntologyNodeData {
    long id;
    QString name;
    QList<long> relations;
    QVariantMap attributes;

    QString attributesAsText() const {

      QJson::Serializer serializer;
      return QString::fromLocal8Bit(serializer.serialize(attributes));
    }

    void setAttributesFromData(const QByteArray &data) {

      QJson::Parser parser;
      attributes = parser.parse(data).toMap();
    }
};

#endif // OLSONTOLOGYNODEDATA_H

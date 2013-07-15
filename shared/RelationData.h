#ifndef RELATIONDATA_H
#define RELATIONDATA_H

#include <qjson/include/qjson/parser.h>
#include <qjson/include/qjson/serializer.h>

#include <QString>
#include <QVariantMap>

struct RelationData {
    long id;
    long sourceNodeId;
    long destinationNodeId;
    QString name;
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

#endif // RELATIONDATA_H

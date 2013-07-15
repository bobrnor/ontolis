#ifndef NODEDATA_H
#define NODEDATA_H

#include <qjson/include/qjson/parser.h>
#include <qjson/include/qjson/serializer.h>

#include <QString>
#include <QList>
#include <QVariantMap>

struct NodeData {
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

#endif // NODEDATA_H

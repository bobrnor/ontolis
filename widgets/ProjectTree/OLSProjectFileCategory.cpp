#include "OLSProjectFileCategory.h"

#include <QVariantMap>

OLSProjectFileCategory::OLSProjectFileCategory(QString name, OLSProjectFile *parentFile)
  : m_name(name), m_parentFile(parentFile) {

}

void OLSProjectFileCategory::setName(QString name) {

  m_name = name;
}

QString OLSProjectFileCategory::name() const {

  return m_name;
}

bool OLSProjectFileCategory::addRelatedNodeId(long id) {

  if (!m_relatedNodeIds.contains(id)) {
    m_relatedNodeIds.insert(id);
    return true;
  }
  return false;
}

void OLSProjectFileCategory::setRelatedNodeIds(QSet<long> ids) {

  m_relatedNodeIds = ids;
}

QSet<long> OLSProjectFileCategory::relatedNodeIds() const {

  return m_relatedNodeIds;
}

QVariant OLSProjectFileCategory::serialize() const {

  QVariantMap jsonMap;
  jsonMap["name"] = m_name;

  QVariantList ids;
  foreach (long id, m_relatedNodeIds) {
    ids.append(QVariant::fromValue(id));
  }
  jsonMap["ids"] = ids;
  return jsonMap;
}

void OLSProjectFileCategory::deserialize(const QVariant &json, OLSProjectFile *parentFile) {

  QVariantMap jsonMap = json.toMap();
  m_parentFile = parentFile;
  m_name = jsonMap["name"].toString();

  m_relatedNodeIds.clear();

  QVariantList idsJson = jsonMap["ids"].toList();
  foreach (QVariant idJson, idsJson) {
    m_relatedNodeIds.insert(idJson.toLongLong());
  }
}

bool OLSProjectFileCategory::operator ==(const OLSProjectFileCategory &category) {

  return m_name == category.name();
}

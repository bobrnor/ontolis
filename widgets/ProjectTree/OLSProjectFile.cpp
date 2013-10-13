#include "OLSProjectFile.h"

OLSProjectFile::OLSProjectFile() : m_name("Untitled"), m_path("") {

  m_ontologyController = new OLSOntologyDataController();
}

OLSProjectFile::OLSProjectFile(const QString &path, const QString &name) : m_name(name), m_path(path) {

  m_ontologyController = new OLSOntologyDataController();
}

OLSProjectFile::~OLSProjectFile() {

  delete m_ontologyController;
}

const QString &OLSProjectFile::name() const {

  return m_name;
}

const QString &OLSProjectFile::path() const {

  return m_path;
}

void OLSProjectFile::setName(const QString &name) {

  m_name = name;
}

void OLSProjectFile::setPath(const QString &path) {

  m_path = path;
}

OLSOntologyDataController *OLSProjectFile::ontologyController() const {

  return m_ontologyController;
}

void OLSProjectFile::addCategory(OLSProjectFileCategory *category) {

  foreach (OLSProjectFileCategory *existsCategory, m_categories) {
    if (category->name() == existsCategory->name()) {
      return;
    }
  }

  m_categories.append(category);
}

void OLSProjectFile::removeCategory(OLSProjectFileCategory *category) {

  m_categories.removeOne(category);
}

OLSProjectFileCategory *OLSProjectFile::getCategoryByName(const QString &name) const {

  foreach (OLSProjectFileCategory *category, m_categories) {
    if (category->name() == name) {
      return category;
    }
  }
  return NULL;
}

QList<OLSProjectFileCategory*> OLSProjectFile::categories() const {

  return m_categories;
}

QVariant OLSProjectFile::serialize() const {

  QVariantMap jsonMap;
  jsonMap["name"] = m_name;
  jsonMap["path"] = m_path;
  jsonMap["ontology"] = m_ontologyController->serialize();

  QVariantList categoriesJson;
  foreach (OLSProjectFileCategory *category, m_categories) {
    categoriesJson.append(category->serialize());
  }
  jsonMap["categories"] = categoriesJson;
  return jsonMap;
}

void OLSProjectFile::deserialize(const QVariant &json) {

  QVariantMap jsonMap = json.toMap();

  m_name = jsonMap["name"].toString();
  m_path = jsonMap["path"].toString();
  m_ontologyController->deserialize(jsonMap["ontology"]);

  m_categories.clear();

  QVariantList categoriesJson = jsonMap["categories"].toList();
  foreach (QVariant categoryJson, categoriesJson) {
    OLSProjectFileCategory *category = new OLSProjectFileCategory("", this);
    category->deserialize(categoryJson, this);
    m_categories.append(category);
  }
}

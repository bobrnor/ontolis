#include "OLSOntologyGraphElement.h"

#include <QDebug>
#include <QSTring>

OLSOntologyGraphElement::OLSOntologyGraphElement() {

  m_id = -1;
  m_name = "";
}

void OLSOntologyGraphElement::setId(long id) {

  m_id = id;
}

long OLSOntologyGraphElement::id() const {

  return m_id;
}

void OLSOntologyGraphElement::setName(const QString &name) {

  m_name = name;
}

QString OLSOntologyGraphElement::name() const {

  return m_name;
}

void OLSOntologyGraphElement::setRelatedDataController(OLSOntologyDataController *dataController) {

  m_dataController = dataController;
}

OLSOntologyDataController *OLSOntologyGraphElement::relatedDataController() const {

  return m_dataController;
}

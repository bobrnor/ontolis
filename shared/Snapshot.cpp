#include "Snapshot.h"

Snapshot::Snapshot() {

  m_sourceOntologySnapshot = NULL;
  m_destinationOntologySnapshot = NULL;
  m_problemsOntologySnapshot = NULL;
  m_logModelSnapshot = NULL;
}

Snapshot::~Snapshot() {

  if (m_sourceOntologySnapshot != NULL) {
    delete m_sourceOntologySnapshot;
  }

  if (m_destinationOntologySnapshot != NULL) {
    delete m_destinationOntologySnapshot;
  }

  if (m_problemsOntologySnapshot != NULL) {
    delete m_problemsOntologySnapshot;
  }

  if (m_logModelSnapshot != NULL) {
    delete m_logModelSnapshot;
  }
}

OntologyDataController *Snapshot::sourceOntologySnapshot() const {

  return m_sourceOntologySnapshot;
}

OntologyDataController *Snapshot::destinationOntologySnapshot() const {

  return m_destinationOntologySnapshot;
}

OntologyDataController *Snapshot::problemsOntologySnapshot() const {

  return m_problemsOntologySnapshot;
}

QStandardItemModel *Snapshot::logModelSnapshot() const {

  return m_logModelSnapshot;
}

void Snapshot::setSourceOntologySnapshot(OntologyDataController *snapshot) {

  m_sourceOntologySnapshot = snapshot;
}

void Snapshot::setDestinationOntologySnapshot(OntologyDataController *snapshot) {

  m_destinationOntologySnapshot = snapshot;
}

void Snapshot::setProblemsOntologySnapshot(OntologyDataController *snapshot) {

  m_problemsOntologySnapshot = snapshot;
}

void Snapshot::setLogModelSnapshot(QStandardItemModel *snapshot) {

  m_logModelSnapshot = snapshot;
}

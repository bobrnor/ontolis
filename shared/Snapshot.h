#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "core/OntologyDataController.h"

class Snapshot {

  private:
    OntologyDataController *m_sourceOntologySnapshot;
    OntologyDataController *m_destinationOntologySnapshot;
    OntologyDataController *m_problemsOntologySnapshot;
    QStandardItemModel *m_logModelSnapshot;

  public:
    Snapshot();
    ~Snapshot();

    OntologyDataController *sourceOntologySnapshot() const;
    OntologyDataController *destinationOntologySnapshot() const;
    OntologyDataController *problemsOntologySnapshot() const;
    QStandardItemModel *logModelSnapshot() const;

    void setSourceOntologySnapshot(OntologyDataController *snapshot);
    void setDestinationOntologySnapshot(OntologyDataController *snapshot);
    void setProblemsOntologySnapshot(OntologyDataController *snapshot);
    void setLogModelSnapshot(QStandardItemModel *snapshot);
};

#endif // SNAPSHOT_H

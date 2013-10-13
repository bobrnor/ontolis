#ifndef OLSPROJECTFILE_H
#define OLSPROJECTFILE_H

#include "../Shared/OLSOntologyDataController.h"

#include "OLSProjectFileCategory.h"

class OLSProjectFile {

    friend class OLSProject;

  private:
    QString m_name;
    QString m_path;

    OLSOntologyDataController *m_ontologyController;
    QList<OLSProjectFileCategory *> m_categories;

    void setName(const QString &name);
    void setPath(const QString &path);

  public:
    OLSProjectFile();
    OLSProjectFile(const QString &path, const QString &name);
    ~OLSProjectFile();

    const QString &name() const;
    const QString &path() const;

    OLSOntologyDataController *ontologyController() const;

    void addCategory(OLSProjectFileCategory *category);
    void removeCategory(OLSProjectFileCategory *category);
    OLSProjectFileCategory *getCategoryByName(const QString &name) const;
    QList<OLSProjectFileCategory*> categories() const;

    QVariant serialize() const;
    void deserialize(const QVariant &json);
};

#endif // OLSPROJECTFILE_H

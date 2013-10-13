#ifndef OLSPROJECT_H
#define OLSPROJECT_H

#include <QString>

#include "OLSProjectFile.h"

class OLSProject {

  private:
    QList<OLSProjectFile *> m_files;

    QVariant serialize() const;
    void deserialize(const QVariant &json);

  public:
    OLSProject();
    ~OLSProject();

    OLSProjectFile *createFile();
    OLSProjectFile *createFile(const QString &jsonString);
    OLSProjectFile *openFile(const QString &path);
    bool saveFile(OLSProjectFile *file, const QString &path);
    bool saveFile(OLSProjectFile *file);
    OLSProjectFile *getProjectFileByIndex(int index) const;
    OLSProjectFile *getProjectFileByName(const QString &name) const;
    int filesCount() const;

    bool openProject(const QString &path);
    bool saveProject(const QString &path);
};

#endif // OLSPROJECT_H

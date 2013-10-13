#ifndef OLSOLSProjectFileCATEGORY_H
#define OLSOLSProjectFileCATEGORY_H

#include <QSet>
#include <QString>
#include <QVariant>

class OLSProjectFile;

class OLSProjectFileCategory {

  private:
    QString m_name;
    QSet<long> m_relatedNodeIds;
    OLSProjectFile *m_parentFile;

  public:
    OLSProjectFileCategory(QString name, OLSProjectFile *parentFile);

    void setName(QString name);
    QString name() const;

    bool addRelatedNodeId(long id);
    void setRelatedNodeIds(QSet<long> ids);
    QSet<long> relatedNodeIds() const;

    QVariant serialize() const;
    void deserialize(const QVariant &json, OLSProjectFile *parentFile);

    bool operator ==(const OLSProjectFileCategory &category);
};

#endif // OLSOLSProjectFileCATEGORY_H

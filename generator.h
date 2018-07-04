#ifndef GENERATOR_H
#define GENERATOR_H

#include <QString>
#include <QHash>

class Generator
{
public:
    static QString generateModel(QString sql, const QHash<QString, QString> &hash);
    static QString generateDTO(QString sql, const QHash<QString, QString> &hash);
};

#endif // GENERATOR_H

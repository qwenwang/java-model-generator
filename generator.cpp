#include "generator.h"
#include <QRegExp>
#include <QDebug>
#include "stringutil.h"
#include <QPair>
#include <QString>

QString Generator::generateDTO(QString sql, const QHash<QString, QString> &hash) {
    QRegExp tableHeader("\\s*create\\s+table\\s+([\\w_]+)\\s+\\(", Qt::CaseInsensitive);
    int headerIndex = tableHeader.indexIn(sql);
    if(headerIndex < 0) {
        return "";
    }
    headerIndex = headerIndex+tableHeader.matchedLength();
    qDebug() << headerIndex;
    QString tableName = StringUtil::toCapitalize(StringUtil::snakeToCamel(tableHeader.cap(1)));
    qDebug() << tableName;
    QString columnStr = sql.mid(headerIndex, sql.lastIndexOf(")")-headerIndex);
    QStringList columns = columnStr.split(QRegExp(",\\s*"));
    QRegExp unavail("\\(|\\)");
    QList<QPair<QString, QString> > list;
    bool open = false;
    foreach(QString column, columns) {
        column = column.trimmed();
        int openIndex = unavail.lastIndexIn(column);
        bool lastOpen = open;
        if(openIndex >= 0) {
            if(column[openIndex] == "(") {
                open = true;
            } else {
                open = false;
            }
        }
        if(column.startsWith("constraint", Qt::CaseInsensitive)) {
            continue;
        }
        if(lastOpen) {
            continue;
        }
        QStringList columnParts = column.split(" ", QString::SkipEmptyParts);
        columnParts[0] = StringUtil::snakeToCamel(columnParts[0]);
        columnParts[1] = columnParts[1].section("(", 0, 0).toLower();
        list.append(qMakePair(columnParts[1], columnParts[0]));
    }
    qDebug() << columnStr;
    for(auto pair: list) {
        qDebug() << pair.first << pair.second;
    }
    QString dtoDef = QString("public class %1 implements Serializable {\n").arg(tableName);
    for(auto pair: list) {
        dtoDef += QString("\tprivate %1 %2;\n").arg(hash[pair.first], pair.second);
    }
    dtoDef += "}\n";
    return dtoDef;
}

QString Generator::generateModel(QString sql, const QHash<QString, QString> &hash) {
    return "";
}

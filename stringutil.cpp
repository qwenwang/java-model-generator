#include "stringutil.h"
#include <QStringList>

QString StringUtil::snakeToCamel(QString str)
{
    QStringList parts = str.split('_', QString::SkipEmptyParts);
    for (int i=1; i<parts.size(); ++i)
        parts[i].replace(0, 1, parts[i][0].toUpper());

    return parts.join("");
}

QString StringUtil::toCapitalize(QString str)
{
    return str[0].toUpper()+str.mid(1);
}

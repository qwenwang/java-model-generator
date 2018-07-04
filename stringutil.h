#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <QString>

class StringUtil
{
public:
    static QString snakeToCamel(QString str);

    static QString toCapitalize(QString str);
};

#endif // STRINGUTIL_H

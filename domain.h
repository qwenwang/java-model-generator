#ifndef DOMAIN_H
#define DOMAIN_H

#include "basetable.h"

class Domain : public BaseEntity {
public:
    void setDomainName(QString domainName){set("domainName", domainName);}
    QString getDomainName() const {return get("domainName").toString();}
    void setDomainType(QString domainType){set("domainType", domainType);}
    QString getDomainType() const {return get("domainType").toString();}
};

#endif // DOMAIN_H

#ifndef IPFMODELERPROCESSCHILD_H
#define IPFMODELERPROCESSCHILD_H

#include "ipfModelerProcessBase.h"
class ipfModelerProcessChild : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChild(QObject *parent, const QString modelerName);
	~ipfModelerProcessChild();

	ipfModelerProcessChild* classType() { return this; };
};

#endif IPFMODELERPROCESSCHILD_H
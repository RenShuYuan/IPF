#ifndef IPFMODELERPROCESSIN_H
#define IPFMODELERPROCESSIN_H

#include "ipfModelerProcessBase.h"

class ipfModelerInDialog;

class ipfModelerProcessIn : public ipfModelerProcessBase
{
public:
	ipfModelerProcessIn(QObject *parent, const QString modelerName);
	~ipfModelerProcessIn();

	ipfModelerProcessIn* classType() { return this; };
	const QString typeName() { return QString("IN"); };

	void setParameter();
	bool checkParameter();

private:
	ipfModelerInDialog *in;
};

#endif IPFMODELERPROCESSIN_H
#ifndef IPFMODELERPROCESSCHILDPROCESSRASTERRECTPOSITION_H
#define IPFMODELERPROCESSCHILDPROCESSRASTERRECTPOSITION_H

#include "ipfModelerProcessBase.h"
class ipfModelerProcessChildProcessRasterRectPosition : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildProcessRasterRectPosition(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildProcessRasterRectPosition();

	ipfModelerProcessChildProcessRasterRectPosition* classType() { return this; };

	bool checkParameter();
	void setParameter();

	void run();
};

#endif // IPFMODELERPROCESSCHILDPROCESSRASTERRECTPOSITION_H
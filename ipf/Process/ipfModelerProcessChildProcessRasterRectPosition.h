#ifndef IPFMODELERPROCESSCHILDPROCESSRASTERRECTPOSITION_H
#define IPFMODELERPROCESSCHILDPROCESSRASTERRECTPOSITION_H

#include "ipfModelerProcessOut.h"
class ipfModelerProcessChildProcessRasterRectPosition : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildProcessRasterRectPosition(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildProcessRasterRectPosition();

	ipfModelerProcessChildProcessRasterRectPosition* classType() { return this; };

	bool checkParameter();
	void setParameter();

	void run();
private:
	QString adjustRange(const QString &fileName);
};

#endif // IPFMODELERPROCESSCHILDPROCESSRASTERRECTPOSITION_H
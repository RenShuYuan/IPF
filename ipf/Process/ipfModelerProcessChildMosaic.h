#ifndef IPFMODELERPROCESSCHILDMOSAIC_H
#define IPFMODELERPROCESSCHILDMOSAIC_H

#include "ipfModelerProcessBase.h"

class ipfModelerMosaicDialog;

class ipfModelerProcessChildMosaic : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildMosaic(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildMosaic();

	ipfModelerProcessChildMosaic* classType() { return this; };

	bool checkParameter();
	void setParameter();

	void run();

private:
	ipfModelerMosaicDialog * mosaic;
};

#endif // IPFMODELERPROCESSCHILDMOSAIC_H
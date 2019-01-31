#ifndef IPFMODELERPROCESSCHILDCREATETFW_H
#define IPFMODELERPROCESSCHILDCREATETFW_H

#include "ipfModelerProcessOut.h"
class ipfModelerProcessChildCreateTfw : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildCreateTfw(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildCreateTfw();

	ipfModelerProcessChildCreateTfw* classType() { return this; };

	bool checkParameter();
	void setParameter();

	void run();

private:
};

#endif // IPFMODELERPROCESSCHILDCREATETFW_H
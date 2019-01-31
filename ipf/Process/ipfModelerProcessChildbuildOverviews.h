#ifndef IPFMODELERPROCESSCHILDBUILDOVERVIEWS_H
#define IPFMODELERPROCESSCHILDBUILDOVERVIEWS_H

#include "ipfModelerProcessOut.h"

class ipfModelerProcessChildbuildOverviews :
	public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildbuildOverviews(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildbuildOverviews();

	ipfModelerProcessChildbuildOverviews* classType() { return this; };

	bool checkParameter();
	void setParameter();

	void run();
};

#endif // IPFMODELERPROCESSCHILDBUILDOVERVIEWS_H
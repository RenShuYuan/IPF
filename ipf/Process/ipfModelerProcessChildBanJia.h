#ifndef IPFMODELERPROCESSCHILDBANJIA_H
#define IPFMODELERPROCESSCHILDBANJIA_H

#include "ipfModelerProcessOut.h"

class ipfModelerProcessChildBanJia : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildBanJia(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildBanJia();

	ipfModelerProcessChildBanJia* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();
};

#endif // IPFMODELERPROCESSCHILDBANJIA_H
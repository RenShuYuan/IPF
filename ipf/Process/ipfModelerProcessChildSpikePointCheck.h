#ifndef IPFMODELERPROCESSCHILDSPIKEPOINTCHECK_H
#define IPFMODELERPROCESSCHILDSPIKEPOINTCHECK_H

#include "ipfModelerProcessOut.h"

class ipfModelerSpikePointCheckDialog;

class ipfModelerProcessChildSpikePointCheck : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildSpikePointCheck(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildSpikePointCheck();

	ipfModelerProcessChildSpikePointCheck* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();
private:
	ipfModelerSpikePointCheckDialog * dialog;
	QString saveName;
	double threshold;
};

#endif // IPFMODELERPROCESSCHILDSPIKEPOINTCHECK_H
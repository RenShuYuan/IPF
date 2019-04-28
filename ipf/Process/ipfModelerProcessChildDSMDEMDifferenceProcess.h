#ifndef IPFMODELERPROCESSCHILDDSMDEMDIFFERENCEPROCESS_H
#define IPFMODELERPROCESSCHILDDSMDEMDIFFERENCEPROCESS_H

#include "ipfModelerProcessBase.h"

class ipfModelerDSMDEMDifferenceProcessDialog;

class ipfModelerProcessChildDSMDEMDifferenceProcess : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildDSMDEMDifferenceProcess(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildDSMDEMDifferenceProcess();

	ipfModelerProcessChildDSMDEMDifferenceProcess* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();
private:
	int getFilesIndex(const QStringList & lists, const QString & th);

private:
	ipfModelerDSMDEMDifferenceProcessDialog * dialog;

	QString typeName;
	double threshold;
	bool isFillNodata;
};

#endif // IPFMODELERPROCESSCHILDDSMDEMDIFFERENCEPROCESS_H
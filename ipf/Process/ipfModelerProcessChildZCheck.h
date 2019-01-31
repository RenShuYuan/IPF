#ifndef IPFMODELERPROCESSCHILDZCHECK_H
#define IPFMODELERPROCESSCHILDZCHECK_H

#include "ipfModelerProcessOut.h"

class ipfModelerZCheckDialog;

class ipfModelerProcessChildZCheck : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildZCheck(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildZCheck();

	ipfModelerProcessChildZCheck* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerZCheckDialog * dialog;
	QString flies;
	QString saveName;
};

#endif // IPFMODELERPROCESSCHILDZCHECK_H
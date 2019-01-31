#ifndef IPFMODELERPROCESSCHILDPROJECTIONCHECK_H
#define IPFMODELERPROCESSCHILDPROJECTIONCHECK_H

#include "ipfModelerProcessOut.h"

class ipfModelerProjectionCheckDialog;

class ipfModelerProcessChildProjectionCheck : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildProjectionCheck(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildProjectionCheck();

	ipfModelerProcessChildProjectionCheck* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerProjectionCheckDialog * dialog;
	QString s_srs;
	QString saveName;
};

#endif // IPFMODELERPROCESSCHILDPROJECTIONCHECK_H
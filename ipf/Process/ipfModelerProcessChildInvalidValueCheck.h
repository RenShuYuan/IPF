#ifndef IPFMODELERPROCESSCHILDINVALIDVALUECHECK_H
#define IPFMODELERPROCESSCHILDINVALIDVALUECHECK_H

#include "ipfModelerProcessOut.h"

class ipfModelerInvalidValueCheckDialog;

class ipfModelerProcessChildInvalidValueCheck : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildInvalidValueCheck(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildInvalidValueCheck();

	ipfModelerProcessChildInvalidValueCheck* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();
private:
	ipfModelerInvalidValueCheckDialog * dialog;
	QString saveName;
	QString invalidValue;
	bool isNegative;
};

#endif // IPFMODELERPROCESSCHILDINVALIDVALUECHECK_H
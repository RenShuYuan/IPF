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
	QString invalidValue;
	QString saveName;
	bool isNegative;
	bool isNodata;
	bool isShape;
	bool bands_noDiffe;
};

#endif // IPFMODELERPROCESSCHILDINVALIDVALUECHECK_H
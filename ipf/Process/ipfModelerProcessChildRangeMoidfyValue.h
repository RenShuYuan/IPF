#ifndef IPFMODELERPROCESSCHILDRANGEMOIDFYVALUE_H
#define IPFMODELERPROCESSCHILDRANGEMOIDFYVALUE_H

#include "ipfModelerProcessBase.h"

class ipfModelerRangeMoidfyValueDialog;

class ipfModelerProcessChildRangeMoidfyValue : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildRangeMoidfyValue(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildRangeMoidfyValue();

	ipfModelerProcessChildRangeMoidfyValue* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();
private:
	ipfModelerRangeMoidfyValueDialog * dialog;

	QString vectorName;
	double value;
};

#endif // IPFMODELERPROCESSCHILDRANGEMOIDFYVALUE_H
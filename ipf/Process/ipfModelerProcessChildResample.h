#ifndef IPFMODELERPROCESSCHILDRESAMPLE_H
#define IPFMODELERPROCESSCHILDRESAMPLE_H

#include "ipfModelerProcessBase.h"

class ipfModelerResampleDialog;

class ipfModelerProcessChildResample : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildResample(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildResample();

	ipfModelerProcessChildResample* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerResampleDialog * dialog;
	QString resampling_method;
	double res;
};

#endif // IPFMODELERPROCESSCHILDRESAMPLE_H
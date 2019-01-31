#ifndef IPFMODELERPROCESSCHILDDEMGROSSERRORCHECK_H
#define IPFMODELERPROCESSCHILDDEMGROSSERRORCHECK_H

#include "ipfModelerProcessOut.h"

class ipfModelerDemGrossErrorCheckDialog;

class ipfModelerProcessChildDemGrossErrorCheck : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildDemGrossErrorCheck(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildDemGrossErrorCheck();

	ipfModelerProcessChildDemGrossErrorCheck* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerDemGrossErrorCheckDialog * dialog;
	QString rasterName;
	QString errFile;
	double threshold;
};

#endif // IPFMODELERPROCESSCHILDDEMGROSSERRORCHECK_H
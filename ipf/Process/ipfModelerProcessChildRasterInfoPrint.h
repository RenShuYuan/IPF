#ifndef IPFMODELERPROCESSCHILDRASTERINFOPRINT_H
#define IPFMODELERPROCESSCHILDRASTERINFOPRINT_H

#include "ipfModelerProcessOut.h"

class ipfModelerRasterInfoPrintDialog;

class ipfModelerProcessChildRasterInfoPrint : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildRasterInfoPrint(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildRasterInfoPrint();

	ipfModelerProcessChildRasterInfoPrint* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerRasterInfoPrintDialog * dialog;
	QString saveName;
};

#endif // IPFMODELERPROCESSCHILDRASTERINFOPRINT_H
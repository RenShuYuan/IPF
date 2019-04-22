#ifndef IPFMODELERPROCESSOUT_H
#define IPFMODELERPROCESSOUT_H

#include "ipfModelerProcessBase.h"
#include "head.h"

class ipfModelerOutDialog;

class ipfModelerProcessOut : public ipfModelerProcessBase
{
public:
	ipfModelerProcessOut(QObject *parent, const QString modelerName);
	~ipfModelerProcessOut();

	ipfModelerProcessOut* classType() { return this; };
	const QString typeName() { return QString("OUT"); };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	bool printErrToFile(const QString &fileName, const QStringList &errList);

	void run();

private:
	QMap<QString, QString> map;
	ipfModelerOutDialog *out;

	QString format;
	QString outPath;
	QString compress;
	QString isTfw;
	QString noData;
};

#endif IPFMODELERPROCESSOUT_H
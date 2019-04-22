#ifndef IPFMODELERPROCESSCHILDDSMDEMDIFFERENCECHECK_H
#define IPFMODELERPROCESSCHILDDSMDEMDIFFERENCECHECK_H

#include "ipfModelerProcessOut.h"

class ipfModelerOutDialog;

class ipfModelerProcessChildDSMDEMDifferenceCheck : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildDSMDEMDifferenceCheck(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildDSMDEMDifferenceCheck();

	ipfModelerProcessChildDSMDEMDifferenceCheck* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();
private:
	QString compareRastersDiff(const QString &oneRaster, const QString &twoRaster, QString &raster);

private:
	QMap<QString, QString> map;
	ipfModelerOutDialog *out;

	QString format;
	QString outPath;
	QString compress;
	QString isTfw;
	QString noData;
};

#endif IPFMODELERPROCESSCHILDDSMDEMDIFFERENCECHECK_H
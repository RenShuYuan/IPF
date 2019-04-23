#ifndef IPFMODELERPROCESSCHILDDSMDEMDIFFERENCECHECK_H
#define IPFMODELERPROCESSCHILDDSMDEMDIFFERENCECHECK_H

#include "ipfModelerProcessOut.h"

class ipfModelerPrintErrRasterDialog;

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
	int getFilesIndex(const QStringList & lists, const QString & th);

private:
	QMap<QString, QString> map;
	ipfModelerPrintErrRasterDialog *dialog;

	QString saveName;
};

#endif IPFMODELERPROCESSCHILDDSMDEMDIFFERENCECHECK_H
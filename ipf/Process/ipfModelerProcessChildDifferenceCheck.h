#ifndef IPFMODELERPROCESSCHILDDIFFERENCECHECK_H
#define ipfModelerProcessChildDifferenceCheck_H

#include "ipfModelerProcessOut.h"

class ipfModelerPrintErrRasterDialog;

class ipfModelerProcessChildDifferenceCheck : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildDifferenceCheck(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildDifferenceCheck();

	ipfModelerProcessChildDifferenceCheck* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	QString compareRastersDiff(const QString &oneRaster, const QString &twoRaster, QStringList &returnRasters);

	// 在数据列表中查找对应图幅
	int getFilesIndex(const QStringList &lists, const QString &th);

private:
	ipfModelerPrintErrRasterDialog *dialog;

	QString saveName;
	double threshold;
};

#endif // ipfModelerProcessChildDifferenceCheck
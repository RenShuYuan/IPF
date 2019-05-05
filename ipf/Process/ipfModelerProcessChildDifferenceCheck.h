#ifndef IPFMODELERPROCESSCHILDDIFFERENCECHECK_H
#define ipfModelerProcessChildDifferenceCheck_H

#include "ipfModelerProcessOut.h"

class ipfModelerDiffeCheckDialog;

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

	// �������б��в��Ҷ�Ӧͼ��
	int getFilesIndex(const QStringList &lists, const QString &th);

private:
	ipfModelerDiffeCheckDialog *dialog;

	QString saveName;
	double valueMax;
};

#endif // ipfModelerProcessChildDifferenceCheck
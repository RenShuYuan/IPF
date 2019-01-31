#ifndef IPFMODELERPROCESSCHILDWATERFLATTEN_H
#define IPFMODELERPROCESSCHILDWATERFLATTEN_H

#include "ipfModelerProcessOut.h"

class ipfModelerWaterFlattenDialog;

class ipfModelerProcessChildWaterFlatten : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildWaterFlatten(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildWaterFlatten();

	ipfModelerProcessChildWaterFlatten* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	bool splitShp(const QString &shpName, QStringList &shps);

	// ��ȡshp�ļ��е�һ��Ҫ�صľ��η�Χ, ������һ������
	QList<double> shpEnvelopeOne(const QString &file, const double R);

private:
	ipfModelerWaterFlattenDialog * dialog;
	QString vectorName;
	QString outPath;
};

#endif // IPFMODELERPROCESSCHILDWATERFLATTEN_H
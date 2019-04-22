#ifndef IPFMODELERPROCESSCHILDSETNODATA_H
#define IPFMODELERPROCESSCHILDSETNODATA_H

#include "ipfModelerProcessOut.h"

class ipfModelerSetNodataDialog;

class ipfModelerProcessChildSetNodata : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildSetNodata(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildSetNodata();

	ipfModelerProcessChildSetNodata* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerSetNodataDialog * dialog;

	// ��34743735.938444�����û��Ƿ�������Чֵ
	double nodata;
	bool isDel;
};

#endif IPFMODELERPROCESSCHILDSETNODATA_H
#ifndef IPFMODELERPROCESSCHILDTYPECONVERT_H
#define IPFMODELERPROCESSCHILDTYPECONVERT_H

#include "ipfModelerProcessBase.h"

class ipfModelerTypeConvertDialog;

class ipfModelerProcessChildTypeConvert : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildTypeConvert(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildTypeConvert();

	ipfModelerProcessChildTypeConvert* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerTypeConvertDialog * typeConvert;
	QString dataType;
};

#endif // IPFMODELERPROCESSCHILDTYPECONVERT_H
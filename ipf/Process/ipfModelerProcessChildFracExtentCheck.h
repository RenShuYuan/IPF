#ifndef IPFMODELERPROCESSCHILDFRACEXTENTCHECK_H
#define IPFMODELERPROCESSCHILDFRACEXTENTCHECK_H

#include "ipfModelerProcessOut.h"

class ipfModelerFracExtentCheckDialog;

class ipfModelerProcessChildFracExtentCheck : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildFracExtentCheck(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildFracExtentCheck();

	ipfModelerProcessChildFracExtentCheck* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerFracExtentCheckDialog *dialog;
	QString saveName;
};

#endif // IPFMODELERPROCESSCHILDFRACEXTENTCHECK_H
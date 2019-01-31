#ifndef IPFMODELERPROCESSCHILDQUICKVIEW_H
#define IPFMODELERPROCESSCHILDQUICKVIEW_H

#include "ipfModelerProcessBase.h"

class ipfModelerQuickViewDialog;

class ipfModelerProcessChildQuickView : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildQuickView(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildQuickView();

	ipfModelerProcessChildQuickView* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerQuickViewDialog * quickView;
	int bs;
};

#endif // IPFMODELERPROCESSCHILDQUICKVIEW_H
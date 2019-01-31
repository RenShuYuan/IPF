#ifndef IPFMODELERPROCESSCHILDPIXELDECIMALS_H
#define IPFMODELERPROCESSCHILDPIXELDECIMALS_H

#include "ipfModelerProcessBase.h"

class ipfModelerPixelDecimalsDialog;

class ipfModelerProcessChildPixelDecimals : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildPixelDecimals(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildPixelDecimals();

	ipfModelerProcessChildPixelDecimals* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerPixelDecimalsDialog * dialog;
	int decimals;
};

#endif // IPFMODELERPROCESSCHILDPIXELDECIMALS_H
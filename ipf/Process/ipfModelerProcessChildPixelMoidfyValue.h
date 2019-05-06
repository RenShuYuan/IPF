#ifndef IPFMODELERPROCESSCHILDPIXELMOIDFYVALUE_H
#define IPFMODELERPROCESSCHILDPIXELMOIDFYVALUE_H

#include "ipfModelerProcessBase.h"

class ipfModelerPixelMoidfyValueDialog;

class ipfModelerProcessChildPixelMoidfyValue : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildPixelMoidfyValue(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildPixelMoidfyValue();

	ipfModelerProcessChildPixelMoidfyValue* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerPixelMoidfyValueDialog * dialog;
	double oldValue_1;
	double newValue_1;
	double oldValue_2;
	double newValue_2;
	bool bands_noDiffe;
};

#endif // IPFMODELERPROCESSCHILDPIXELMOIDFYVALUE_H
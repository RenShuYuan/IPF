#ifndef IPFMODELERPROCESSCHILDCLIPVECTOR_H
#define IPFMODELERPROCESSCHILDCLIPVECTOR_H

#include "ipfModelerProcessBase.h"

class ipfModelerClipVectorDialog;

class ipfModelerProcessChildClipVector : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildClipVector(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildClipVector();
	ipfModelerProcessChildClipVector* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerClipVectorDialog * clip;
	QString vectorName;
};

#endif // IPFMODELERPROCESSCHILDCLIPVECTOR_H
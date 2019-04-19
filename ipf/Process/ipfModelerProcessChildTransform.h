#ifndef IPFMODELERPROCESSCHILDTRANSFORM_H
#define IPFMODELERPROCESSCHILDTRANSFORM_H

#include "ipfModelerProcessBase.h"

class ipfModelerTransformDialog;

class ipfModelerProcessChildTransform : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildTransform(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildTransform();

	ipfModelerProcessChildTransform* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerTransformDialog * dialog;
	QString resampling_method;
	QString s_srs;
	QString t_srs;
};

#endif // IPFMODELERPROCESSCHILDTRANSFORM_H
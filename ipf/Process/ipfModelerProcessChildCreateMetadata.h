#ifndef IPFMODELERPROCESSCHILDZJMETADATA_H
#define IPFMODELERPROCESSCHILDZJMETADATA_H

#include "ipfModelerProcessOut.h"

class ipfModelerCreateMetadataDialog;

class ipfModelerProcessChildCreateMetadata : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildCreateMetadata(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildCreateMetadata();

	ipfModelerProcessChildCreateMetadata* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	void createZjMetaData(const QString &var);
	void createDsmMetaData(const QString &var);
	void createDemMetaData(const QString &var);
	void createDomMetaData(const QString &var);

private:
	ipfModelerCreateMetadataDialog *dialog;
	QString metaDataType;
	QString outPath;
	QString sample;
};

#endif // IPFMODELERPROCESSCHILDZJMETADATA_H
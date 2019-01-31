#ifndef LPFDATABASEMANAGEMENT_H
#define LPFDATABASEMANAGEMENT_H

#include <QObject>
#include "head.h"

class lpfDatabaseManagement : public QObject
{
	Q_OBJECT
public:
	lpfDatabaseManagement(QObject *parent = nullptr);
	~lpfDatabaseManagement();

	void openIPFDatabase(const QString& layerName);

	QString getIpfDatabasePath();

	QStringList getAllLayer();
	
	void addvectorLayer(const QString& layerName);

private:
	GDALDataset * poDS;
	QString databaseName;
	QStringList mLayers;
};

#endif // LPFDATABASEMANAGEMENT_H
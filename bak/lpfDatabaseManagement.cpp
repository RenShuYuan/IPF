#include "lpfDatabaseManagement.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

lpfDatabaseManagement::lpfDatabaseManagement(QObject *parent)
	: QObject(parent)
	, poDS(nullptr)
{
	databaseName = QDir::currentPath() + "/Resources/IPF/IPF.gpkg";
	openIPFDatabase(databaseName);
}

lpfDatabaseManagement::~lpfDatabaseManagement()
{
	if (poDS) GDALClose(poDS);
}

void lpfDatabaseManagement::openIPFDatabase(const QString & layerName)
{
	if (!QFile(layerName).exists(layerName))
	{
		QMessageBox::critical(nullptr, QStringLiteral("错误"), QStringLiteral("请确认databaseName存在，或重新安装。"));
		return;
	}

	OGRSFDriverH hGpkgDriver = OGRGetDriverByName("GPKG");
	if (!hGpkgDriver)
	{
		QMessageBox::critical(nullptr, tr("Layer creation failed"), tr("GeoPackage driver not found"));
		return;
	}

	GDALDataset* mDs = nullptr;
	mDs = (GDALDataset*)GDALOpenEx(layerName.toUtf8().constData(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (mDs==NULL)
	{
		QString msg(tr("Opening of database failed (OGR error:%1)").arg(QString::fromUtf8(CPLGetLastErrorMsg())));
		QMessageBox::critical(nullptr, tr("Layer creation failed"), msg);
		return;
	}

	if (poDS) GDALClose(poDS);
	poDS = mDs;
}

QString lpfDatabaseManagement::getIpfDatabasePath()
{
	return databaseName;
}

QStringList lpfDatabaseManagement::getAllLayer()
{
	QStringList layers;

	if (!poDS) return layers;

	int layerCount = poDS->GetLayerCount();
	for (int i=0; i<layerCount; ++i)
	{
		OGRLayer *poLayer = poDS->GetLayer(i);
		layers.append(poLayer->GetName());
	}
	return layers;
}

void lpfDatabaseManagement::addvectorLayer(const QString & layerName)
{

}

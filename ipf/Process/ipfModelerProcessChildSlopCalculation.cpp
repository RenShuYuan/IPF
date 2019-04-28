#include "ipfModelerProcessChildSlopCalculation.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"

ipfModelerProcessChildSlopCalculation::ipfModelerProcessChildSlopCalculation(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
}


ipfModelerProcessChildSlopCalculation::~ipfModelerProcessChildSlopCalculation()
{
}

bool ipfModelerProcessChildSlopCalculation::checkParameter()
{
	return true;
}

void ipfModelerProcessChildSlopCalculation::setParameter()
{
}

QMap<QString, QString> ipfModelerProcessChildSlopCalculation::getParameter()
{
	return QMap<QString, QString>();
}

void ipfModelerProcessChildSlopCalculation::setDialogParameter(QMap<QString, QString> map)
{
}

void ipfModelerProcessChildSlopCalculation::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		QString dsm = "d:/dsm.tif";
		QString dem = "d:/dem.tif";
		QString target = ipfFlowManage::instance()->getTempFormatFile(var, ".vrt");
		QString err = gdal.dsmdemDiffeProcess(dsm, dem, target, "DEM", 50, false);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

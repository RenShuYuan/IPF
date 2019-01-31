#include "ipfModelerProcessChildMosaic.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerMosaicDialog.h"

ipfModelerProcessChildMosaic::ipfModelerProcessChildMosaic(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	mosaic = new ipfModelerMosaicDialog();
}


ipfModelerProcessChildMosaic::~ipfModelerProcessChildMosaic()
{
	RELEASE(mosaic);
}

bool ipfModelerProcessChildMosaic::checkParameter()
{
	return true;
}

void ipfModelerProcessChildMosaic::setParameter()
{
	mosaic->exec();
}

void ipfModelerProcessChildMosaic::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(1);
	gdal.showProgressDialog();

	QString target = ipfFlowManage::instance()->getTempVrtFile(QStringLiteral("mosaic"));
	QString err = gdal.mosaic_Buildvrt(filesIn(), target);
	//QString err = gdal.mosaic_Warp(filesIn(), target);
	if (err.isEmpty())
		appendOutFile(target);
	else
		addErrList("mosaic: " + err);
}

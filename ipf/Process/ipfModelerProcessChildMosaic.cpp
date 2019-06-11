#include "ipfModelerProcessChildMosaic.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerMosaicDialog.h"
#include "../ipfOgr.h"

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

	int bands = -1;
	QStringList inList;
	foreach ( QString str, filesIn() )
	{
		ipfOGR ogr(str);
		if (!ogr.isOpen())
		{
			addErrList(str + QStringLiteral(": 读取影像失败，已跳过。"));
			continue;
		}
		int mBands = ogr.getBandSize();

		if (bands == -1)
			bands = mBands;
		else
		{
			if (bands != mBands)
			{
				addErrList(str + QStringLiteral(": 栅格波段数量不一致，已跳过。"));
				continue;
			}
		}

		bool isbl = true;
		QString target = ipfFlowManage::instance()->getTempVrtFile(str);
		QString err = gdal.formatConvert(str, target, gdal.enumFormatToString("vrt"), "NONE", "NO", "none");
		ipfOGR oggg(target, true);
		for (int i = 1; i <= mBands; ++i)
		{
			if (CE_None != oggg.getRasterBand(i)->SetColorInterpretation(GCI_Undefined))
			{
				isbl = false;
				break;
			}
		}
		if (isbl)
			inList << str;

		//QString target = ipfFlowManage::instance()->getTempVrtFile(str);
		//QString err = gdal.clearColorInterp(str, target, mBands);
		//if (err.isEmpty())
		//	inList << target;
		//else
		//	addErrList(str + ": " + err);

		
	}

	QString target = ipfFlowManage::instance()->getTempVrtFile("mosaic");
	QString err = gdal.mosaic_Buildvrt(inList, target);
	if (err.isEmpty())
		appendOutFile(target);
	else
		addErrList("mosaic: " + err);
}

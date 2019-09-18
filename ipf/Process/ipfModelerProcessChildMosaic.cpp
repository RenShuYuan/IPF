#include "ipfModelerProcessChildMosaic.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerMosaicDialog.h"
#include "../ipfOgr.h"
#include "qgsrasterlayer.h"

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
/*
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(1);
	gdal.showProgressDialog();

	int bands = -1;
	QStringList inList;
	foreach ( QString str, filesIn() )
	{
		ipfOGR ogr(str, true);
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
		QString target = ipfApplication::instance()->getTempVrtFile(str);
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
		oggg.close();
		if (isbl)
			inList << target;

		//bool isbl = true;
		//for (int i = 1; i <= mBands; ++i)
		//{
		//	if (CE_None != ogr.getRasterBand(i)->SetColorInterpretation(GDALColorInterp::GCI_SaturationBand))
		//	{
		//		isbl = false;
		//		break;
		//	}
		//}
		//if (isbl)
		//	inList << str;
	}

	QString target = ipfApplication::instance()->getTempVrtFile("mosaic");
	QString err = gdal.mosaic_Buildvrt(inList, target);
	if (err.isEmpty())
		appendOutFile(target);
	else
		addErrList("mosaic: " + err);
*/

	clearOutFiles();
	clearErrList();

	int bands = -1;
	QgsCoordinateReferenceSystem crs;
	QStringList inList;

	// 过滤信息不一致数据
	foreach(QString str, filesIn())
	{
		QgsRasterLayer* layer = new QgsRasterLayer(str, "rasterLayer", "gdal");
		if (!layer || !layer->isValid())
		{
			addErrList(str + QStringLiteral(": 栅格数据读取失败。"));
			continue;
		}

		// 检查波段数量
		if (bands == -1)
			bands = layer->bandCount();
		else
		{
			if (bands != layer->bandCount())
			{
				addErrList(str + QStringLiteral(": 栅格波段数量不一致，已跳过。"));
				RELEASE(layer);
				continue;
			}
		}

		// 检查空间坐标信息
		if (!crs.isValid())
			crs = layer->crs();
		else
		{
			if (crs != layer->crs())
			{
				addErrList(str + QStringLiteral(": 栅格空间信息不一致，已跳过。"));
				RELEASE(layer);
				continue;
			}
		}

		RELEASE(layer);
		inList << str;
	}

	// 转VRT文件
	ipfGdalProgressTools gdal;
	gdal.setProgressSize(inList.size());
	gdal.showProgressDialog();
	QStringList vrtList;
	for (auto str : inList)
	{
		QString target = ipfApplication::instance()->getTempVrtFile(str);
		QString err = gdal.formatConvert(str, target, gdal.enumFormatToString("vrt"), "NONE", "NO", "none");
		if (!err.isEmpty())
		{
			addErrList(str + QStringLiteral(": 转换临时栅格文件失败，请自行核查该数据 -1。"));
			continue;
		}
		vrtList << str;
	}

	// 修改颜色表
	QStringList outList;
	for (auto str : vrtList)
	{
		ipfOGR ogr(str);
		if (!ogr.isOpen())
		{
			addErrList(str + QStringLiteral(": 读取临时文件失败..."));
			continue;
		}
		for (int i = 1; i <= ogr.getBandSize(); ++i)
		{
			if (CE_None != ogr.getRasterBand(i)->SetColorInterpretation(GDALColorInterp::GCI_SaturationBand))
			{
				addErrList(str + QStringLiteral(": 修改栅格数据颜色表失败..."));
				continue;
			}
		}
		outList << str;
	}

	QString target = ipfApplication::instance()->getTempVrtFile("mosaic");
	QString err = gdal.mosaic_Buildvrt(outList, target);
	if (err.isEmpty())
		appendOutFile(target);
	else
		addErrList("mosaic: " + err);
}

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
			addErrList(str + QStringLiteral(": ��ȡӰ��ʧ�ܣ���������"));
			continue;
		}
		int mBands = ogr.getBandSize();

		if (bands == -1)
			bands = mBands;
		else
		{
			if (bands != mBands)
			{
				addErrList(str + QStringLiteral(": դ�񲨶�������һ�£���������"));
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

	// ������Ϣ��һ������
	foreach(QString str, filesIn())
	{
		QgsRasterLayer* layer = new QgsRasterLayer(str, "rasterLayer", "gdal");
		if (!layer || !layer->isValid())
		{
			addErrList(str + QStringLiteral(": դ�����ݶ�ȡʧ�ܡ�"));
			continue;
		}

		// ��鲨������
		if (bands == -1)
			bands = layer->bandCount();
		else
		{
			if (bands != layer->bandCount())
			{
				addErrList(str + QStringLiteral(": դ�񲨶�������һ�£���������"));
				RELEASE(layer);
				continue;
			}
		}

		// ���ռ�������Ϣ
		if (!crs.isValid())
			crs = layer->crs();
		else
		{
			if (crs != layer->crs())
			{
				addErrList(str + QStringLiteral(": դ��ռ���Ϣ��һ�£���������"));
				RELEASE(layer);
				continue;
			}
		}

		RELEASE(layer);
		inList << str;
	}

	// תVRT�ļ�
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
			addErrList(str + QStringLiteral(": ת����ʱդ���ļ�ʧ�ܣ������к˲������ -1��"));
			continue;
		}
		vrtList << str;
	}

	// �޸���ɫ��
	QStringList outList;
	for (auto str : vrtList)
	{
		ipfOGR ogr(str);
		if (!ogr.isOpen())
		{
			addErrList(str + QStringLiteral(": ��ȡ��ʱ�ļ�ʧ��..."));
			continue;
		}
		for (int i = 1; i <= ogr.getBandSize(); ++i)
		{
			if (CE_None != ogr.getRasterBand(i)->SetColorInterpretation(GDALColorInterp::GCI_SaturationBand))
			{
				addErrList(str + QStringLiteral(": �޸�դ��������ɫ��ʧ��..."));
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

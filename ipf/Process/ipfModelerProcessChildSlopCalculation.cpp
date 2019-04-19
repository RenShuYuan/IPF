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

	//ipfProgress proDialog;
	//proDialog.setRangeTotal(0, filesIn().size());

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		//QFileInfo info(var);
		//QString baseName = info.baseName();

		//// ��Դդ��
		//ipfOGR ogr(var);
		//if (!ogr.isOpen())
		//{
		//	addErrList(var + QStringLiteral(": ��ȡդ������ʧ�ܣ��޷�������"));
		//	continue;
		//}
		//int nBands = ogr.getBandSize();

		//// ʹ��3x3ģ�����դ���׼�� ----->
		//for (int i = 0; i < nBands; ++i)
		//{
		//	// �������դ��
		//	QString rasterFile = ipfFlowManage::instance()->getTempFormatFile(QString("%1@%2").arg(baseName).arg(i+1), ".tif");

			//GDALDataset* poDataset_target = ogr.createNewRaster(rasterFile, 1, GDT_Float32);
			//if (!poDataset_target)
			//{
			//	addErrList(rasterFile + QStringLiteral(": �������դ������ʧ�ܣ��޷�������"));
			//	continue;
			//}
			//GDALRasterBand* datasetBand = poDataset_target->GetRasterBand(1);
			//datasetBand->SetNoDataValue(-9999);

			//if (poDataset_target)
			//{
			//	GDALClose((GDALDatasetH)poDataset_target);
			//	poDataset_target = nullptr;
			//}


		//	QString err = gdal.stdevp3x3Alg(var, rasterFile, i + 1);
		//	if (err.isEmpty())
		//		appendOutFile(rasterFile);
		//	else
		//		addErrList(QStringLiteral("��׼�����: ") + err);
		//}

		QString target = ipfFlowManage::instance()->getTempFormatFile(var, ".vrt");
		QString err = gdal.slopCalculation_S2(var, target);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

#include "ipfModelerProcessChildConsistency.h"
//#include "qgsrastercalculator.h"
#include "../ipfFractalmanagement.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "ipfFlowManage.h"
#include "../ipfOgr.h"
#include "../../head.h"

#include "QgsRasterLayer.h"

#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

ipfModelerProcessChildConsistency::ipfModelerProcessChildConsistency
	(QObject *parent, const QString modelerName) : ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
}


ipfModelerProcessChildConsistency::~ipfModelerProcessChildConsistency()
{
}

bool ipfModelerProcessChildConsistency::checkParameter()
{
	return true;
}

void ipfModelerProcessChildConsistency::setParameter()
{
	QMessageBox::about(0, QStringLiteral("")
		, QStringLiteral(""));
}

void ipfModelerProcessChildConsistency::run()
{
	clearOutFiles();
	clearErrList();
	
	foreach(QString var, filesIn())
	{
		QFileInfo info(var);
		QString baseName = info.baseName();

		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡդ������ʧ�ܣ��޷�������"));
			continue;
		}

		int nBands = ogr.getBandSize();
		if (nBands != 3)
		{
			addErrList(var + QStringLiteral(": ��ʱֻ֧��3���Σ��޷�������"));
			continue;
		}

		int nXSize = ogr.getYXSize().at(1);
		int nYSize = ogr.getYXSize().at(0);

		// �������դ��
		QString rasterFile = "d:\\" + baseName + "_v.img";
		GDALDataset* poDataset_target = ogr.createNewRaster(rasterFile, "-9999", 1, GDT_Float32);
		if (!poDataset_target)
		{
			addErrList(rasterFile + QStringLiteral(": �������դ������ʧ�ܣ��޷�������"));
			continue;
		}
		GDALRasterBand* datasetBand = poDataset_target->GetRasterBand(1);

		long blockSize = nXSize * nYSize * nBands;
		double *pSrcData = new double[blockSize];
		double *pDstData = new double[nXSize * nYSize];

		// ��ȡԭʼͼ���
		if (!ogr.readRasterIO(pSrcData, 0, 0, nXSize, nYSize, GDT_Float64)) //ogr.getDataType_y()
		{
			addErrList(var + QStringLiteral(": ��ȡӰ��ֿ�����ʧ�ܣ���������"));
			continue;
		}

		// �����㷨
		for (long mi = 0; (mi + nBands) < blockSize; mi += nBands)
		{
			double valueR = pSrcData[mi + 0];
			double valueG = pSrcData[mi + 1];
			double valueB = pSrcData[mi + 2];

			// RGB�ۺ�ֵ����
			//double devsq = 0.0;
			//double stdevp = 0.0;

			//if (!(valueR == 0 && valueG == 0 && valueB ==0))
			//{
			//	double arvge = (valueR + valueG + valueB) / 3;
			//	devsq += pow(valueR - arvge, 2);
			//	devsq += pow(valueG - arvge, 2);
			//	devsq += pow(valueB - arvge, 2);
			//	stdevp = sqrt(devsq / 3);
			//}

			//pDstData[mi / nBands] = stdevp;

			// RGB ��Ȩƽ������
			double n = (valueR*0.0 + valueG*0.5 + valueB*0.5) / 1;
			pDstData[mi / nBands] = n;
		}
		//д�����ͼ��
		datasetBand->RasterIO(GF_Write, 0, 0, nXSize, nYSize, pDstData, nXSize, nYSize, GDT_Float64, 0, 0, 0);

		//�ͷ�������ڴ�
		RELEASE(pSrcData);
		RELEASE(pDstData);

		if (poDataset_target)
		{
			GDALClose((GDALDatasetH)poDataset_target);
			poDataset_target = nullptr;
		}
	}
}

int ipfModelerProcessChildConsistency::getFilesIndex(const QStringList & lists, const QString & th)
{
	int index = -1;
	QRegExp strExp("[N](" + th + ")[UG][.]");
	for (int i = 0; i < lists.size(); ++i)
	{
		if (lists.at(i).contains(strExp))
		{
			index = i;
			break;
		}
	}
	return index;
}

bool ipfModelerProcessChildConsistency::chackRasterVaule0(const QString & file)
{
	ipfOGR org(file);
	if (!org.isOpen())
		return false;

	float *pDataBuffer = 0;
	if (!org.readRasterIO(&pDataBuffer))
	{
		org.close();
		return false;
	}

	double nodata = org.getNodataValue(1);
	QList<int> list = org.getYXSize();
	int count = list.at(0) * list.at(1);

	org.close();

	for (int i = 0; i < count; ++i)
	{
		if (pDataBuffer[i] != 0 && pDataBuffer[i] != nodata)
		{
			delete[] pDataBuffer; pDataBuffer = 0;
			return false;
		}
	}

	delete[] pDataBuffer; pDataBuffer = 0;
	return true;
}

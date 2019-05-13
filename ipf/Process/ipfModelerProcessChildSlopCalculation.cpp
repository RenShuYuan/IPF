#include "ipfModelerProcessChildSlopCalculation.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"
#include "QgsVectorLayer.h"

#include <QProgressDialog>

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

	QString vectorName = "d:/fw.shp";
	//GDALDataset *poDS = (GDALDataset*)GDALOpenEx(vectorName.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	//if (poDS == NULL)
	//{
	//	addErrList(vectorName + QStringLiteral(": ��ʧ��"));
	//	return;
	//}
	//OGRLayer *poLayer = poDS->GetLayerByName("fw");
	//OGRFeature* poFeature = poLayer->GetFeature(0);
	//OGRGeometry* poGeometry = poFeature->GetGeometryRef();
	QgsVectorLayer *layer = new QgsVectorLayer(vectorName, "vector");
	if (!layer || !layer->isValid())
		return;
	QgsFeature f = layer->getFeature(0);
	QgsGeometry geo = f.geometry();

	foreach(QString var, filesIn())
	{
		ipfOGR org(var, true);
		if (!org.isOpen())
			continue;

		double nodata = org.getNodataValue(1);
		double R = org.getPixelSize();
		double midR = R / 2;
		QList<int> list = org.getYXSize();
		int nYSize = list.at(0);
		int nXSize = list.at(1);
		long nSize = nYSize * nXSize;
		QgsRectangle rang = org.getXY();
		double lx = rang.xMinimum();
		double ly = rang.yMaximum();

		////������
		int prCount = 0;
		ipfProgress proDialog;
		proDialog.setRangeTotal(0, filesIn().size());
		proDialog.setRangeChild(0, nSize);
		proDialog.show();

		// �ֿ����
		int nBlockSize = 64;
		long blockSize = nBlockSize * nBlockSize;
		double *pData = new double[blockSize];


		for (int i = 0; i < nYSize; i += nBlockSize)
		{
			for (int j = 0; j < nXSize; j += nBlockSize)
			{
				// ����ֿ�ʵ�ʴ�С
				int nXBK = nBlockSize;
				int nYBK = nBlockSize;

				//�������������ұߵĿ鲻����ʣ�¶��ٶ�ȡ����
				if (i + nBlockSize > nYSize)
					nYBK = nYSize - i;
				if (j + nBlockSize > nXSize)
					nXBK = nXSize - j;
				long size = nYBK * nXBK;

				// ��ȡԭʼͼ���
				
#pragma omp critical
				{
					if (!org.readRasterIO(pData, j, i, nXBK, nYBK, GDT_Float64))
					{
						addErrList(QStringLiteral(": ��ȡӰ��ֿ�����ʧ�ܣ���������"));
						//continue;
					}
				}

				//if (prCount < nSize)
				//{
				//	proDialog.setValue(prCount += size);
				//	QApplication::processEvents();
				//}
//#pragma omp critical
//				{
//					if (prCount < nSize)
//						proDialog.setValue(prCount += size);
//				}
				// �����㷨
				bool isbl = false;
#pragma omp parallel for
				for (long mi = 0; mi < nYBK; ++mi)
				{
					for (long mj = 0; mj < nXBK; ++mj)
					{
#pragma omp critical
						{
							if (prCount < nSize)
								proDialog.setValue(++prCount);
						}
						//OGRPoint point(lx + R * (j + mj) + midR, ly - R * (i + mi) - midR);
						QgsPointXY pointxy(lx + R * (j + mj) + midR, ly - R * (i + mi) - midR);
						//if (poGeometry->Contains(&point))
						if (geo.contains(&pointxy))
						{
							int index = mi * nXBK + mj;
							if (pData[index] != nodata) 
							{
								pData[index] = -8888;
								isbl = true;
							}
						}
					}
				}

				//д�����ͼ��
				if (isbl)
					org.getRasterBand(1)->RasterIO(GF_Write, j, i, nXBK, nYBK, pData, nXBK, nYBK, GDT_Float64, 0, 0, 0);
				
			}
		}
		RELEASE_ARRAY(pData);
	}
	RELEASE(layer);
}

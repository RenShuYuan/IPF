#include "ipfModelerProcessChildVegeataionExtraction.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerVegeataionExtractionDialog.h"
#include "../ipfOgr.h"

#include "qgsvectorlayer.h"

#include <QProgressDialog>

ipfModelerProcessChildVegeataionExtraction::ipfModelerProcessChildVegeataionExtraction(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerVegeataionExtractionDialog();

	fieldName = "EXIN";
}


ipfModelerProcessChildVegeataionExtraction::~ipfModelerProcessChildVegeataionExtraction()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildVegeataionExtraction::checkParameter()
{
	if (!QDir(fileName).exists())
	{
		addErrList(QStringLiteral("��Ч������ļ��С�"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildVegeataionExtraction::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		fileName = map["fileName"];
		index = map["index"].toDouble();
		stlip_index = map["stlip_index"].toDouble();
		minimumArea = map["minimumArea"].toInt();
		minimumRingsArea = map["minimumRingsArea"].toInt();
		buffer = map["buffer"].toInt();
	}
}

QMap<QString, QString> ipfModelerProcessChildVegeataionExtraction::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["index"] = QString::number(index);
	map["stlip_index"] = QString::number(stlip_index);
	map["minimumArea"] = QString::number(minimumArea);
	map["minimumRingsArea"] = QString::number(minimumRingsArea);
	map["buffer"] = QString::number(buffer);

	return map;
}

void ipfModelerProcessChildVegeataionExtraction::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	fileName = map["fileName"];
	index = map["index"].toDouble();
	stlip_index = map["stlip_index"].toDouble();
	minimumArea = map["minimumArea"].toInt();
	minimumRingsArea = map["minimumRingsArea"].toInt();
	buffer = map["buffer"].toInt();
}

double ipfModelerProcessChildVegeataionExtraction::vegeataionIndex(const double R, const double NIR)
{
	double index = (NIR - R) / (NIR + R);
	return index;
}

double ipfModelerProcessChildVegeataionExtraction::ylviIndex(const double B, const double G)
{
	double index = (B - G) / (B + G);
	return index;
}

void ipfModelerProcessChildVegeataionExtraction::run()
{
	clearOutFiles();
	clearErrList();

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, filesIn().size());

	foreach(QString var, filesIn())
	{
		QFileInfo info(var);
		QString baseName = info.baseName();

		// ��ȡ����NDVI����Ҫ���դ������ ----->
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡդ������ʧ�ܣ��޷�������"));
			continue;
		}

		int nBands = ogr.getBandSize();
		if (nBands != 4)
		{
			addErrList(var + QStringLiteral(": ��Ҫ4���ε�Ӱ�����ݣ�R��G��B��NIR�����޷�������"));
			continue;
		}

		int nXSize = ogr.getYXSize().at(1);
		int nYSize = ogr.getYXSize().at(0);
		double nodata1 = ogr.getNodataValue(1);
		double nodata2 = ogr.getNodataValue(2);
		double nodata3 = ogr.getNodataValue(3);
		double nodata4 = ogr.getNodataValue(4);
		QString prj = ogr.getProjection();

		// ��������ļ�
		QString rasterFile = ipfFlowManage::instance()->getTempFormatFile(var, ".img");
		QString vectorFile = fileName + "\\" + baseName + ".shp";
		GDALDataset* poDataset_target = ogr.createNewRaster(rasterFile, 1, GDT_Float32);
		if (!poDataset_target)
		{
			addErrList(rasterFile + QStringLiteral(": ������ʱʧ�ܣ��޷�������"));
			continue;
		}
		GDALRasterBand* datasetBand = poDataset_target->GetRasterBand(1);
		datasetBand->SetNoDataValue(-9999);

		// �ֿ����
		int nBlockSize = 1024;
		long blockSize = nBlockSize * nBlockSize * nBands;
		double *pSrcData = new double[blockSize];
		double *pDstData = new double[nBlockSize * nBlockSize];

		// ���������ֵ
		int proX = 0;
		int proY = 0;
		if (nXSize % nBlockSize == 0)
			proX = nXSize / nBlockSize;
		else
			proX = nXSize / nBlockSize + 1;
		if (nYSize % nBlockSize == 0)
			proY = nYSize / nBlockSize;
		else
			proY = nYSize / nBlockSize + 1;
		proDialog.setRangeChild(0, proX * proY);
		proDialog.show();

		//ѭ���ֿ鲢���д���
		int proCount = 0;
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

				long size = nYBK * nXBK * nBands;

				// ��ȡԭʼͼ���
				if (!ogr.readRasterIO(pSrcData, j, i, nXBK, nYBK, GDT_Float64)) //ogr.getDataType_y()
				{
					addErrList(baseName + QStringLiteral(": ��ȡӰ��ֿ�����ʧ�ܣ���������"));
					proDialog.setValue(++proCount);
					QApplication::processEvents();
					continue;
				}

				// �����㷨
				for (long mi = 0; (mi + nBands) < size; mi += nBands)
				{
					double B = pSrcData[mi];
					double G = pSrcData[mi+1];
					double R = pSrcData[mi+2];
					double NIR = pSrcData[mi+3];

					// �ų���Чֵ
					if (B == 0 || B == nodata1 ||
						G == 0 || G == nodata2 ||
						R == 0 || R == nodata3 ||
						NIR == 0 || NIR == nodata4)
					{
						pDstData[mi / nBands] = -9999;
						continue;
					}

					// ����ֲ��ָ��
					double NDVI = vegeataionIndex(R, NIR);
					if (abs(NDVI) > index)
					{
						if (stlip_index != 0)
						{
							//double ylVI = ylviIndex(B, G);
							double ylVI = ylviIndex(B, R);
							if (abs(ylVI) > stlip_index)
								pDstData[mi / nBands] = NDVI;
							else
								pDstData[mi / nBands] = -9999; // -9999
						}
						else
							pDstData[mi / nBands] = NDVI;
					}
					else
						pDstData[mi / nBands] = -9999; // -9999
				}

				//д�����ͼ��
				datasetBand->RasterIO(GF_Write, j, i, nXBK, nYBK, pDstData, nXBK, nYBK, GDT_Float64, 0, 0, 0);

				proDialog.setValue(++proCount);
				QApplication::processEvents();
				if (proDialog.wasCanceled())
				{
					//�ͷ�������ڴ�
					RELEASE(pSrcData);
					RELEASE(pDstData);

					// ɾ����ʱդ������
					QFile::remove(rasterFile);
					return;
				}
			}
		}
		
		//�ͷ�������ڴ�
		RELEASE(pSrcData);
		RELEASE(pDstData);

		if (poDataset_target)
		{
			GDALClose((GDALDatasetH)poDataset_target);
			poDataset_target = nullptr;
		}
		// ��ȡ����NDVI����Ҫ���դ������ -----<

		// �ָ�դ������դ��תʸ����Ч�� ----->
		QStringList clipRasers;
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

				QList<int> srcList;
				srcList << j << i << nXBK << nYBK;

				ipfGdalProgressTools gdal;
				QString target = ipfFlowManage::instance()->getTempVrtFile(var);
				QString err = gdal.proToClip_Translate_src(rasterFile, target, srcList);
				if (!err.isEmpty())
				{
					addErrList(baseName + QStringLiteral(": ���ݷֿ�ʧ�ܣ���������"));

					// ɾ����ʱդ������
					QFile::remove(rasterFile);

					continue;
				}
				else
					clipRasers << target;
			}
		}
		// �ָ�դ������դ��תʸ����Ч�� -----<

		// ����ʸ���ļ� ------>
		// ����shp����
		const char *pszDriverName = "ESRI Shapefile";
		GDALDriver *poDriver;
		poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
		if (poDriver == NULL)
		{
			addErrList(vectorFile + QStringLiteral(": ����ʸ������ʧ�ܡ�"));

			// ɾ����ʱդ������
			QFile::remove(rasterFile);

			continue;
		}

		// ����ʸ���ļ�
		GDALDataset *poDS;
		poDS = poDriver->Create(vectorFile.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
		if (poDS == NULL)
		{
			addErrList(vectorFile + QStringLiteral(": ����ʸ���ļ�ʧ�ܡ�"));

			// ɾ����ʱդ������
			QFile::remove(rasterFile);

			continue;
		}

		// ����ʸ��ͼ��
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer(baseName.toStdString().c_str(), NULL, wkbPolygon, NULL);
		if (poLayer == NULL)
		{
			addErrList(vectorFile + QStringLiteral(": ����ͼ��ʧ�ܡ�"));

			// ɾ����ʱդ������
			QFile::remove(rasterFile);

			continue;
		}

		// ����ͶӰ
		poDS->SetProjection(prj.toStdString().c_str());
		GDALClose(poDS);
		// ����ʸ���ļ� ------<

		// դ��תʸ�� ----->
		ipfGdalProgressTools gdal_v;
		gdal_v.setProgressTitle(QStringLiteral("��ȡʸ����Χ"));
		gdal_v.setProgressSize(clipRasers.size());
		gdal_v.showProgressDialog();
		for (int i = 0; i < clipRasers.size(); ++i)
		{
			QString clipRaster = clipRasers.at(i);
			QString err = gdal_v.rasterToVector(clipRaster, vectorFile, 0);
			if (!err.isEmpty())
			{
				addErrList(QStringLiteral("ֲ����ȡ: ") + err);

				// ɾ����ʱդ������
				QFile::remove(rasterFile);

				continue;
			}	
		}
		gdal_v.hideProgressDialog();
		// դ��תʸ�� -----<

		// ɾ����ʱդ������
		QFile::remove(rasterFile);

		// �ں�Ҫ�� ----->
		// �ں�Ҫ�� -----<

		// ����ն���������������Ҫ�� ----->
		QgsVectorLayer *layer = new QgsVectorLayer(vectorFile, "vector");
		if (!layer || !layer->isValid())
		{
			addErrList(vectorFile + QStringLiteral(": ��ȡʸ������ʧ��(����ն�)��"));
			continue;
		}

		QProgressDialog prDialog(QStringLiteral("Ҫ�ؼӹ�..."), QStringLiteral("ȡ��"), 0, layer->featureCount(), nullptr);
		prDialog.setWindowTitle(QStringLiteral("�������"));
		prDialog.setWindowModality(Qt::WindowModal);
		prDialog.show();
		int prCount = 0;

		QgsFeature f;
		QList<QgsFeatureId> idList;
		QgsFeatureIterator fList = layer->getFeatures();
		
		while (fList.nextFeature(f))
		{
			if (f.hasGeometry())
				idList << f.id();
		}

		QgsFeatureIds ids;
		QgsFeatureList features;
		int size = idList.size();
		
		// ���ʹ��OpenMP������
#pragma omp parallel for
		for (int i = 0; i < size; ++i)
		{
			QgsFeature f = layer->getFeature(idList.at(i));

			// ɾ������
			if (f.geometry().area() < minimumArea)
			{
#pragma omp critical
				{
					ids << f.id();
				}
			}
			else
			{
				// ɾ���ն�
				QgsFeature outFeature = f;
				QgsGeometry geometry = outFeature.geometry();
				outFeature.setGeometry(geometry.removeInteriorRings(minimumRingsArea));
#pragma omp critical
				{
					ids << f.id();
					features << outFeature;
				}
			}

#pragma omp critical
			{
				if (++prCount < size)
				{
					prDialog.setValue(prCount);
					QApplication::processEvents();
				}
			}
		}

		layer->startEditing();
		layer->deleteFeatures(ids);
		layer->addFeatures(features);
		layer->commitChanges();
		RELEASE(layer);
		// ����ն���������������Ҫ�� -----<
	}
}

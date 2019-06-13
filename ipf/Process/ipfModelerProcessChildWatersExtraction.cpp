#include "ipfModelerProcessChildWatersExtraction.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerWatersExtractionDialog.h"
#include "../ipfOgr.h"

#include "qgsvectorlayer.h"

#include <QProgressDialog>

ipfModelerProcessChildWatersExtraction::ipfModelerProcessChildWatersExtraction(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerWatersExtractionDialog();

	fieldName = "EXIN";
}

ipfModelerProcessChildWatersExtraction::~ipfModelerProcessChildWatersExtraction()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildWatersExtraction::checkParameter()
{
	QDir dir(fileName);
	if (!dir.exists())
	{
		addErrList(QStringLiteral("��Ч������ļ��С�"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildWatersExtraction::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		fileName = map["fileName"];
		index = map["index"].toDouble();
		minimumArea = map["minimumArea"].toInt();
		minimumRingsArea = map["minimumRingsArea"].toInt();
	}
}

QMap<QString, QString> ipfModelerProcessChildWatersExtraction::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["index"] = QString::number(index);
	map["minimumArea"] = QString::number(minimumArea);
	map["minimumRingsArea"] = QString::number(minimumRingsArea);

	return map;
}

void ipfModelerProcessChildWatersExtraction::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	fileName = map["fileName"];
	index = map["index"].toDouble();
	minimumArea = map["minimumArea"].toInt();
	minimumRingsArea = map["minimumRingsArea"].toInt();
}

double ipfModelerProcessChildWatersExtraction::watersIndex(const double G, const double NIR)
{
	// NDWI
	double index = (G - NIR) / (NIR + G);
	return index;
}

void ipfModelerProcessChildWatersExtraction::run()
{
	clearOutFiles();
	clearErrList();

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, filesIn().size());

	foreach(QString var, filesIn())
	{
		QString baseName = removeDelimiter(var);

		// ��ȡ��������Ҫ���դ������ ----->
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

		// �������դ��
		QString rasterFile = fileName + "\\" + baseName + "_v.img";
		QString vectorFile = fileName + "\\" + baseName + ".shp";
		GDALDataset* poDataset_target = ogr.createNewRaster(rasterFile, "-9999", 1, GDT_Float32);
		if (!poDataset_target)
		{
			addErrList(rasterFile + QStringLiteral(": �������դ������ʧ�ܣ��޷�������"));
			continue;
		}
		GDALRasterBand* datasetBand = poDataset_target->GetRasterBand(1);

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
		bool isbl = true;
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
					addErrList(var + QStringLiteral(": ��ȡӰ��ֿ�����ʧ�ܣ���������"));
					proDialog.setValue(++proCount);
					continue;
				}

				// �����㷨
				for (long mi = 0; (mi + nBands) < size; mi += nBands)
				{
					double B = pSrcData[mi];
					double G = pSrcData[mi + 1];
					double R = pSrcData[mi + 2];
					double NIR = pSrcData[mi + 3];

					// �ų���Чֵ
					if (B == 0 || B == nodata1 ||
						G == 0 || G == nodata2 ||
						R == 0 || R == nodata3 ||
						NIR == 0 || NIR == nodata4)
					{
						pDstData[mi / nBands] = -9999;
						continue;
					}

					// ����ˮ��ָ��
					double NDWI = watersIndex( G, NIR);
					if (NDWI > index)
					{
						pDstData[mi / nBands] = NDWI;
					}
					else
					{
						pDstData[mi / nBands] = -9999;
					}
				}

				//д�����ͼ��
				datasetBand->RasterIO(GF_Write, j, i, nXBK, nYBK, pDstData, nXBK, nYBK, GDT_Float64, 0, 0, 0);

				proDialog.setValue(++proCount);
				if (proDialog.wasCanceled())
					return;
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
		// ��ȡ��������Ҫ���դ������ -----<

		// �ָ�դ������դ��תʸ����Ч�� ----->
		QStringList clipRasers;
		ipfOGR ogr_clip(rasterFile);
		if (!ogr_clip.isOpen())
		{
			addErrList(baseName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -2��"));
			QFile::remove(rasterFile);
			continue;
		}
		if (!ogr_clip.splitRaster(1024, clipRasers))
		{
			addErrList(baseName + QStringLiteral(": ת��ʸ��ʧ�ܣ���������"));
			QFile::remove(rasterFile);
			continue;
		}

		// ����ʸ��ͼ�� ----->
		if (!ipfOGR::createrVectorlayer(vectorFile, QgsWkbTypes::Polygon, QgsFields(), prj))
		{
			addErrList(vectorFile + QStringLiteral(": ����ʸ���ļ�ʧ�ܣ���������"));
			QFile::remove(rasterFile);
			continue;
		}

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
				addErrList(QStringLiteral("ˮ����ȡ: ") + err);
				QFile::remove(rasterFile);
				continue;
			}
		}
		gdal_v.hideProgressDialog();
		// դ��תʸ�� -----<

		// ɾ����ʱդ������
		QFile::remove(rasterFile);

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

		layer->startEditing();
		int size = idList.size();
		for (int i = 0; i < size; ++i)
		{
			QgsFeature f = layer->getFeature(idList.at(i));

			// ɾ������
			if (f.geometry().area() < minimumArea)
			{
				layer->deleteFeature(f.id());
				prDialog.setValue(++prCount);
				continue;
			}

			// ɾ���ն�
			QgsGeometry geo = f.geometry().removeInteriorRings(minimumRingsArea);
			QgsFeature outFeature;
			outFeature.setGeometry(geo);
			QgsAttributes inattrs = f.attributes();
			outFeature.setAttributes(inattrs);
			layer->addFeature(outFeature);
			layer->deleteFeature(f.id());

			prDialog.setValue(++prCount);
			if (prDialog.wasCanceled())
			{
				layer->commitChanges();
				break;
			}
		}
		layer->commitChanges();
		RELEASE(layer);
		// ����ն���������������Ҫ�� -----<
	}
}


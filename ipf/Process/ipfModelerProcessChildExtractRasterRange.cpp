#include "ipfModelerProcessChildExtractRasterRange.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerExtractRasterRangeDialog.h"
#include "../ipfOgr.h"

#include "qgsvectorlayer.h"
#include <QProgressDialog>

ipfModelerProcessChildExtractRasterRange::ipfModelerProcessChildExtractRasterRange(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerExtractRasterRangeDialog();
}

ipfModelerProcessChildExtractRasterRange::~ipfModelerProcessChildExtractRasterRange()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildExtractRasterRange::checkParameter()
{
	if (!QDir(fileName).exists())
	{
		addErrList(QStringLiteral("��Ч������ļ��С�"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildExtractRasterRange::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		fileName = map["fileName"];
		background = map["background"].toDouble();
		minimumRingsArea = map["minimumRingsArea"].toInt();
	}
}

QMap<QString, QString> ipfModelerProcessChildExtractRasterRange::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["background"] = QString::number(background);
	map["minimumRingsArea"] = QString::number(minimumRingsArea);

	return map;
}

void ipfModelerProcessChildExtractRasterRange::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	fileName = map["fileName"];
	background = map["background"].toDouble();
	minimumRingsArea = map["minimumRingsArea"].toInt();
}

void ipfModelerProcessChildExtractRasterRange::run()
{
	clearOutFiles();
	clearErrList();

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, filesIn().size());

	foreach(QString var, filesIn())
	{
		QFileInfo info(var);
		QString baseName = info.baseName();

		// ��ȡ�Ǳ���ֵ������դ������ ----->
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡդ������ʧ�ܣ��޷�������"));
			continue;
		}

		int nBands = ogr.getBandSize();
		if (nBands == 0)
		{
			addErrList(var + QStringLiteral(": դ������ȱ����Ч���Σ��޷�������"));
			continue;
		}

		int nXSize = ogr.getYXSize().at(1);
		int nYSize = ogr.getYXSize().at(0);
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

		// �ֿ����
		int nBlockSize = 512;
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
					// �ų���Чֵ
					bool bl = false;
					for (int i = 0; i < nBands; ++i)
					{
						double value = pSrcData[mi + i];
						if (background == value)
						{
							bl = true;
							break;
						}
					}
					if (bl)
						pDstData[mi / nBands] = -9999; // Nodata
					else
						pDstData[mi / nBands] = 100;
				}

				//д�����ͼ��
				//datasetBand->RasterIO(GF_Write, j, i, nXBK, nYBK, pDstData, nXBK, nYBK, GDT_Float64, 0, 0, 0);

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
		 //��ȡ�Ǳ���ֵ������դ������ -----<

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
					addErrList(rasterFile + QStringLiteral(": դ��ֿ�ʧ�ܣ���������"));
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
			addErrList(vectorFile + QStringLiteral(": ��������ʧ�ܡ�"));
			continue;
		}

		// ����ʸ���ļ�
		GDALDataset *poDS;
		poDS = poDriver->Create(vectorFile.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
		if (poDS == NULL)
		{
			addErrList(vectorFile + QStringLiteral(": ����ʸ���ļ�ʧ�ܡ�"));
			continue;
		}

		// ����ʸ��ͼ��
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer(baseName.toStdString().c_str(), NULL, wkbPolygon, NULL);
		if (poLayer == NULL)
		{
			addErrList(vectorFile + QStringLiteral(": ����ͼ��ʧ�ܡ�"));
			continue;
		}

		// �����ֶ�
		if (OGRERR_NONE != poLayer->CreateField(new OGRFieldDefn(fieldName.toStdString().c_str(), OFTInteger)))
		{
			addErrList(vectorFile + QStringLiteral(": �����ֶ�ʧ�ܡ�"));
			continue;
		}
		int dst_field = poLayer->GetLayerDefn()->GetFieldIndex(fieldName.toStdString().c_str());
		GDALClose(poDS);
		// ����ʸ���ļ� ------<

		// դ��תʸ�� ----->
		ipfGdalProgressTools gdal_v;
		gdal_v.setProgressTitle(QStringLiteral("��ȡʸ����Χ"));
		gdal_v.setProgressSize(1);
		gdal_v.showProgressDialog();
		QString err = gdal_v.rasterToVector(rasterFile, vectorFile, dst_field);
		if (!err.isEmpty())
		{
			addErrList(QStringLiteral("ֲ����ȡ: ") + err);
			continue;
		}
		gdal_v.hideProgressDialog();
		// դ��תʸ�� -----<

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
			if (f.geometry().area() < minimumRingsArea)
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

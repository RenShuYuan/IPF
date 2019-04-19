#include "ipfModelerProcessChildWaterFlatten.h"
#include "../../ui/ipfModelerWaterFlattenDialog.h"
#include "../../ui/ipfProgress.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"

ipfModelerProcessChildWaterFlatten::ipfModelerProcessChildWaterFlatten(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerWaterFlattenDialog();
}

ipfModelerProcessChildWaterFlatten::~ipfModelerProcessChildWaterFlatten()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildWaterFlatten::checkParameter()
{
	bool isbl = true;

	QFileInfo info(vectorName);
	if (!info.exists())
	{
		isbl = false;
		addErrList(QStringLiteral("ʸ���ļ�·����Ч��"));
	}

	QDir dir(outPath);
	if (!dir.exists())
	{
		isbl = false;
		addErrList(QStringLiteral("��Ч������ļ��С�"));
	}

	return isbl;
}

void ipfModelerProcessChildWaterFlatten::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		vectorName = map["vectorName"];
		outPath = map["outPath"];
	}
}

QMap<QString, QString> ipfModelerProcessChildWaterFlatten::getParameter()
{
	QMap<QString, QString> map;
	map["vectorName"] = vectorName;
	map["outPath"] = outPath;

	return map;
}

void ipfModelerProcessChildWaterFlatten::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	vectorName = map["vectorName"];
	outPath = map["outPath"];
}

bool ipfModelerProcessChildWaterFlatten::splitShp(const QString & shpName, QStringList & shps)
{
	GDALDataset *poDS;
	poDS = (GDALDataset*)GDALOpenEx(vectorName.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (poDS == NULL)
	{
		addErrList(vectorName + QStringLiteral(": ��ȡʸ���ļ�ʧ�ܣ��޷�������"));
		return false;
	}
	OGRLayer *poLayer = poDS->GetLayer(0);
	if (!poLayer)
	{
		addErrList(vectorName + QStringLiteral(": ʧ�ܣ��޷���ȡʸ��ͼ�㡣"));
		return false;
	}

	OGRFeature * poFeature;
	while ((poFeature = poLayer->GetNextFeature()) != NULL)
	{
		// ����shp����
		const char *pszDriverName = "ESRI Shapefile";
		GDALDriver *poDriver;
		poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
		if (poDriver == NULL)
		{
			addErrList(vectorName + QStringLiteral(": ��������ʧ�ܡ�"));
			return false;
		}

		// ����ʸ���ļ�
		GDALDataset *poDS;
		QString new_shp = ipfFlowManage::instance()->getTempFormatFile(vectorName, ".shp");
		poDS = poDriver->Create(new_shp.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
		if (poDS == NULL)
		{
			addErrList(vectorName + QStringLiteral(": ������ʱʸ���ļ�ʧ�ܡ�"));
			return false;
		}

		// ����ʸ��ͼ��
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer("out", NULL, wkbPolygon, NULL);
		if (poLayer == NULL)
		{
			addErrList(vectorName + QStringLiteral(": ����ͼ��ʧ�ܡ�"));
			return false;
		}

		// ���Ҫ��
		poLayer->CreateFeature(poFeature);
		OGRFeature::DestroyFeature(poFeature);
		GDALClose(poDS);

		shps << new_shp;
	}
	GDALClose(poDS);
	return true;
}

QList<double> ipfModelerProcessChildWaterFlatten::shpEnvelopeOne(const QString & file, const double R)
{
	QList<double> list;

	// ����դ��������Χ
	GDALDataset *poDS = NULL;
	poDS = (GDALDataset*)GDALOpenEx(file.toStdString().c_str(),
		GDAL_OF_READONLY | GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (!poDS)
	{
		addErrList(file + QStringLiteral(": ʧ�ܣ��޷���ʸ�����ݡ�"));
		return list;
	}

	OGRLayer *poLayer = poDS->GetLayer(0);
	if (!poLayer)
	{
		addErrList(file + QStringLiteral(": ʧ�ܣ��޷���ȡʸ�����ݡ�"));
		return list;
	}

	OGREnvelope oExt;
	if (poLayer->GetExtent(0, &oExt, TRUE) != OGRERR_NONE)
	{
		addErrList(file + QStringLiteral(": ʧ�ܣ��޷���ȡʸ����ķ�Χ��"));
		return list;
	}
	GDALClose(poDS);

	double ulx = 0.0, uly = 0.0, lrx = 0.0, lry = 0.0;
	if (oExt.MinX < 360)
		ulx = ((int)(oExt.MinX / R)) * R + R;
	else
		ulx = ((int)(oExt.MinX / R)) * R;

	uly = (int)(oExt.MaxY / R) * R;
	if (oExt.MaxY > uly) uly += R;

	lrx = (int)(oExt.MaxX / R) * R;
	if (oExt.MaxX > lrx) lrx += R;

	if (oExt.MinY < 360)
		lry = (int)(oExt.MinY / R) * R - R;
	else
		lry = (int)(oExt.MinY / R) * R;

	list << ulx << uly << lrx << lry;
	return list;
}

void ipfModelerProcessChildWaterFlatten::run()
{
	clearOutFiles();
	clearErrList();

	// �ָ�shp
	QStringList shps;
	if (!splitShp(vectorName, shps)) return;

	ipfGdalProgressTools gdal;
	QStringList outList;

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, filesIn().size());
	proDialog.setRangeChild(0, shps.size());
	proDialog.show();

	foreach(QString var, filesIn())
	{
		QFileInfo info(var);
		QString rasterBaseName = info.baseName();

		// ��ȡӰ��ֱ���
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(rasterBaseName + QStringLiteral(": ��ȡӰ��ֱ���ʧ�ܣ��޷�������"));
			continue;
		}
		double R = ogr.getPixelSize();
		ogr.close();

		// ����ÿ��ʸ����
		int proCount = 0;
		
		foreach(QString shp, shps)
		{
			proDialog.setValue(++proCount);
			QApplication::processEvents();
			if (proDialog.wasCanceled())
				return;

			// ���Ҫ����ʸ���ķ�Χ
			QList<double> list = shpEnvelopeOne(shp, R);
			if (list.size() != 4) continue;

			// ���ʸ����Χ�Ƿ���դ��Χ��
			int iRow = 0, iCol = 0;
			QString err = gdal.locationPixelInfo(var, list.at(0), list.at(1), iRow, iCol);
			if (!err.isEmpty())
			{
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(list.at(0), 'f', 3) + ","
					+ QString::number(list.at(1), 'f', 3) + ")"
					+ QStringLiteral(": ʸ����Χû��դ��Χ�ڡ�");
				continue;
			}
			err = gdal.locationPixelInfo(var, list.at(2), list.at(3), iRow, iCol);
			if (!err.isEmpty())
			{
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(list.at(0), 'f', 3) + ","
					+ QString::number(list.at(1), 'f', 3) + ")"
					+ QStringLiteral(": ʸ����Χû��դ��Χ�ڡ�");
				continue;
			}

			// ʹ��ʸ���ļ�����դ��
			QString target = ipfFlowManage::instance()->getTempVrtFile(var);
			err = gdal.AOIClip(var, target, shp, false);
			if (!err.isEmpty())
			{
				addErrList(rasterBaseName + ": " + err);
				continue;
			}

			// ������դ����С��Χ
			QString new_target = ipfFlowManage::instance()->getTempVrtFile(target);
			err = gdal.proToClip_Translate(target, new_target, list);
			if (!err.isEmpty())
			{
				addErrList(rasterBaseName + ": " + err);
				continue;
			}

			// �����Ԫֵ�Ƿ�һ��
			ipfOGR org(new_target);
			if (!org.isOpen())
			{
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(list.at(0), 'f', 3) + ","
					+ QString::number(list.at(1), 'f', 3) + ")"
					+ QStringLiteral(": ���ˮƽʱ�����쳣����-1��");
				continue;
			}

			float *pDataBuffer = 0;
			if (!org.readRasterIO(&pDataBuffer))
			{
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(list.at(0), 'f', 3) + ","
					+ QString::number(list.at(1), 'f', 3) + ")"
					+ QStringLiteral(": ���ˮƽʱ�����쳣����-2��");
				org.close();
				continue;
			}

			double nodata = org.getNodataValue(1);
			QList<int> rect = org.getYXSize();
			org.close();
			int count = rect.at(0) * rect.at(1);

			bool isErr = true;
			bool isNodata = true;
			for (int i = 0; i < count - 1; ++i)
			{
				if (pDataBuffer[i] != nodata && pDataBuffer[i+1] != nodata)
				{
					isNodata = false;
					if (pDataBuffer[i] != pDataBuffer[i + 1])
					{
						isErr = false;

						// �������դ������
						QString target = outPath + "\\" + rasterBaseName + "@" + QString::number(proCount) + ".img";
						QString err = gdal.formatConvert(new_target, target, gdal.enumFormatToString("img"), "NONE", "NO", "none");
						if (!err.isEmpty())
							addErrList(rasterBaseName + ": " + err);
						break;
					}
				}
			}
			delete[] pDataBuffer; pDataBuffer = 0;
			
			if (isNodata)
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
				+ QString::number(list.at(0), 'f', 3) + ","
				+ QString::number(list.at(1), 'f', 3) + ")"
				+ QStringLiteral(": ʸ����Χ��NODATA�����С�");
			else
			{
				if (isErr)
					outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(list.at(0), 'f', 3) + ","
					+ QString::number(list.at(1), 'f', 3) + ")"
					+ QStringLiteral(": ��ֹˮ��߳�һ�¡�");
				else
					outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(list.at(0), 'f', 3) + ","
					+ QString::number(list.at(1), 'f', 3) + ")"
					+ QStringLiteral(": ��ֹˮ��̲߳�һ�¡�");
			}
		}
	}

	QString saveName = outPath + QStringLiteral("/ˮƽ���.txt");
	QFile file(saveName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		addErrList(saveName + QStringLiteral("���������ļ�ʧ�ܣ�����ֹ��"));
		return;
	}
	QTextStream out(&file);
	foreach(QString str, outList)
		out << str << endl;
	file.close();
}

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

void ipfModelerProcessChildWaterFlatten::run()
{
	clearOutFiles();
	clearErrList();

	// �ָ�shp
	QStringList shps;
	if (!ipfOGR::splitShp(vectorName, shps))
	{
		addErrList(vectorName + QStringLiteral(": �ָ�ʸ������ʧ�ܣ��޷�������"));
		return;
	}

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

		// ����ÿ��ʸ����
		int proCount = 0;
		foreach(QString shp, shps)
		{
			proDialog.setValue(++proCount);
			if (proDialog.wasCanceled())
				return;

			// ������з�Χ
			ipfOGR ogr(var);
			if (!ogr.isOpen())
			{
				addErrList(var + QStringLiteral(": ��ȡդ������ʧ�ܣ���������"));
				return;
			}

			QgsRectangle rect;
			if (!ogr.shpEnvelope(vectorName, rect))
			{
				addErrList(vectorName + QStringLiteral(": ����ʸ����Χʧ�ܣ���������"));
				return;
			}

			// ���ʸ����Χ�Ƿ���դ��Χ��
			QgsRectangle rectRaster = ogr.getXY();
			if (!rectRaster.contains(rect))
			{
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": ʸ����Χû��դ��Χ�ڡ�");
				continue;
			}
			ogr.close();

			// ʹ��ʸ���ļ�����դ��
			QString target = ipfFlowManage::instance()->getTempVrtFile(var);
			QString err = gdal.AOIClip(var, target, shp);
			if (!err.isEmpty())
			{
				addErrList(rasterBaseName + ": " + err);
				continue;
			}

			// ������դ����С��Χ
			QString new_target = ipfFlowManage::instance()->getTempVrtFile(target);
			err = gdal.proToClip_Translate(target, new_target, rect);
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
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": ���ˮƽʱ�����쳣����-1��");
				continue;
			}

			float *pDataBuffer = 0;
			if (!org.readRasterIO(&pDataBuffer))
			{
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": ���ˮƽʱ�����쳣����-2��");
				org.close();
				continue;
			}

			double nodata = org.getNodataValue(1);
			QList<int> rectXY = org.getYXSize();
			org.close();
			int count = rectXY.at(0) * rectXY.at(1);

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
						QString target = outPath + "\\" + rasterBaseName + "@" + QString::number(proCount) + ".tif";
						QString err = gdal.formatConvert(new_target, target, gdal.enumFormatToString("tif"), "NONE", "NO", "none");
						if (!err.isEmpty())
							addErrList(rasterBaseName + ": " + err);
						break;
					}
				}
			}
			RELEASE_ARRAY(pDataBuffer);
			
			if (isNodata)
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
				+ QString::number(rect.xMinimum(), 'f', 3) + ","
				+ QString::number(rect.yMaximum(), 'f', 3) + ")"
				+ QStringLiteral(": ʸ����Χ��NODATA�����С�");
			else
			{
				if (isErr)
					outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": ��ֹˮ��߳�һ�¡�");
				else
					outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": ��ֹˮ��̲߳�һ�¡�");
			}
		}
	}

	QString saveName = outPath + QStringLiteral("/ˮƽ���.txt");
	printErrToFile(saveName, outList);
}

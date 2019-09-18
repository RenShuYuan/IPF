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
		QString rasterBaseName = QFileInfo(var).baseName();

		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡդ������ʧ�ܣ���������"));
			return;
		}

		int proCount = 0;
		foreach(QString shp, shps)
		{
			proDialog.setValue(++proCount);
			if (proDialog.wasCanceled())
				return;

			// ������з�Χ
			QgsRectangle rect;
			CPLErr gErr = ogr.shpEnvelope(shp, rect);
			if (gErr == CE_Failure)
			{
				addErrList(vectorName + QStringLiteral(": ����ʸ����Χʧ�ܣ���������"));
				return;
			}
			else if (gErr == CE_Warning)
				continue;

			// ʹ��ʸ���ļ�����դ��
			QString target = ipfApplication::instance()->getTempVrtFile(var);
			QString err = gdal.AOIClip(var, target, shp);
			if (!err.isEmpty())
			{
				addErrList(rasterBaseName + ": " + err);
				continue;
			}

			// ������դ����С��Χ
			QList<int> srcList;
			int iRowLu = 0, iColLu = 0, iRowRd = 0, iColRd = 0;
			if (!ogr.Projection2ImageRowCol(rect.xMinimum(), rect.yMaximum(), iColLu, iRowLu)
				|| !ogr.Projection2ImageRowCol(rect.xMaximum(), rect.yMinimum(), iColRd, iRowRd))
			{
				addErrList(var + QStringLiteral(": ƥ����Ԫλ��ʧ�ܻ򳬳���Χ���޷�������"));
				continue;
			}
			srcList << iColLu << iRowLu << iColRd - iColLu + 1 << iRowRd - iRowLu + 1;
			QString new_target = ipfApplication::instance()->getTempVrtFile(var);
			err = gdal.proToClip_Translate_src(target, new_target, srcList);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
				continue;
			}

			// �����Ԫֵ�Ƿ�һ��
			ipfOGR org_err(new_target);
			if (!org_err.isOpen())
			{
				outList << rasterBaseName + "->" + QString::number(proCount)
					+ QStringLiteral(": ���ˮƽʱ�����쳣����-1��");
				continue;
			}

			CPLErr cErr = org_err.ComputeMinMax(IPF_EQUAL);
			if (cErr == CE_None)
				outList << rasterBaseName + "->" + QString::number(proCount)
				+ QStringLiteral(": ��ֹˮ��߳�һ�¡�");
			else if (cErr == CE_Warning)
			{
				outList << rasterBaseName + "->" + QString::number(proCount)
					+ QStringLiteral(": ��ֹˮ��̲߳�һ�¡�");

				QString target = outPath + "\\" + rasterBaseName + "@" + QString::number(proCount) + ".tif";
				QString err = gdal.formatConvert(new_target, target, gdal.enumFormatToString("tif"), "NONE", "NO", "none");
				if (!err.isEmpty())
					addErrList(rasterBaseName + ": " + err);
			}
			else
				outList << rasterBaseName + "->" + QString::number(proCount)
				+ QStringLiteral(": ʸ����Χ��NODATA�����С�");
		}
	}

	QString saveName = outPath + QStringLiteral("/ˮƽ���.txt");
	printErrToFile(saveName, outList);
}

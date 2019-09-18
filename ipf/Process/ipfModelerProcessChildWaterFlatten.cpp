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
		addErrList(QStringLiteral("矢量文件路径无效。"));
	}

	QDir dir(outPath);
	if (!dir.exists())
	{
		isbl = false;
		addErrList(QStringLiteral("无效的输出文件夹。"));
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

	// 分割shp
	QStringList shps;
	if (!ipfOGR::splitShp(vectorName, shps))
	{
		addErrList(vectorName + QStringLiteral(": 分割矢量数据失败，无法继续。"));
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
			addErrList(var + QStringLiteral(": 读取栅格数据失败，已跳过。"));
			return;
		}

		int proCount = 0;
		foreach(QString shp, shps)
		{
			proDialog.setValue(++proCount);
			if (proDialog.wasCanceled())
				return;

			// 计算裁切范围
			QgsRectangle rect;
			CPLErr gErr = ogr.shpEnvelope(shp, rect);
			if (gErr == CE_Failure)
			{
				addErrList(vectorName + QStringLiteral(": 计算矢量范围失败，已跳过。"));
				return;
			}
			else if (gErr == CE_Warning)
				continue;

			// 使用矢量文件裁切栅格
			QString target = ipfApplication::instance()->getTempVrtFile(var);
			QString err = gdal.AOIClip(var, target, shp);
			if (!err.isEmpty())
			{
				addErrList(rasterBaseName + ": " + err);
				continue;
			}

			// 裁切至栅格最小范围
			QList<int> srcList;
			int iRowLu = 0, iColLu = 0, iRowRd = 0, iColRd = 0;
			if (!ogr.Projection2ImageRowCol(rect.xMinimum(), rect.yMaximum(), iColLu, iRowLu)
				|| !ogr.Projection2ImageRowCol(rect.xMaximum(), rect.yMinimum(), iColRd, iRowRd))
			{
				addErrList(var + QStringLiteral(": 匹配像元位置失败或超出范围，无法继续。"));
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

			// 检查像元值是否一致
			ipfOGR org_err(new_target);
			if (!org_err.isOpen())
			{
				outList << rasterBaseName + "->" + QString::number(proCount)
					+ QStringLiteral(": 检查水平时出现异常错误-1。");
				continue;
			}

			CPLErr cErr = org_err.ComputeMinMax(IPF_EQUAL);
			if (cErr == CE_None)
				outList << rasterBaseName + "->" + QString::number(proCount)
				+ QStringLiteral(": 静止水体高程一致。");
			else if (cErr == CE_Warning)
			{
				outList << rasterBaseName + "->" + QString::number(proCount)
					+ QStringLiteral(": 静止水体高程不一致。");

				QString target = outPath + "\\" + rasterBaseName + "@" + QString::number(proCount) + ".tif";
				QString err = gdal.formatConvert(new_target, target, gdal.enumFormatToString("tif"), "NONE", "NO", "none");
				if (!err.isEmpty())
					addErrList(rasterBaseName + ": " + err);
			}
			else
				outList << rasterBaseName + "->" + QString::number(proCount)
				+ QStringLiteral(": 矢量范围在NODATA区域中。");
		}
	}

	QString saveName = outPath + QStringLiteral("/水平检查.txt");
	printErrToFile(saveName, outList);
}

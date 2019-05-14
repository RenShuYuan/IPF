#include "ipfModelerProcessChildRangeMoidfyValue.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerRangeMoidfyValueDialog.h"
#include "../ipfOgr.h"

ipfModelerProcessChildRangeMoidfyValue::ipfModelerProcessChildRangeMoidfyValue(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerRangeMoidfyValueDialog();
	if (modelerName == MODELER_RANGEMOIDFYVALUE)
		dialog->setValueEnable(false);
	else
		dialog->setValueEnable(true);
}

ipfModelerProcessChildRangeMoidfyValue::~ipfModelerProcessChildRangeMoidfyValue()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildRangeMoidfyValue::checkParameter()
{
	clearErrList();

	if (!QFileInfo(vectorName).exists())
	{
		addErrList(QStringLiteral("矢量文件路径无效。"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildRangeMoidfyValue::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		vectorName = map["vectorName"];
		value = map["value"].toDouble();
		if (map["isOutChange"] == "YES")
			isOutChange = true;
		else
			isOutChange = false;
	}
}

QMap<QString, QString> ipfModelerProcessChildRangeMoidfyValue::getParameter()
{
	QMap<QString, QString> map;

	map["vectorName"] = vectorName;
	map["value"] = QString::number(value);
	if (isOutChange)
		map["isOutChange"] = "YES";
	else
		map["isOutChange"] = "NO";

	return map;
}

void ipfModelerProcessChildRangeMoidfyValue::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	vectorName = map["vectorName"];
	value = map["value"].toDouble();
	if (map["isOutChange"] == "YES")
		isOutChange = true;
	else
		isOutChange = false;
}

void ipfModelerProcessChildRangeMoidfyValue::run()
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

	// 设置填充值
	if (name() == MODELER_SEAMOIDFYVALUE)
		value = -8888;

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();
	foreach(QString var, filesIn())
	{
		gdal.pulsValueTatal();
		QString format = var.right(3);

		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": 读取栅格数据失败，已跳过。"));
			return;
		}
		double nodata = ogr.getNodataValue(1);

		QStringList mosaicList;
		mosaicList << var;

		for (int i = 0; i < shps.size(); ++i)
		{
			QString shp = shps.at(i);

			// 计算裁切范围
			QgsRectangle rect;
			CPLErr gErr = ogr.shpEnvelope(shp, rect);
			if (gErr == CE_Failure)
			{
				addErrList(var + QStringLiteral(": 计算矢量范围失败，已跳过。"));
				return;
			}
			else if (gErr == CE_Warning)
				continue;

			// 裁切AOI VRT栅格
			QString target = ipfFlowManage::instance()->getTempVrtFile(var);
			QString err = gdal.AOIClip(var, target, vectorName);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
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
			srcList << iColLu << iRowLu << iColRd - iColLu << iRowRd - iRowLu;
			QString new_target = ipfFlowManage::instance()->getTempVrtFile(target);
			err = gdal.proToClip_Translate_src(target, new_target, srcList);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
				continue;
			}

			// 注册填充算法
			QString target_target = ipfFlowManage::instance()->getTempVrtFile(var);
			err = gdal.pixelFillValue(new_target, target_target, nodata, value);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
				continue;
			}

			// 转为实体栅格
			QString targetTo = ipfFlowManage::instance()->getTempFormatFile(var, "." + format);
			err = gdal.formatConvert(target_target, targetTo, gdal.enumFormatToString(format), "NONE", "NO", QString::number(nodata));
			if (!err.isEmpty())
			{
				addErrList(var + QStringLiteral(": 转为实体栅格失败，请检查C盘空间是否充足。"));
				continue;
			}
			mosaicList << targetTo;
		}

		// 镶嵌回大块
		if (mosaicList.size() > 1)
		{
			QString targetOut = ipfFlowManage::instance()->getTempVrtFile(var);
			QString err = gdal.mosaic_Buildvrt(mosaicList, targetOut);
			if (err.isEmpty())
				appendOutFile(targetOut);
			else
				addErrList(var + ": " + err);
		}
		else if (!isOutChange)
			appendOutFile(var);
	}
}

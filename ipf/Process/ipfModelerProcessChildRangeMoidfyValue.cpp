#include "ipfModelerProcessChildRangeMoidfyValue.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerRangeMoidfyValueDialog.h"
#include "../ipfOgr.h"

ipfModelerProcessChildRangeMoidfyValue::ipfModelerProcessChildRangeMoidfyValue(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	dialog = new ipfModelerRangeMoidfyValueDialog();
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
	}
}

QMap<QString, QString> ipfModelerProcessChildRangeMoidfyValue::getParameter()
{
	QMap<QString, QString> map;

	map["vectorName"] = vectorName;
	map["value"] = QString::number(value);

	return map;
}

void ipfModelerProcessChildRangeMoidfyValue::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	vectorName = map["vectorName"];
	value = map["value"].toDouble();
}

void ipfModelerProcessChildRangeMoidfyValue::run()
{
	clearOutFiles();
	clearErrList();

	// 分割矢量
	QStringList shpLists;
	if (!ipfOGR::splitShp(vectorName, shpLists))
	{
		addErrList(QStringLiteral("分割矢量数据失败。"));
		return;
	}

	ipfGdalProgressTools gdal;
	foreach(QString var, filesIn())
	{
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": 读取栅格数据失败，已跳过。"));
			return;
		}
		double nodata = ogr.getNodataValue(1);

		// 用矢量裁切栅格
		QStringList rasterList;
		foreach(QString shp, shpLists)
		{
			// 计算裁切范围
			QgsRectangle rect;
			if (!ogr.shpEnvelope(vectorName, rect))
			{
				addErrList(vectorName + QStringLiteral(": 计算矢量范围失败，已跳过。"));
				return;
			}
			int iRowLu = 0;
			int iColLu = 0;
			int iRowRd = 0;
			int iColRd = 0;
			QList<int> srcList;
			if (!ogr.Projection2ImageRowCol(rect.xMinimum(), rect.yMaximum(), iColLu, iRowLu)
				|| !ogr.Projection2ImageRowCol(rect.xMaximum(), rect.yMinimum(), iColRd, iRowRd))
			{
				addErrList(var + QStringLiteral(": 匹配像元行列失败，无法继续。"));
				return;
			}
			srcList << iColLu << iRowLu << iColRd - iColLu << iRowRd - iRowLu;

			// 使用矢量文件裁切栅格
			QString target = ipfFlowManage::instance()->getTempVrtFile(var);
			QString err = gdal.AOIClip(var, target, shp);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
				continue;
			}

			// 裁切至栅格最小范围
			QString new_target = ipfFlowManage::instance()->getTempVrtFile(target);
			err = gdal.proToClip_Translate_src(target, new_target, srcList);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
				continue;
			}
			//rasterList << new_target;

			// 修改栅格像元值
			QString new_new_target = ipfFlowManage::instance()->getTempVrtFile(new_target);
			err = gdal.pixelFillValue(new_target, new_new_target, nodata, value);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
				continue;
			}

			// 镶嵌回大块
			QString new_new_new_target = ipfFlowManage::instance()->getTempVrtFile(new_new_target);
			err = gdal.mosaic_Buildvrt(QStringList() << var << new_new_target, new_new_new_target);
			if (err.isEmpty())
				appendOutFile(new_new_new_target);
			else
				addErrList(var + ": " + err);
		}

		//// 修改栅格像元值
		//QStringList fillList;
		//fillList << var;
		//foreach (QString var, rasterList)
		//{
		//	QString target = ipfFlowManage::instance()->getTempVrtFile(var);
		//	QString err = gdal.pixelFillValue(var, target, nodata, value);
		//	if (!err.isEmpty())
		//	{
		//		addErrList(var + ": " + err);
		//		continue;
		//	}
		//	fillList << target;
		//}

		//if (!fillList.isEmpty())
		//{
		//	// 镶嵌回大块
		//	QString target = ipfFlowManage::instance()->getTempVrtFile(var);
		//	QString err = gdal.mosaic_Buildvrt(fillList, target);
		//	if (err.isEmpty())
		//		appendOutFile(target);
		//	else
		//		addErrList(var + ": " + err);
		//}
	}
}

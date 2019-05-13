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
		ogr.close();

		// 使用矢量文件裁切栅格
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);
		QString err = gdal.AOIClip(var, target, vectorName);
		if (!err.isEmpty())
		{
			addErrList(var + ": " + err);
			continue;
		}

		// 填充值海域特定值
		if (name() == MODELER_SEAMOIDFYVALUE)
			value = -8888;

		QString target_target = ipfFlowManage::instance()->getTempVrtFile(var);
		err = gdal.pixelFillValue(target, target_target, nodata, value);
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
			addErrList(var + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -1。"));
			continue;
		}

		// 镶嵌回大块
		QStringList mosaicList;
		mosaicList << var;
		mosaicList << targetTo;
		QString targetOut = ipfFlowManage::instance()->getTempVrtFile(var);
		err = gdal.mosaic_Buildvrt(mosaicList, targetOut);
		if (err.isEmpty())
			appendOutFile(targetOut);
		else
			addErrList(var + ": " + err);
	}
}

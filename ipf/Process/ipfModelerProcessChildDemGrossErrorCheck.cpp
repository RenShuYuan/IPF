#include "ipfModelerProcessChildDemGrossErrorCheck.h"
#include "../../ui/ipfModelerDemGrossErrorCheckDialog.h"
#include "../../ui/ipfProgress.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"
#include "QgsRasterLayer.h"
#include "qgscoordinatereferencesystem.h"

ipfModelerProcessChildDemGrossErrorCheck::ipfModelerProcessChildDemGrossErrorCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerDemGrossErrorCheckDialog();
}

ipfModelerProcessChildDemGrossErrorCheck::~ipfModelerProcessChildDemGrossErrorCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildDemGrossErrorCheck::checkParameter()
{
	bool isbl = true;

	QDir dir = QFileInfo(errFile).dir();
	if (!dir.exists())
	{
		addErrList(QStringLiteral("无效的输出路径。"));
		isbl = false;
	}

	if (!QFileInfo(rasterName).exists())
	{
		addErrList(QStringLiteral("无效的输入路径。"));
		return false;
	}
	return isbl;
}

void ipfModelerProcessChildDemGrossErrorCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		rasterName = map["rasterName"];
		errFile = map["errFile"];
		threshold = map["threshold"].toDouble();
	}
}

QMap<QString, QString> ipfModelerProcessChildDemGrossErrorCheck::getParameter()
{
	QMap<QString, QString> map;
	map["rasterName"] = rasterName;
	map["errFile"] = errFile;
	map["threshold"] = QString::number(threshold);

	return map;
}

void ipfModelerProcessChildDemGrossErrorCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	rasterName = map["rasterName"];
	errFile = map["errFile"];
	threshold = map["threshold"].toDouble();
}

void ipfModelerProcessChildDemGrossErrorCheck::run()
{
	clearOutFiles();
	clearErrList();

	QStringList outList;
	ipfGdalProgressTools gdal;

	foreach(QString var, filesIn())
	{
		QList<double> xylist;
		QString epsg;
		QString err;

		// 获得DEM参考坐标系
		QgsRasterLayer* rasterLayer = new QgsRasterLayer(var, "rasterLayer", "gdal");
		if (!rasterLayer->isValid())
		{
			addErrList(var + QStringLiteral(": 数字高程模型读取失败，无法继续。"));
			continue;
		}
		QgsCoordinateReferenceSystem crs = rasterLayer->crs();
		epsg = crs.authid();
		RELEASE(rasterLayer);

		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": 读取数字高程模型失败，无法继续。"));
			continue;
		}
		if (ogr.getBandSize() != 1)
		{
			addErrList(var + QStringLiteral(": 只支持波段数为1的数字高程模型，无法继续。"));
			continue;
		}

		// 获得DEM四至范围
		xylist = ogr.getXY();

		// 利用DEM四至范围计算在参考DEM上的行列位置
		int iRowLu = 0;
		int iColLu = 0;
		int iRowRd = 0;
		int iColRd = 0;
		err = gdal.locationPixelInfo(rasterName, epsg, xylist.at(0), xylist.at(1), iRowLu, iColLu);
		if (!err.isEmpty())
		{
			addErrList(var + QStringLiteral(": 数字高程模型与参考DEM位置匹配失败，请检查参考DEM是否能完全覆盖，无法继续。"));
			continue;
		}
		err = gdal.locationPixelInfo(rasterName, epsg, xylist.at(2), xylist.at(3), iRowRd, iColRd);
		if (!err.isEmpty())
		{
			addErrList(var + QStringLiteral(": 数字高程模型与参考DEM位置匹配失败，请检查参考DEM是否能完全覆盖，无法继续。"));
			continue;
		}
		QList<int> srcList;
		srcList << iColLu << iRowLu << iColRd - iColLu << iRowRd - iRowLu;

		// 裁切参考DEM
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);
		err = gdal.proToClip_Translate_src(rasterName, target, srcList);
		if (!err.isEmpty())
		{
			addErrList(var + QStringLiteral(": 提取参考DEM失败，无法继续。"));
			continue;
		}

		// 计算DEM、小块参考DEM的极值
		double adfMinMax[2];
		double reAdfMinMax[2];
		CPLErr Cerr = ogr.getRasterBand(1)->ComputeRasterMinMax(FALSE, adfMinMax);

		ipfOGR ogrRe(target);
		if (!ogrRe.isOpen())
		{
			addErrList(target + QStringLiteral(": 读取参考数字高程模型失败，无法继续。"));
			continue;
		}
		if (ogrRe.getBandSize() != 1)
		{
			addErrList(var + QStringLiteral(": 只支持波段数为1的数字高程模型，无法继续。"));
			continue;
		}
		CPLErr reCerr = ogrRe.getRasterBand(1)->ComputeRasterMinMax(FALSE, reAdfMinMax);

		if (Cerr != CE_None || reCerr != CE_None)
		{
			addErrList(var + QStringLiteral(": 计算最大、最小值失败，无法继续。"));
			continue;
		}
		// 计算DEM、小块参考DEM的极值 -- 结束

		// 比较差值
		if (abs(adfMinMax[0] - reAdfMinMax[0]) > threshold ||
			abs(adfMinMax[1] - reAdfMinMax[1]) > threshold)
		{
			outList << var + QStringLiteral(": 高程模型最大、最小值超过阈值------>注意核实。");
		}
		else
			outList << var + QStringLiteral(": 高程模型最大、最小值未超过阈值。");
	}

	printErrToFile(errFile, outList);
}

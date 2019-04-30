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
		QFileInfo info(var);
		QString rasterBaseName = info.baseName();

		// 遍历每个矢量面
		int proCount = 0;
		foreach(QString shp, shps)
		{
			proDialog.setValue(++proCount);
			if (proDialog.wasCanceled())
				return;

			// 计算裁切范围
			ipfOGR ogr(var);
			if (!ogr.isOpen())
			{
				addErrList(var + QStringLiteral(": 读取栅格数据失败，已跳过。"));
				return;
			}

			QgsRectangle rect;
			if (!ogr.shpEnvelope(vectorName, rect))
			{
				addErrList(vectorName + QStringLiteral(": 计算矢量范围失败，已跳过。"));
				return;
			}

			// 检查矢量范围是否在栅格范围内
			QgsRectangle rectRaster = ogr.getXY();
			if (!rectRaster.contains(rect))
			{
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": 矢量范围没在栅格范围内。");
				continue;
			}
			ogr.close();

			// 使用矢量文件裁切栅格
			QString target = ipfFlowManage::instance()->getTempVrtFile(var);
			QString err = gdal.AOIClip(var, target, shp);
			if (!err.isEmpty())
			{
				addErrList(rasterBaseName + ": " + err);
				continue;
			}

			// 裁切至栅格最小范围
			QString new_target = ipfFlowManage::instance()->getTempVrtFile(target);
			err = gdal.proToClip_Translate(target, new_target, rect);
			if (!err.isEmpty())
			{
				addErrList(rasterBaseName + ": " + err);
				continue;
			}

			// 检查像元值是否一致
			ipfOGR org(new_target);
			if (!org.isOpen())
			{
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": 检查水平时出现异常错误-1。");
				continue;
			}

			float *pDataBuffer = 0;
			if (!org.readRasterIO(&pDataBuffer))
			{
				outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": 检查水平时出现异常错误-2。");
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

						// 输出错误栅格数据
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
				+ QStringLiteral(": 矢量范围在NODATA区域中。");
			else
			{
				if (isErr)
					outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": 静止水体高程一致。");
				else
					outList << rasterBaseName + "->" + QString::number(proCount) + "("
					+ QString::number(rect.xMinimum(), 'f', 3) + ","
					+ QString::number(rect.yMaximum(), 'f', 3) + ")"
					+ QStringLiteral(": 静止水体高程不一致。");
			}
		}
	}

	QString saveName = outPath + QStringLiteral("/水平检查.txt");
	printErrToFile(saveName, outList);
}

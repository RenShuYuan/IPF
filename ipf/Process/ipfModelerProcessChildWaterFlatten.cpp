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

bool ipfModelerProcessChildWaterFlatten::splitShp(const QString & shpName, QStringList & shps)
{
	GDALDataset *poDS;
	poDS = (GDALDataset*)GDALOpenEx(vectorName.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (poDS == NULL)
	{
		addErrList(vectorName + QStringLiteral(": 读取矢量文件失败，无法继续。"));
		return false;
	}
	OGRLayer *poLayer = poDS->GetLayer(0);
	if (!poLayer)
	{
		addErrList(vectorName + QStringLiteral(": 失败，无法获取矢量图层。"));
		return false;
	}

	OGRFeature * poFeature;
	while ((poFeature = poLayer->GetNextFeature()) != NULL)
	{
		// 加载shp驱动
		const char *pszDriverName = "ESRI Shapefile";
		GDALDriver *poDriver;
		poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
		if (poDriver == NULL)
		{
			addErrList(vectorName + QStringLiteral(": 加载驱动失败。"));
			return false;
		}

		// 创建矢量文件
		GDALDataset *poDS;
		QString new_shp = ipfFlowManage::instance()->getTempFormatFile(vectorName, ".shp");
		poDS = poDriver->Create(new_shp.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
		if (poDS == NULL)
		{
			addErrList(vectorName + QStringLiteral(": 创建临时矢量文件失败。"));
			return false;
		}

		// 创建矢量图层
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer("out", NULL, wkbPolygon, NULL);
		if (poLayer == NULL)
		{
			addErrList(vectorName + QStringLiteral(": 创建图层失败。"));
			return false;
		}

		// 添加要素
		poLayer->CreateFeature(poFeature);
		OGRFeature::DestroyFeature(poFeature);
		GDALClose(poDS);

		shps << new_shp;
	}
	GDALClose(poDS);
	return true;
}

QList<double> ipfModelerProcessChildWaterFlatten::shpEnvelope(const QString & file, const double R)
{
	QList<double> list;

	// 计算栅格四至范围
	GDALDataset *poDS = NULL;
	poDS = (GDALDataset*)GDALOpenEx(file.toStdString().c_str(),
		GDAL_OF_READONLY | GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (!poDS)
	{
		addErrList(file + QStringLiteral(": 失败，无法打开矢量数据。"));
		return list;
	}

	OGRLayer *poLayer = poDS->GetLayer(0);
	if (!poLayer)
	{
		addErrList(file + QStringLiteral(": 失败，无法获取矢量数据。"));
		return list;
	}

	OGREnvelope oExt;
	if (poLayer->GetExtent(0, &oExt, TRUE) != OGRERR_NONE)
	{
		addErrList(file + QStringLiteral(": 失败，无法获取矢量面的范围。"));
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

	// 分割shp
	QStringList shps;
	if (!splitShp(vectorName, shps))
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

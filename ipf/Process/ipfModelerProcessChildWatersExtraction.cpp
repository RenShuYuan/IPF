#include "ipfModelerProcessChildWatersExtraction.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerWatersExtractionDialog.h"
#include "../ipfOgr.h"

#include "qgsvectorlayer.h"

#include <QProgressDialog>

ipfModelerProcessChildWatersExtraction::ipfModelerProcessChildWatersExtraction(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerWatersExtractionDialog();

	fieldName = "EXIN";
}

ipfModelerProcessChildWatersExtraction::~ipfModelerProcessChildWatersExtraction()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildWatersExtraction::checkParameter()
{
	QDir dir(fileName);
	if (!dir.exists())
	{
		addErrList(QStringLiteral("无效的输出文件夹。"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildWatersExtraction::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		fileName = map["fileName"];
		index = map["index"].toDouble();
		minimumArea = map["minimumArea"].toInt();
		minimumRingsArea = map["minimumRingsArea"].toInt();
	}
}

QMap<QString, QString> ipfModelerProcessChildWatersExtraction::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["index"] = QString::number(index);
	map["minimumArea"] = QString::number(minimumArea);
	map["minimumRingsArea"] = QString::number(minimumRingsArea);

	return map;
}

void ipfModelerProcessChildWatersExtraction::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	fileName = map["fileName"];
	index = map["index"].toDouble();
	minimumArea = map["minimumArea"].toInt();
	minimumRingsArea = map["minimumRingsArea"].toInt();
}

double ipfModelerProcessChildWatersExtraction::watersIndex(const double G, const double NIR)
{
	// NDWI
	double index = (G - NIR) / (NIR + G);
	return index;
}

void ipfModelerProcessChildWatersExtraction::run()
{
	clearOutFiles();
	clearErrList();

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, filesIn().size());

	foreach(QString var, filesIn())
	{
		QString baseName = removeDelimiter(var);

		// 提取满足因子要求的栅格数据 ----->
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": 读取栅格数据失败，无法继续。"));
			continue;
		}

		int nBands = ogr.getBandSize();
		if (nBands != 4)
		{
			addErrList(var + QStringLiteral(": 需要4波段的影像数据（R、G、B、NIR），无法继续。"));
			continue;
		}

		int nXSize = ogr.getYXSize().at(1);
		int nYSize = ogr.getYXSize().at(0);
		double nodata1 = ogr.getNodataValue(1);
		double nodata2 = ogr.getNodataValue(2);
		double nodata3 = ogr.getNodataValue(3);
		double nodata4 = ogr.getNodataValue(4);
		QString prj = ogr.getProjection();

		// 创建输出栅格
		QString rasterFile = fileName + "\\" + baseName + "_v.img";
		QString vectorFile = fileName + "\\" + baseName + ".shp";
		GDALDataset* poDataset_target = ogr.createNewRaster(rasterFile, "-9999", 1, GDT_Float32);
		if (!poDataset_target)
		{
			addErrList(rasterFile + QStringLiteral(": 创建输出栅格数据失败，无法继续。"));
			continue;
		}
		GDALRasterBand* datasetBand = poDataset_target->GetRasterBand(1);

		// 分块参数
		int nBlockSize = 1024;
		long blockSize = nBlockSize * nBlockSize * nBands;
		double *pSrcData = new double[blockSize];
		double *pDstData = new double[nBlockSize * nBlockSize];

		// 计算进度条值
		int proX = 0;
		int proY = 0;
		if (nXSize % nBlockSize == 0)
			proX = nXSize / nBlockSize;
		else
			proX = nXSize / nBlockSize + 1;
		if (nYSize % nBlockSize == 0)
			proY = nYSize / nBlockSize;
		else
			proY = nYSize / nBlockSize + 1;
		proDialog.setRangeChild(0, proX * proY);
		proDialog.show();

		//循环分块并进行处理
		int proCount = 0;
		bool isbl = true;
		for (int i = 0; i < nYSize; i += nBlockSize)
		{
			for (int j = 0; j < nXSize; j += nBlockSize)
			{
				// 保存分块实际大小
				int nXBK = nBlockSize;
				int nYBK = nBlockSize;

				//如果最下面和最右边的块不够，剩下多少读取多少
				if (i + nBlockSize > nYSize)
					nYBK = nYSize - i;
				if (j + nBlockSize > nXSize)
					nXBK = nXSize - j;

				long size = nYBK * nXBK * nBands;

				// 读取原始图像块
				if (!ogr.readRasterIO(pSrcData, j, i, nXBK, nYBK, GDT_Float64)) //ogr.getDataType_y()
				{
					addErrList(var + QStringLiteral(": 读取影像分块数据失败，已跳过。"));
					proDialog.setValue(++proCount);
					continue;
				}

				// 处理算法
				for (long mi = 0; (mi + nBands) < size; mi += nBands)
				{
					double B = pSrcData[mi];
					double G = pSrcData[mi + 1];
					double R = pSrcData[mi + 2];
					double NIR = pSrcData[mi + 3];

					// 排除无效值
					if (B == 0 || B == nodata1 ||
						G == 0 || G == nodata2 ||
						R == 0 || R == nodata3 ||
						NIR == 0 || NIR == nodata4)
					{
						pDstData[mi / nBands] = -9999;
						continue;
					}

					// 计算水体指数
					double NDWI = watersIndex( G, NIR);
					if (NDWI > index)
					{
						pDstData[mi / nBands] = NDWI;
					}
					else
					{
						pDstData[mi / nBands] = -9999;
					}
				}

				//写到结果图像
				datasetBand->RasterIO(GF_Write, j, i, nXBK, nYBK, pDstData, nXBK, nYBK, GDT_Float64, 0, 0, 0);

				proDialog.setValue(++proCount);
				if (proDialog.wasCanceled())
					return;
			}
		}

		//释放申请的内存
		RELEASE(pSrcData);
		RELEASE(pDstData);

		if (poDataset_target)
		{
			GDALClose((GDALDatasetH)poDataset_target);
			poDataset_target = nullptr;
		}
		// 提取满足因子要求的栅格数据 -----<

		// 分割栅格，提升栅格转矢量的效率 ----->
		QStringList clipRasers;
		ipfOGR ogr_clip(rasterFile);
		if (!ogr_clip.isOpen())
		{
			addErrList(baseName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -2。"));
			QFile::remove(rasterFile);
			continue;
		}
		if (!ogr_clip.splitRaster(1024, clipRasers))
		{
			addErrList(baseName + QStringLiteral(": 转换矢量失败，已跳过。"));
			QFile::remove(rasterFile);
			continue;
		}

		// 创建矢量图层 ----->
		if (!ipfOGR::createrVectorlayer(vectorFile, QgsWkbTypes::Polygon, QgsFields(), prj))
		{
			addErrList(vectorFile + QStringLiteral(": 创建矢量文件失败，已跳过。"));
			QFile::remove(rasterFile);
			continue;
		}

		// 栅格转矢量 ----->
		ipfGdalProgressTools gdal_v;
		gdal_v.setProgressTitle(QStringLiteral("提取矢量范围"));
		gdal_v.setProgressSize(clipRasers.size());
		gdal_v.showProgressDialog();
		for (int i = 0; i < clipRasers.size(); ++i)
		{
			QString clipRaster = clipRasers.at(i);
			QString err = gdal_v.rasterToVector(clipRaster, vectorFile, 0);
			if (!err.isEmpty())
			{
				addErrList(QStringLiteral("水域提取: ") + err);
				QFile::remove(rasterFile);
				continue;
			}
		}
		gdal_v.hideProgressDialog();
		// 栅格转矢量 -----<

		// 删除临时栅格数据
		QFile::remove(rasterFile);

		// 清除空洞、过滤面积不足的要素 ----->
		QgsVectorLayer *layer = new QgsVectorLayer(vectorFile, "vector");
		if (!layer || !layer->isValid())
		{
			addErrList(vectorFile + QStringLiteral(": 读取矢量数据失败(清除空洞)。"));
			continue;
		}

		QProgressDialog prDialog(QStringLiteral("要素加工..."), QStringLiteral("取消"), 0, layer->featureCount(), nullptr);
		prDialog.setWindowTitle(QStringLiteral("处理进度"));
		prDialog.setWindowModality(Qt::WindowModal);
		prDialog.show();
		int prCount = 0;

		QgsFeature f;
		QList<QgsFeatureId> idList;
		QgsFeatureIterator fList = layer->getFeatures();

		while (fList.nextFeature(f))
		{
			if (f.hasGeometry())
				idList << f.id();
		}

		layer->startEditing();
		int size = idList.size();
		for (int i = 0; i < size; ++i)
		{
			QgsFeature f = layer->getFeature(idList.at(i));

			// 删除碎面
			if (f.geometry().area() < minimumArea)
			{
				layer->deleteFeature(f.id());
				prDialog.setValue(++prCount);
				continue;
			}

			// 删除空洞
			QgsGeometry geo = f.geometry().removeInteriorRings(minimumRingsArea);
			QgsFeature outFeature;
			outFeature.setGeometry(geo);
			QgsAttributes inattrs = f.attributes();
			outFeature.setAttributes(inattrs);
			layer->addFeature(outFeature);
			layer->deleteFeature(f.id());

			prDialog.setValue(++prCount);
			if (prDialog.wasCanceled())
			{
				layer->commitChanges();
				break;
			}
		}
		layer->commitChanges();
		RELEASE(layer);
		// 清除空洞、过滤面积不足的要素 -----<
	}
}


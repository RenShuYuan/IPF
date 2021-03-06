#include "ipfModelerProcessChildVegeataionExtraction.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerVegeataionExtractionDialog.h"
#include "../ipfOgr.h"

#include "qgsvectorlayer.h"

#include <QProgressDialog>

ipfModelerProcessChildVegeataionExtraction::ipfModelerProcessChildVegeataionExtraction(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerVegeataionExtractionDialog();

	fieldName = "EXIN";
}


ipfModelerProcessChildVegeataionExtraction::~ipfModelerProcessChildVegeataionExtraction()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildVegeataionExtraction::checkParameter()
{
	QDir dir(fileName);
	if (!dir.exists())
	{
		addErrList(QStringLiteral("无效的输出文件夹。"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildVegeataionExtraction::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		fileName = map["fileName"];
		index = map["index"].toDouble();
		stlip_index = map["stlip_index"].toDouble();
		minimumArea = map["minimumArea"].toInt();
		minimumRingsArea = map["minimumRingsArea"].toInt();
		buffer = map["buffer"].toInt();
	}
}

QMap<QString, QString> ipfModelerProcessChildVegeataionExtraction::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["index"] = QString::number(index);
	map["stlip_index"] = QString::number(stlip_index);
	map["minimumArea"] = QString::number(minimumArea);
	map["minimumRingsArea"] = QString::number(minimumRingsArea);
	map["buffer"] = QString::number(buffer);

	return map;
}

void ipfModelerProcessChildVegeataionExtraction::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	fileName = map["fileName"];
	index = map["index"].toDouble();
	stlip_index = map["stlip_index"].toDouble();
	minimumArea = map["minimumArea"].toInt();
	minimumRingsArea = map["minimumRingsArea"].toInt();
	buffer = map["buffer"].toInt();
}

double ipfModelerProcessChildVegeataionExtraction::vegeataionIndex(const double R, const double NIR)
{
	double index = (NIR - R) / (NIR + R);
	return index;
}

double ipfModelerProcessChildVegeataionExtraction::ylviIndex(const double B, const double G)
{
	double index = (B - G) / (B + G);
	return index;
}

void ipfModelerProcessChildVegeataionExtraction::run()
{
	clearOutFiles();
	clearErrList();

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, filesIn().size());

	foreach(QString var, filesIn())
	{
		QFileInfo info(var);
		QString baseName = info.baseName();

		// 提取满足NDVI因子要求的栅格数据 ----->
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
		GDALDataset* poDataset_target = ogr.createNewRaster(rasterFile, 1, GDT_Float32);
		if (!poDataset_target)
		{
			addErrList(rasterFile + QStringLiteral(": 创建输出栅格数据失败，无法继续。"));
			continue;
		}
		GDALRasterBand* datasetBand = poDataset_target->GetRasterBand(1);
		datasetBand->SetNoDataValue(-9999);

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
					QApplication::processEvents();
					continue;
				}

				// 处理算法
				for (long mi = 0; (mi + nBands) < size; mi += nBands)
				{
					double B = pSrcData[mi];
					double G = pSrcData[mi+1];
					double R = pSrcData[mi+2];
					double NIR = pSrcData[mi+3];

					// 排除无效值
					if (B == 0 || B == nodata1 ||
						G == 0 || G == nodata2 ||
						R == 0 || R == nodata3 ||
						NIR == 0 || NIR == nodata4)
					{
						pDstData[mi / nBands] = -9999;
						continue;
					}

					// 计算植被指数
					double NDVI = vegeataionIndex(R, NIR);
					if (abs(NDVI) > index)
					{
						if (stlip_index != 0)
						{
							double ylVI = ylviIndex(B, G);
							if (abs(ylVI) > stlip_index)
								pDstData[mi / nBands] = NDVI;
							else
								pDstData[mi / nBands] = -9999; // -9999
						}
						else
							pDstData[mi / nBands] = NDVI;
					}
					else
						pDstData[mi / nBands] = -9999; // -9999
				}

				//写到结果图像
				datasetBand->RasterIO(GF_Write, j, i, nXBK, nYBK, pDstData, nXBK, nYBK, GDT_Float64, 0, 0, 0);

				proDialog.setValue(++proCount);
				QApplication::processEvents();
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
		// 提取满足NDVI因子要求的栅格数据 -----<

		// 分割栅格，提升栅格转矢量的效率 ----->
		QStringList clipRasers;
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

				QList<int> srcList;
				srcList << j << i << nXBK << nYBK;

				ipfGdalProgressTools gdal;
				QString target = ipfFlowManage::instance()->getTempVrtFile(var);
				QString err = gdal.proToClip_Translate_src(rasterFile, target, srcList);
				if (!err.isEmpty())
				{
					addErrList(rasterFile + QStringLiteral(": 栅格分块失败，已跳过。"));
					continue;
				}
				else
					clipRasers << target;
			}
		}
		// 分隔栅格，提升栅格转矢量的效率 -----<

		// 创建矢量文件 ------>
		// 加载shp驱动
		const char *pszDriverName = "ESRI Shapefile";
		GDALDriver *poDriver;
		poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
		if (poDriver == NULL)
		{
			addErrList(vectorFile + QStringLiteral(": 加载驱动失败。"));
			continue;
		}

		// 创建矢量文件
		GDALDataset *poDS;
		poDS = poDriver->Create(vectorFile.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
		if (poDS == NULL)
		{
			addErrList(vectorFile + QStringLiteral(": 创建矢量文件失败。"));
			continue;
		}

		// 创建矢量图层
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer(baseName.toStdString().c_str(), NULL, wkbPolygon, NULL);
		if (poLayer == NULL)
		{
			addErrList(vectorFile + QStringLiteral(": 创建图层失败。"));
			continue;
		}

		// 定义投影
		//poDS->SetProjection(prj.toStdString().c_str());

		// 新增字段
		if (OGRERR_NONE != poLayer->CreateField(new OGRFieldDefn(fieldName.toStdString().c_str(), OFTInteger)))
		{
			addErrList(vectorFile + QStringLiteral(": 创建字段失败。"));
			continue;
		}
		int dst_field = poLayer->GetLayerDefn()->GetFieldIndex(fieldName.toStdString().c_str());
		GDALClose(poDS);
		// 创建矢量文件 ------<

		// 栅格转矢量 ----->
		ipfGdalProgressTools gdal_v;
		gdal_v.setProgressTitle(QStringLiteral("提取矢量范围"));
		gdal_v.setProgressSize(clipRasers.size());
		gdal_v.showProgressDialog();
		for (int i = 0; i < clipRasers.size(); ++i)
		{
			QString clipRaster = clipRasers.at(i);
			QString err = gdal_v.rasterToVector(clipRaster, vectorFile, dst_field);
			if (!err.isEmpty())
			{
				addErrList(QStringLiteral("植被提取: ") + err);
				continue;
			}	
		}
		gdal_v.hideProgressDialog();
		// 栅格转矢量 -----<

		// 融合要素 ----->
		//ipfGdalProgressTools gdal_m;
		//QString err = gdal_m.mergeVector("d:/111.shp", vectorFile, fieldName);
		//if (!err.isEmpty())
		//{
		//	addErrList(QStringLiteral("融合矢量范围: ") + err);
		//	continue;
		//}



		//for (int j = i + 1; j < idList.size(); ++j)
		//{
		//	QgsFeature csf = layer->getFeature(idList.at(j));
		//	if (f.geometry().touches(csf.geometry()))
		//	{
		//		int fIN = f.attribute("IN").toInt();
		//		int csfIN = csf.attribute("IN").toInt();
		//		QgsGeometry geo = f.geometry().combine(csf.geometry());
		//		double area = geo.area();
		//		QgsFeature outFeature;
		//		outFeature.setGeometry(geo);
		//		QgsAttributes inattrs = f.attributes();
		//		outFeature.setAttributes(inattrs);
		//		layer->addFeature(outFeature);
		//		break;
		//	}
		//}

		// 融合要素 -----<

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
			QApplication::processEvents();
			if (prDialog.wasCanceled())
			{
				layer->commitChanges();
				break;
			}
		}
		layer->commitChanges();
		RELEASE(layer);
		// 清除空洞、过滤面积不足的要素 -----<

		/*
		// 光滑
		//QgsFeature outFeature;
		//QgsGeometry inGeo = f.geometry();
		//QgsAttributes inattrs = f.attributes();
		//QgsGeometry outGeo = inGeo.smooth(2, 0.3, -1, 90);
		////QgsGeometry outGeo = inGeo.removeInteriorRings(minimumRingsArea);
		//if (outGeo.isGeosValid())
		//{
		//	outFeature.setGeometry(outGeo);
		//	outFeature.setAttributes(inattrs);
		//	layer->addFeature(outFeature);
		//	layer->deleteFeature(f.id());
		}*/
	}
}

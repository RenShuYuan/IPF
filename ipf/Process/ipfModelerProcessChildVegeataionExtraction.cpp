#include "ipfModelerProcessChildVegeataionExtraction.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerVegeataionExtractionDialog.h"
#include "ipfSpatialGeometryAlgorithm.h"
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
	proDialog.show();

	for (auto srcRaster : filesIn())
	{
		proDialog.userPulsValueTatal();

		QString err;
		ipfOGR ogr_clip;
		QStringList clipRasers;
		ipfSpatialGeometryAlgorithm sga(6);

		QgsVectorLayer *layer_src = nullptr;
		QgsVectorLayer *layer_target = nullptr;
		QgsVectorLayer *layer_target_target = nullptr;

		QString tmpRaster = ipfApplication::instance()->getTempFormatFile(srcRaster, ".tif");
		QString tmpShape = ipfApplication::instance()->getTempFormatFile(srcRaster, ".shp");
		QString tmpShape_dissolve = ipfApplication::instance()->getTempFormatFile(srcRaster, ".shp");
		QString dstShp = fileName + "\\" + ipfApplication::instance()->removeDelimiter(srcRaster) + ".shp";

		// 计算NDVI栅格数据
		ipfGdalProgressTools gdal;
		gdal.showProgressDialog();
		err = gdal.calculateNDVI(srcRaster, tmpRaster, index, stlip_index);
		if (!err.isEmpty()) goto end;

		// 分割栅格
		if (!ogr_clip.open(tmpRaster))
		{
			err = QStringLiteral(": 输出检查结果失败，请自行核查该数据 -2。");
			goto end;
		}
		if (!ogr_clip.splitRaster(BLOCKSIZE_VECTOR, clipRasers))
		{
			err = QStringLiteral(": 转换矢量失败，已跳过。");
			goto end;
		}

		// 创建矢量文件
		if (!ipfOGR::createrVectorFile(tmpShape, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection()))
		{
			err = QStringLiteral(": 创建矢量文件失败，已跳过。");
			goto end;
		}

		// 栅格转矢量
		gdal.setProgressTitle(QStringLiteral("提取矢量范围"));
		gdal.setProgressSize(clipRasers.size());
		gdal.showProgressDialog();
		for (int i = 0; i < clipRasers.size(); ++i)
		{
			QString clipRaster = clipRasers.at(i);
			err = gdal.rasterToVector(clipRaster, tmpShape, 0);
			if (!err.isEmpty()) goto end;
		}
		gdal.hideProgressDialog();

		// 融合矢量
		layer_src = new QgsVectorLayer(tmpShape, "vector");
		if (!layer_src || !layer_src->isValid())
		{
			err = QStringLiteral("读取临时文件失败。");
			goto end;
		}
		layer_target = ipfOGR::createrVectorlayer(tmpShape_dissolve, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection());
		if (layer_target == nullptr)
		{
			err = QStringLiteral("创建临时文件失败。");
			goto end;
		}

		err = sga.dissovle(layer_src, layer_target, QStringList());

		// 处理矢量碎面
		layer_target_target = ipfOGR::createrVectorlayer(dstShp, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection());
		if (layer_target_target == nullptr)
		{
			err = QStringLiteral("创建矢量文件失败。");
			goto end;
		}
		if (!sga.clearPolygonANDring(layer_target, layer_target_target, minimumArea, minimumRingsArea))
		{
			err = QStringLiteral("整理矢量文件失败。");
			goto end;
		}

	end:
		if (!err.isEmpty()) addErrList(srcRaster + ": " + err);
		QFile::remove(tmpRaster);

		RELEASE(layer_src);
		RELEASE(layer_target);
		RELEASE(layer_target_target);
	}
}

#include "ipfModelerProcessChildWatersExtraction.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerWatersExtractionDialog.h"
#include "ipfSpatialGeometryAlgorithm.h"
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

void ipfModelerProcessChildWatersExtraction::run()
{
	clearOutFiles();
	clearErrList();

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, filesIn().size());
	proDialog.show();

	for(auto srcRaster : filesIn())
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

		// 计算NDWI栅格数据
		ipfGdalProgressTools gdal;
		gdal.showProgressDialog();
		err = gdal.calculateNDWI(srcRaster, tmpRaster, index);
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


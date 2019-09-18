#include "ipfModelerProcessChildSlopCalculation.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "ipfSpatialGeometryAlgorithm.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"

ipfModelerProcessChildSlopCalculation::ipfModelerProcessChildSlopCalculation(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
}


ipfModelerProcessChildSlopCalculation::~ipfModelerProcessChildSlopCalculation()
{
}

bool ipfModelerProcessChildSlopCalculation::checkParameter()
{
	return true;
}

void ipfModelerProcessChildSlopCalculation::setParameter()
{
}

QMap<QString, QString> ipfModelerProcessChildSlopCalculation::getParameter()
{
	return QMap<QString, QString>();
}

void ipfModelerProcessChildSlopCalculation::setDialogParameter(QMap<QString, QString> map)
{
}

void ipfModelerProcessChildSlopCalculation::run()
{
	clearOutFiles();
	clearErrList();
	double threshold = 0.3;
	ipfGdalProgressTools gdal;
	gdal.showProgressDialog();

	QString srcRaster = "D:/testData/dissovle/zy302a_MS.pix";
	QString dstRaster = "D:/testData/dissovle/zy302a_MS_out.img";
	QString tmpShape = "D:/testData/dissovle/zy302a_MS_out_tmp.shp";
	QString dstShape = "D:/testData/dissovle/zy302a_MS_dissole.shp";
	QString dstShp = "D:/testData/dissovle/zy302a_MS.shp";

	// 计算NDWI栅格数据
	QString err = gdal.calculateNDWI(srcRaster, dstRaster, threshold);
	if (!err.isEmpty())
	{
		addErrList(srcRaster + ": " + err);
		return;
	}

	// 分割栅格
	QStringList clipRasers;
	ipfOGR ogr_clip(dstRaster);
	if (!ogr_clip.isOpen())
	{
		addErrList(srcRaster + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -2。"));
		return;
	}
	if (!ogr_clip.splitRaster(BLOCKSIZE_VECTOR, clipRasers))
	{
		addErrList(srcRaster + QStringLiteral(": 转换矢量失败，已跳过。"));
		return;
	}

	// 创建矢量文件
	if (!ipfOGR::createrVectorFile(tmpShape, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection()))
	{
		addErrList(srcRaster + QStringLiteral(": 创建矢量文件失败，已跳过。"));
		return;
	}

	// 栅格转矢量
	ipfGdalProgressTools gdal_v;
	gdal_v.setProgressTitle(QStringLiteral("提取矢量范围"));
	gdal_v.setProgressSize(clipRasers.size());
	gdal_v.showProgressDialog();
	for (int i = 0; i < clipRasers.size(); ++i)
	{
		QString clipRaster = clipRasers.at(i);
		QString err = gdal_v.rasterToVector(clipRaster, tmpShape, 0);
		if (!err.isEmpty())
		{
			addErrList(QStringLiteral("水域提取: ") + err);
			return;
		}
	}
	gdal_v.hideProgressDialog();

	// 融合矢量
	QgsVectorLayer *layer_src = new QgsVectorLayer(tmpShape, "vector");
	if (!layer_src || !layer_src->isValid())
	{
		addErrList(QStringLiteral("读取临时文件失败。"));
		return;
	}
	QgsVectorLayer *layer_target = ipfOGR::createrVectorlayer(dstShape, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection());
	if (layer_target == nullptr)
	{
		addErrList(QStringLiteral("创建临时文件失败。"));
		return;
	}

	ipfSpatialGeometryAlgorithm sga(6);
	err = sga.dissovle(layer_src, layer_target, QStringList());
	RELEASE(layer_src);

	// 处理矢量碎面
	QgsVectorLayer *layer_target_target = ipfOGR::createrVectorlayer(dstShp, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection());
	if (layer_target_target == nullptr)
	{
		addErrList(QStringLiteral("创建矢量文件失败。"));
		return;
	}
	if (!sga.clearPolygonANDring(layer_target, layer_target_target, 1000, 1000))
	{
		addErrList(QStringLiteral("整理矢量文件失败。"));
	}
	RELEASE(layer_target);
	RELEASE(layer_target_target);
}

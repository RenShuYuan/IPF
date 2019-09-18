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

	// ����NDWIդ������
	QString err = gdal.calculateNDWI(srcRaster, dstRaster, threshold);
	if (!err.isEmpty())
	{
		addErrList(srcRaster + ": " + err);
		return;
	}

	// �ָ�դ��
	QStringList clipRasers;
	ipfOGR ogr_clip(dstRaster);
	if (!ogr_clip.isOpen())
	{
		addErrList(srcRaster + QStringLiteral(": ��������ʧ�ܣ������к˲������ -2��"));
		return;
	}
	if (!ogr_clip.splitRaster(BLOCKSIZE_VECTOR, clipRasers))
	{
		addErrList(srcRaster + QStringLiteral(": ת��ʸ��ʧ�ܣ���������"));
		return;
	}

	// ����ʸ���ļ�
	if (!ipfOGR::createrVectorFile(tmpShape, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection()))
	{
		addErrList(srcRaster + QStringLiteral(": ����ʸ���ļ�ʧ�ܣ���������"));
		return;
	}

	// դ��תʸ��
	ipfGdalProgressTools gdal_v;
	gdal_v.setProgressTitle(QStringLiteral("��ȡʸ����Χ"));
	gdal_v.setProgressSize(clipRasers.size());
	gdal_v.showProgressDialog();
	for (int i = 0; i < clipRasers.size(); ++i)
	{
		QString clipRaster = clipRasers.at(i);
		QString err = gdal_v.rasterToVector(clipRaster, tmpShape, 0);
		if (!err.isEmpty())
		{
			addErrList(QStringLiteral("ˮ����ȡ: ") + err);
			return;
		}
	}
	gdal_v.hideProgressDialog();

	// �ں�ʸ��
	QgsVectorLayer *layer_src = new QgsVectorLayer(tmpShape, "vector");
	if (!layer_src || !layer_src->isValid())
	{
		addErrList(QStringLiteral("��ȡ��ʱ�ļ�ʧ�ܡ�"));
		return;
	}
	QgsVectorLayer *layer_target = ipfOGR::createrVectorlayer(dstShape, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection());
	if (layer_target == nullptr)
	{
		addErrList(QStringLiteral("������ʱ�ļ�ʧ�ܡ�"));
		return;
	}

	ipfSpatialGeometryAlgorithm sga(6);
	err = sga.dissovle(layer_src, layer_target, QStringList());
	RELEASE(layer_src);

	// ����ʸ������
	QgsVectorLayer *layer_target_target = ipfOGR::createrVectorlayer(dstShp, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection());
	if (layer_target_target == nullptr)
	{
		addErrList(QStringLiteral("����ʸ���ļ�ʧ�ܡ�"));
		return;
	}
	if (!sga.clearPolygonANDring(layer_target, layer_target_target, 1000, 1000))
	{
		addErrList(QStringLiteral("����ʸ���ļ�ʧ�ܡ�"));
	}
	RELEASE(layer_target);
	RELEASE(layer_target_target);
}

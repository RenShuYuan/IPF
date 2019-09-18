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
		addErrList(QStringLiteral("��Ч������ļ��С�"));
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

		// ����NDVIդ������
		ipfGdalProgressTools gdal;
		gdal.showProgressDialog();
		err = gdal.calculateNDVI(srcRaster, tmpRaster, index, stlip_index);
		if (!err.isEmpty()) goto end;

		// �ָ�դ��
		if (!ogr_clip.open(tmpRaster))
		{
			err = QStringLiteral(": ��������ʧ�ܣ������к˲������ -2��");
			goto end;
		}
		if (!ogr_clip.splitRaster(BLOCKSIZE_VECTOR, clipRasers))
		{
			err = QStringLiteral(": ת��ʸ��ʧ�ܣ���������");
			goto end;
		}

		// ����ʸ���ļ�
		if (!ipfOGR::createrVectorFile(tmpShape, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection()))
		{
			err = QStringLiteral(": ����ʸ���ļ�ʧ�ܣ���������");
			goto end;
		}

		// դ��תʸ��
		gdal.setProgressTitle(QStringLiteral("��ȡʸ����Χ"));
		gdal.setProgressSize(clipRasers.size());
		gdal.showProgressDialog();
		for (int i = 0; i < clipRasers.size(); ++i)
		{
			QString clipRaster = clipRasers.at(i);
			err = gdal.rasterToVector(clipRaster, tmpShape, 0);
			if (!err.isEmpty()) goto end;
		}
		gdal.hideProgressDialog();

		// �ں�ʸ��
		layer_src = new QgsVectorLayer(tmpShape, "vector");
		if (!layer_src || !layer_src->isValid())
		{
			err = QStringLiteral("��ȡ��ʱ�ļ�ʧ�ܡ�");
			goto end;
		}
		layer_target = ipfOGR::createrVectorlayer(tmpShape_dissolve, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection());
		if (layer_target == nullptr)
		{
			err = QStringLiteral("������ʱ�ļ�ʧ�ܡ�");
			goto end;
		}

		err = sga.dissovle(layer_src, layer_target, QStringList());

		// ����ʸ������
		layer_target_target = ipfOGR::createrVectorlayer(dstShp, QgsWkbTypes::Polygon, QgsFields(), ogr_clip.getProjection());
		if (layer_target_target == nullptr)
		{
			err = QStringLiteral("����ʸ���ļ�ʧ�ܡ�");
			goto end;
		}
		if (!sga.clearPolygonANDring(layer_target, layer_target_target, minimumArea, minimumRingsArea))
		{
			err = QStringLiteral("����ʸ���ļ�ʧ�ܡ�");
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

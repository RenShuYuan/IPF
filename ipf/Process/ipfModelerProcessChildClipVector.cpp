#include "ipfModelerProcessChildClipVector.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerClipVectorDialog.h"
#include "../ipfOgr.h"

#include <QFileInfo>

ipfModelerProcessChildClipVector::ipfModelerProcessChildClipVector(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	clip = new ipfModelerClipVectorDialog();
}


ipfModelerProcessChildClipVector::~ipfModelerProcessChildClipVector()
{
	if (clip) { delete clip; }
}

bool ipfModelerProcessChildClipVector::checkParameter()
{
	bool isbl = true;

	if (vectorName.isEmpty())
	{
		isbl = false;
		addErrList(QStringLiteral("没有选择矢量文件。"));
	}
	else
	{
		QFileInfo info(vectorName);
		if (!info.exists())
		{
			isbl = false;
			addErrList(QStringLiteral("矢量文件路径无效。"));
		}
	}

	return isbl;
}

void ipfModelerProcessChildClipVector::setParameter()
{
	if (clip->exec())
	{
		vectorName = clip->getParameter();
	}
}

void ipfModelerProcessChildClipVector::setDialogParameter(QMap<QString, QString> map)
{
	clip->setParameter(map);

	vectorName = map["vectorName"];
}

QMap<QString, QString> ipfModelerProcessChildClipVector::getParameter()
{
	QMap<QString, QString> map;
	map["vectorName"] = vectorName;

	return map;
}

void ipfModelerProcessChildClipVector::run()
{
	clearOutFiles();
	clearErrList();

	// 暂时读取第一个影像处理
	if (filesIn().size() != 1)
	{
		addErrList(QStringLiteral("警告：该功能只支持对单一数据源处理"
			"，若您需要处理多块数据"
			"，请在前面插入“镶嵌”模块以解决该问题。"
			"有多个数据源输入时，自动提取第一个进行处理。"));
	}
	QString soucre = filesIn().at(0);

	// 获取影像分辨率
	ipfOGR ogr(soucre);
	if (!ogr.isOpen())
	{
		addErrList(soucre + QStringLiteral(": 1.读取影像分辨率失败，无法继续。"));
		return;
	}
	double R = ogr.getPixelSize();
	ogr.close();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	QString target = ipfFlowManage::instance()->getTempVrtFile(soucre);

	QString err = gdal.AOIClip(soucre, target, vectorName);
	if (!err.isEmpty())
	{
		addErrList(soucre + ": 2." + err);
		return;
	}

	// 裁切多余画布
	GDALDataset *poDS = NULL;
	poDS = (GDALDataset*)GDALOpenEx(vectorName.toStdString().c_str(),
		GDAL_OF_READONLY | GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (!poDS)
	{
		addErrList(soucre + ": 3.失败，无法打开矢量数据。");
		return;
	}

	OGRLayer *poLayer = poDS->GetLayer(0);
	if (!poLayer)
	{
		addErrList(soucre + ": 4.失败，无法获取矢量图层。");
		return;
	}

	OGREnvelope oExt;
	for (int iGeom = 0; iGeom < 1; ++iGeom)
	{
		if (poLayer->GetExtent(iGeom, &oExt, TRUE) != OGRERR_NONE)
		{
			addErrList(soucre + ": 5.失败，无法获取矢量面的范围。");
			return;
		}
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

	QString new_target = ipfFlowManage::instance()->getTempVrtFile(target);
	err = gdal.proToClip_Translate(target, new_target, QList<double>() << ulx << uly << lrx << lry);
	if (err.isEmpty())
		appendOutFile(new_target);
	else
		addErrList(soucre + ": 6." + err);
}

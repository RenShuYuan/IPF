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
		addErrList(QStringLiteral("û��ѡ��ʸ���ļ���"));
	}
	else
	{
		QFileInfo info(vectorName);
		if (!info.exists())
		{
			isbl = false;
			addErrList(QStringLiteral("ʸ���ļ�·����Ч��"));
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

	// ��ʱ��ȡ��һ��Ӱ����
	if (filesIn().size() != 1)
	{
		addErrList(QStringLiteral("���棺�ù���ֻ֧�ֶԵ�һ����Դ����"
			"��������Ҫ����������"
			"������ǰ����롰��Ƕ��ģ���Խ�������⡣"
			"�ж������Դ����ʱ���Զ���ȡ��һ�����д���"));
	}
	QString soucre = filesIn().at(0);

	// ��ȡӰ��ֱ���
	ipfOGR ogr(soucre);
	if (!ogr.isOpen())
	{
		addErrList(soucre + QStringLiteral(": 1.��ȡӰ��ֱ���ʧ�ܣ��޷�������"));
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

	// ���ж��໭��
	GDALDataset *poDS = NULL;
	poDS = (GDALDataset*)GDALOpenEx(vectorName.toStdString().c_str(),
		GDAL_OF_READONLY | GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (!poDS)
	{
		addErrList(soucre + ": 3.ʧ�ܣ��޷���ʸ�����ݡ�");
		return;
	}

	OGRLayer *poLayer = poDS->GetLayer(0);
	if (!poLayer)
	{
		addErrList(soucre + ": 4.ʧ�ܣ��޷���ȡʸ��ͼ�㡣");
		return;
	}

	OGREnvelope oExt;
	for (int iGeom = 0; iGeom < 1; ++iGeom)
	{
		if (poLayer->GetExtent(iGeom, &oExt, TRUE) != OGRERR_NONE)
		{
			addErrList(soucre + ": 5.ʧ�ܣ��޷���ȡʸ����ķ�Χ��");
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

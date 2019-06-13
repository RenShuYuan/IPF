#include "ipfModelerProcessChildSlopCalculation.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "ipfSpatialGeometryAlgorithm.h"
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

	// ��ȡʸ��ͼ��
	QString vectorName = "D:/testData/dissovle/fw.gpkg";
	QgsVectorLayer *layer_src = new QgsVectorLayer(vectorName, "vector");
	if (!layer_src || !layer_src->isValid())
	{
		addErrList(QStringLiteral("��ȡԴʸ������ͼ��ʧ�ܡ�"));
		return;
	}

	// �������ʸ��ͼ��
	QString vectorFileOut = "D:/testData/dissovle/fwOut.gpkg";
	if (!ipfOGR::createrVectorlayer(vectorFileOut, QgsWkbTypes::Polygon, layer_src->fields(), layer_src->crs()))
	{
		addErrList(QStringLiteral("����Ŀ��ʸ������ͼ��ʧ�ܡ�"));
		return;
	}
	QgsVectorLayer *layer_target = new QgsVectorLayer(vectorFileOut, "vector");
	if (!layer_target || !layer_target->isValid())
	{
		addErrList(QStringLiteral("��ȡĿ��ʸ������ͼ��ʧ�ܡ�"));
		return;
	}

	ipfSpatialGeometryAlgorithm ipfsga(12);
	QString err = ipfsga.dissovle(layer_src, layer_target, QStringList()<< "GB" << "TYPE");
	addErrList(err);

	RELEASE(layer_src);
	RELEASE(layer_target);
}

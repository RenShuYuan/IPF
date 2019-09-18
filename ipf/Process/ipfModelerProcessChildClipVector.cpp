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
	dialog = new ipfModelerClipVectorDialog();
}


ipfModelerProcessChildClipVector::~ipfModelerProcessChildClipVector()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildClipVector::checkParameter()
{
	if (!QFileInfo(vectorName).exists())
	{
		addErrList(QStringLiteral("ʸ���ļ�·����Ч��"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildClipVector::setParameter()
{
	if (dialog->exec())
	{
		vectorName = dialog->getParameter();
	}
}

void ipfModelerProcessChildClipVector::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

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

	// ������з�Χ
	ipfOGR ogr(soucre);
	if (!ogr.isOpen())
	{
		addErrList(soucre + QStringLiteral(": ��ȡդ������ʧ�ܣ���������"));
		return;
	}

	QgsRectangle rect;
	CPLErr gErr = ogr.shpEnvelope(vectorName, rect);
	if (gErr == CE_Failure)
	{
		addErrList(soucre + QStringLiteral(": ����ʸ����Χʧ�ܣ���������"));
		return;
	}
	else if (gErr == CE_Warning)
		return;

	QList<int> srcList;
	int iRowLu = 0, iColLu = 0, iRowRd = 0, iColRd = 0;
	if (!ogr.Projection2ImageRowCol(rect.xMinimum(), rect.yMaximum(), iColLu, iRowLu)
		|| !ogr.Projection2ImageRowCol(rect.xMaximum(), rect.yMinimum(), iColRd, iRowRd))
	{
		addErrList(soucre + QStringLiteral(": ƥ����Ԫλ��ʧ�ܣ��޷�������"));
		return;
	}
	srcList << iColLu << iRowLu << iColRd - iColLu + 1 << iRowRd - iRowLu + 1;

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	QString target = ipfApplication::instance()->getTempVrtFile(soucre);

	QString err = gdal.AOIClip(soucre, target, vectorName);
	if (!err.isEmpty())
	{
		addErrList(soucre + ": " + err);
		return;
	}

	QString new_target = ipfApplication::instance()->getTempVrtFile(soucre);
	err = gdal.proToClip_Translate_src(target, new_target, srcList);
	if (err.isEmpty())
		appendOutFile(new_target);
	else
		addErrList(soucre + ": " + err);
}

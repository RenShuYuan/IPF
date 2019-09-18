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
		addErrList(QStringLiteral("矢量文件路径无效。"));
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

	// 暂时读取第一个影像处理
	if (filesIn().size() != 1)
	{
		addErrList(QStringLiteral("警告：该功能只支持对单一数据源处理"
			"，若您需要处理多块数据"
			"，请在前面插入“镶嵌”模块以解决该问题。"
			"有多个数据源输入时，自动提取第一个进行处理。"));
	}
	QString soucre = filesIn().at(0);

	// 计算裁切范围
	ipfOGR ogr(soucre);
	if (!ogr.isOpen())
	{
		addErrList(soucre + QStringLiteral(": 读取栅格数据失败，已跳过。"));
		return;
	}

	QgsRectangle rect;
	CPLErr gErr = ogr.shpEnvelope(vectorName, rect);
	if (gErr == CE_Failure)
	{
		addErrList(soucre + QStringLiteral(": 计算矢量范围失败，已跳过。"));
		return;
	}
	else if (gErr == CE_Warning)
		return;

	QList<int> srcList;
	int iRowLu = 0, iColLu = 0, iRowRd = 0, iColRd = 0;
	if (!ogr.Projection2ImageRowCol(rect.xMinimum(), rect.yMaximum(), iColLu, iRowLu)
		|| !ogr.Projection2ImageRowCol(rect.xMaximum(), rect.yMinimum(), iColRd, iRowRd))
	{
		addErrList(soucre + QStringLiteral(": 匹配像元位置失败，无法继续。"));
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

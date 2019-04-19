#include "ipfModelerProcessChildFracClip.h"
#include "../ui/ipfModelerFracClipDialog.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfFractalmanagement.h"
#include "../ipfOgr.h"
#include "ipfFlowManage.h"
#include <QFileInfo>

ipfModelerProcessChildFracClip::ipfModelerProcessChildFracClip(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	clip = new ipfModelerFracClipDialog();
}

ipfModelerProcessChildFracClip::~ipfModelerProcessChildFracClip()
{
	RELEASE(clip);
}

bool ipfModelerProcessChildFracClip::checkParameter()
{
	bool isbl = true;

	// 验证图号
	isbl = readTh();

	return isbl;
}

void ipfModelerProcessChildFracClip::setParameter()
{
	if (clip->exec())
	{
		QMap<QString, QString> map = clip->getParameter();
		fileName = map["fileName"];
		dateType = map["dateType"];
	}
}

QMap<QString, QString> ipfModelerProcessChildFracClip::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["dateType"] = dateType;

	return map;
}

void ipfModelerProcessChildFracClip::setDialogParameter(QMap<QString, QString> map)
{
	clip->setParameter(map);

	fileName = map["fileName"];
	dateType = map["dateType"];
}

void ipfModelerProcessChildFracClip::run()
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
		addErrList(soucre + QStringLiteral(": 读取影像分辨率失败，无法继续。"));
		return;
	}
	double R = ogr.getPixelSize();
	ogr.close();

	// 检查分辨率正确否
	if (R != 10.0 && R != 2.0 && R != 16.0)
	{
		addErrList(soucre + QStringLiteral(": 栅格数据分辨率与全球项目设计不符，无法继续。"));
		return;
	}

	// 定义比例尺
	int blc = 50000;
	if (R == 16.0)
		blc = 250000;
	ipfFractalManagement frac(blc);

	// 计算每幅图的外扩距离，并裁切
	ipfGdalProgressTools gdal;
	gdal.setProgressSize(thList.size());
	gdal.showProgressDialog();

	QStringList outList;
	foreach(QString var, thList)
	{
		QString err;

		QList<QgsPointXY> four;
		four = frac.dNToXy(var);

		if (four.size() != 4)
		{
			addErrList(var + QStringLiteral(": 坐标计算失败。"));
			continue;
		}

		// 计算外扩范围
		int ext = 50;
		if (R == 2.0)
			ext = 200;
		else if (R == 16.0)
			ext = 100;

		QList<double> list = ipfFractalManagement::external(four, R, ext);

		QString target;
		QString hemisphere = "N";
		if (var.size() == 11)
			hemisphere = "";

		if (dateType == "DOM")
			target = ipfFlowManage::instance()->getTempVrtFile(hemisphere + var + "DOMU");
		else if (dateType == "DSM")
			target = ipfFlowManage::instance()->getTempVrtFile(hemisphere + var + "DSMU");
		else
			target = ipfFlowManage::instance()->getTempVrtFile(hemisphere + var + "DEMU");

		// 当数据范围不正确时，会自动进行重采样
		err = gdal.proToClip_Warp(soucre, target, list);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

bool ipfModelerProcessChildFracClip::readTh() // 把图号验证放这里
{
	clearErrList();
	thList.clear();

	// 读取文件内容
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		addErrList(file.fileName() + QStringLiteral(": 无法读取图号文件。"));
		return false;
	}
	
	bool isOk = true;
	QTextStream in(&file);

	ipfFractalManagement frac;
	while (!in.atEnd())
	{
		QString line = in.readLine();
		if (!line.isEmpty())
		{
			if (frac.effectiveness(line))
				thList.append(line);
			else
			{
				addErrList(line + QStringLiteral(": 无效的分幅图号。"));
				isOk = false;
			}
		}
	}
	file.close();
	return isOk;
}
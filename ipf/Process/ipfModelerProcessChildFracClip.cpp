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
	if (clip) { delete clip; }
}

bool ipfModelerProcessChildFracClip::checkParameter()
{
	bool isbl = true;

	// 验证图号
	isbl = readTh();

	if (ext < 0)
	{
		isbl = false;
		addErrList(QStringLiteral("外扩像元数量不能为负数。"));
	}

	return isbl;
}

void ipfModelerProcessChildFracClip::setParameter()
{
	if (clip->exec())
	{
		QMap<QString, QString> map = clip->getParameter();
		fileName = map["fileName"];
		saveName = map["saveName"];
		ext = map["ext"].toInt();
		isChecked = map["isChecked"];
	}
}

QMap<QString, QString> ipfModelerProcessChildFracClip::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["saveName"] = saveName;
	map["isChecked"] = isChecked;
	map["ext"] = QString::number(ext);

	return map;
}

void ipfModelerProcessChildFracClip::setDialogParameter(QMap<QString, QString> map)
{
	clip->setParameter(map);

	fileName = map["fileName"];
	saveName = map["saveName"];
	ext = map["ext"].toInt();
}

void ipfModelerProcessChildFracClip::run()
{
	clearOutFiles();
	clearErrList();

	ipfFractalManagement frac(50000);

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
	QString strR = QString::number(R, 'f', 11);
	R = strR.toDouble();
	ogr.close();

	// 计算每幅图的外扩距离，并裁切
	ipfGdalProgressTools gdal;
	gdal.setProgressSize(thList.size());
	gdal.showProgressDialog();

	QStringList outList;
	foreach(QString var, thList)
	{
		QString err;

		QList<QgsPointXY> four;
		if (R < 0.1)
			four = frac.dNToLal(var);
		else
			four = frac.dNToXy(var);

		if (four.size() != 4)
		{
			addErrList(var + QStringLiteral(": 坐标计算失败。"));
			continue;
		}

		// 计算外扩范围
		QList<double> list = ipfFractalManagement::external(four, R, ext);

		// 用于范围输出
		QList<double> listR = ipfFractalManagement::external(four, R, ext, false);
		outList << QString("%1 %2 %3 %4 %5").arg(var)
			.arg(listR.at(0), 0, 'f', 11).arg(listR.at(1), 0, 'f', 11)
			.arg(listR.at(2), 0, 'f', 11).arg(listR.at(3), 0, 'f', 11);

		QString target;
		if (R < 0.1)
			target  = ipfFlowManage::instance()->getTempVrtFile('N' + var + 'G');
		else
			target  = ipfFlowManage::instance()->getTempVrtFile('N' + var + 'U');

		// 当数据范围不正确时，会自动进行重采样
		err = gdal.proToClip_Warp(soucre, target, list);	//
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}

	// 输出外扩坐标
	if (!saveName.isEmpty())
	{
		QFile file(saveName);
		if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
		{
			addErrList(saveName + QStringLiteral("创建文件失败，已终止。"));
			return;
		}
		QTextStream out(&file);
		foreach(QString str, outList)
			out << str << endl;
		file.close();
	}
}

bool ipfModelerProcessChildFracClip::readTh() // 把图号验证放这里
{
	// 读取文件内容
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		addErrList(file.fileName() + QStringLiteral(": 无法读取图号文件。"));
		return false;
	}
	
	thList.clear();
	bool isOk = true;
	QTextStream in(&file);
	ipfFractalManagement frac(50000);
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
#include "ipfModelerProcessChildZCheck.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../../ui/ipfModelerZCheckDialog.h"
#include "../../ui/ipfProgress.h"
#include "../ipfOgr.h"

#include <QFile>
#include <QFileInfo>

ipfModelerProcessChildZCheck::ipfModelerProcessChildZCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerZCheckDialog();
}

ipfModelerProcessChildZCheck::~ipfModelerProcessChildZCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildZCheck::checkParameter()
{
	bool isbl = true;

	if (!QFile::exists(flies))
		isbl = false;

	return isbl;
}

void ipfModelerProcessChildZCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		flies = map["flies"];
		saveName = map["saveName"];
	}
}

QMap<QString, QString> ipfModelerProcessChildZCheck::getParameter()
{
	QMap<QString, QString> map;
	map["flies"] = flies;
	map["saveName"] = saveName;

	return map;
}

void ipfModelerProcessChildZCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	flies = map["flies"];
	saveName = map["saveName"];
}

void ipfModelerProcessChildZCheck::run()
{
	clearOutFiles();
	clearErrList();
	QList <QStringList> jcList;

	// 读取检查点信息
	QFile file(flies);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		addErrList(file.fileName() + QStringLiteral(": 无法读取文件。"));
		return;
	}
	QTextStream in(&file);
	while (!in.atEnd())
	{
		QString line = in.readLine();
		QStringList list = line.split(QRegExp("[ ,\t]"), QString::SkipEmptyParts);
		if (list.size() != 4)
			addErrList(line + QStringLiteral(": 格式不正确。"));
		else
			jcList << list;
	}
	file.close();

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, filesIn().size());
	proDialog.setRangeChild(0, jcList.size());
	proDialog.show();

	ipfGdalProgressTools gdal;
	foreach(QString var, filesIn())
	{
		// 打开栅格
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": 读取影像源失败，无法继续。"));
			continue;
		}

		// 检查波段
		if (ogr.getBandSize() != 1)
		{
			addErrList(var + QStringLiteral(": 该功能针对的是单波段高程数字模型，波段数量不正确。"));
			continue;
		}

		// 获得栅格NODATA值
		double nodata = ogr.getNodataValue(1);

		// 获得栅格外接矩形范围
		QgsRectangle rect = ogr.getXY();

		// 遍历检查点
		QStringList outLines;
		double count = 0.0;
		double maxValue = 0.0;

		for( int i=0; i<jcList.size(); ++i)
		{
			proDialog.setValue(i+1);
			if (proDialog.wasCanceled())
				return;

			int iCol = 0;
			int iRow = 0;
			QStringList errs = jcList.at(i);
			double dProjX = errs.at(1).toDouble();
			double dProjY = errs.at(2).toDouble();
			double dProjZ = errs.at(3).toDouble();
			double value = 0.0;

			// 检查点是否在栅格的外接矩形范围内
			if (!rect.contains(QgsPointXY(dProjX, dProjY)))
				continue;

			if (ogr.getPixelValue(1, dProjX, dProjY, value))
			{
				// 忽略NODATA
				if (value != nodata)
				{
					double dd = dProjZ - value;

					outLines << errs.at(0) + ' ' + errs.at(1) + ' ' + errs.at(2) + ' ' + errs.at(3)
						+ ' ' + QString::number(value, 'f', 3) + ' ' + QString::number(dd, 'f', 3);
					count += dd * dd;
					if (fabs(dd) > fabs(maxValue))
						maxValue = dd;
				}
			}
			else
			{
				addErrList(errs.at(0) + QStringLiteral(": 查询像元值失败。"));
			}
		}

		// 输出错误文件
		QFileInfo info(var);
		QString flie = saveName + "/" + info.baseName() + ".txt";
		QFile outFile(flie);
		if (!outFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
		{
			addErrList(flie + QStringLiteral("创建文件失败，已终止。"));
			return;
		}
		QTextStream out(&outFile);
		out << QStringLiteral("备查点名称 X Y Z DsmZ Z-DsmZ") << endl;

		foreach(QString str, outLines)
		{
			out << str << endl;
		}

		count = count / outLines.size();
		count = sqrt(count);
		out << QStringLiteral("总点数 = %1, 最大误差 = %2, 高程中误差 = ±%3")
			.arg(outLines.size()).arg(maxValue).arg(QString::number(count, 'f', 3));

		outFile.close();
	}
}

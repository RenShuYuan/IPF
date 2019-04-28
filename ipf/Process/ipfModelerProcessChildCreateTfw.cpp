#include "ipfModelerProcessChildCreateTfw.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "ipfFlowManage.h"
#include "../ipfOgr.h"
#include <QMessageBox>

ipfModelerProcessChildCreateTfw::ipfModelerProcessChildCreateTfw(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
}


ipfModelerProcessChildCreateTfw::~ipfModelerProcessChildCreateTfw()
{
}

bool ipfModelerProcessChildCreateTfw::checkParameter()
{
	return true;
}

void ipfModelerProcessChildCreateTfw::setParameter()
{
	QMessageBox::about(0, QStringLiteral("创建TFW")
		, QStringLiteral("该模块创建全球测图项目专用TFW文件，\n输出路径与接收的影像路径保存一致。"));
}

void ipfModelerProcessChildCreateTfw::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;

	foreach(QString var, filesIn())
	{
		// 获取影像分辨率与角点坐标
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": 读取影像失败，无法继续。"));
			continue;
		}
		double R = ogr.getPixelSize();
		QgsRectangle rect = ogr.getXY();
		ogr.close();

		// 创建TFW
		QString target = var.mid(0, var.size()-3) + "tfw";
		QFile file(target);
		if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
		{
			addErrList(target + QStringLiteral("创建tfw失败，已终止。"));
			continue;
		}
		QTextStream out(&file);

		if (R<0.1)
		{
			out << QString::number(R, 'f', 11) << endl;
			out << QString::number(0, 'f', 11) << endl;
			out << QString::number(0, 'f', 11) << endl;
			out << '-' + QString::number(R, 'f', 11) << endl;
			out << QString::number(rect.xMinimum() + R / 2, 'f', 11) << endl;
			out << QString::number(rect.yMaximum() - R / 2, 'f', 11) << endl;
		}
		else
		{
			out << QString::number(R, 'f', 3) << endl;
			out << QString::number(0, 'f', 3) << endl;
			out << QString::number(0, 'f', 3) << endl;
			out << '-' + QString::number(R, 'f', 3) << endl;
			out << QString::number(rect.xMinimum() + R / 2, 'f', 3) << endl;
			out << QString::number(rect.yMaximum() - R / 2, 'f', 3) << endl;
		}

		file.close();
	}
}

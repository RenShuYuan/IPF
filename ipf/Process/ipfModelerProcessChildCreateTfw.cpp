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
	QMessageBox::about(0, QStringLiteral("����TFW")
		, QStringLiteral("��ģ�鴴��ȫ���ͼ��Ŀר��TFW�ļ���\n���·������յ�Ӱ��·������һ�¡�"));
}

void ipfModelerProcessChildCreateTfw::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;

	foreach(QString var, filesIn())
	{
		// ��ȡӰ��ֱ�����ǵ�����
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡӰ��ʧ�ܣ��޷�������"));
			continue;
		}
		double R = ogr.getPixelSize();
		QgsRectangle rect = ogr.getXY();
		ogr.close();

		// ����TFW
		QString target = var.mid(0, var.size()-3) + "tfw";
		QFile file(target);
		if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
		{
			addErrList(target + QStringLiteral("����tfwʧ�ܣ�����ֹ��"));
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

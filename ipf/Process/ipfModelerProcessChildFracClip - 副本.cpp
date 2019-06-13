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

	// ��֤ͼ��
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
		addErrList(soucre + QStringLiteral(": ��ȡӰ��ֱ���ʧ�ܣ��޷�������"));
		return;
	}
	double R = ogr.getPixelSize();
	ogr.close();

	// ���ֱ�����ȷ��
	//if (R != GGI_DOM_2M && R != GGI_DOM_16M && R != GGI_DEM)
	//{
	//	addErrList(soucre + QStringLiteral(": դ�����ݷֱ�����ȫ����Ŀ��Ʋ������޷�������"));
	//	return;
	//}

	// ���������
	//int blc = 50000;
	//if (R == GGI_DOM_16M)
	//	blc = 250000;
	ipfFractalManagement frac(10000);

	// ����ÿ��ͼ���������룬������
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
			addErrList(var + QStringLiteral(": �������ʧ�ܡ�"));
			continue;
		}

		// ��������λС��
		for (int i = 0; i < four.size(); ++i)
		{
			QgsPointXY point = four.at(i);
			double x = QString::number(point.x(), 'f', 3).toDouble();
			double y = QString::number(point.y(), 'f', 3).toDouble();
			four[i] = QgsPointXY(x, y);
		}

		// ����������Χ
		int ext = 50;
		if (R == GGI_DOM_2M)
			ext = 200;
		else if (R == GGI_DOM_16M)
			ext = 100;

		QList<double> list = ipfFractalManagement::external(four, R, 100);

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

		// �����ݷ�Χ����ȷʱ�����Զ������ز���
		err = gdal.proToClip_Warp(soucre, target, list);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

bool ipfModelerProcessChildFracClip::readTh() // ��ͼ����֤������
{
	clearErrList();
	thList.clear();

	// ��ȡ�ļ�����
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		addErrList(file.fileName() + QStringLiteral(": �޷���ȡͼ���ļ���"));
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
				addErrList(line + QStringLiteral(": ��Ч�ķַ�ͼ�š�"));
				isOk = false;
			}
		}
	}
	file.close();
	return isOk;
}
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

	// ��֤ͼ��
	isbl = readTh();

	if (ext < 0)
	{
		isbl = false;
		addErrList(QStringLiteral("������Ԫ��������Ϊ������"));
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
	QString strR = QString::number(R, 'f', 11);
	R = strR.toDouble();
	ogr.close();

	// ����ÿ��ͼ���������룬������
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
			addErrList(var + QStringLiteral(": �������ʧ�ܡ�"));
			continue;
		}

		// ����������Χ
		QList<double> list = ipfFractalManagement::external(four, R, ext);

		// ���ڷ�Χ���
		QList<double> listR = ipfFractalManagement::external(four, R, ext, false);
		outList << QString("%1 %2 %3 %4 %5").arg(var)
			.arg(listR.at(0), 0, 'f', 11).arg(listR.at(1), 0, 'f', 11)
			.arg(listR.at(2), 0, 'f', 11).arg(listR.at(3), 0, 'f', 11);

		QString target;
		if (R < 0.1)
			target  = ipfFlowManage::instance()->getTempVrtFile('N' + var + 'G');
		else
			target  = ipfFlowManage::instance()->getTempVrtFile('N' + var + 'U');

		// �����ݷ�Χ����ȷʱ�����Զ������ز���
		err = gdal.proToClip_Warp(soucre, target, list);	//
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}

	// �����������
	if (!saveName.isEmpty())
	{
		QFile file(saveName);
		if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
		{
			addErrList(saveName + QStringLiteral("�����ļ�ʧ�ܣ�����ֹ��"));
			return;
		}
		QTextStream out(&file);
		foreach(QString str, outList)
			out << str << endl;
		file.close();
	}
}

bool ipfModelerProcessChildFracClip::readTh() // ��ͼ����֤������
{
	// ��ȡ�ļ�����
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		addErrList(file.fileName() + QStringLiteral(": �޷���ȡͼ���ļ���"));
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
				addErrList(line + QStringLiteral(": ��Ч�ķַ�ͼ�š�"));
				isOk = false;
			}
		}
	}
	file.close();
	return isOk;
}
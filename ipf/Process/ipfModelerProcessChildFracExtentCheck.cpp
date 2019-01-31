#include "ipfModelerProcessChildFracExtentCheck.h"
#include "../ui/ipfModelerFracExtentCheckDialog.h"
#include "../ipfFractalmanagement.h"
#include "../ipfOgr.h"

#include <QFileInfo>
#include <QProgressDialog>

ipfModelerProcessChildFracExtentCheck::ipfModelerProcessChildFracExtentCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerFracExtentCheckDialog();
}


ipfModelerProcessChildFracExtentCheck::~ipfModelerProcessChildFracExtentCheck()
{
	if (dialog) { delete dialog; }
}

bool ipfModelerProcessChildFracExtentCheck::checkParameter()
{
	if (ext < 0)
	{
		addErrList(QStringLiteral("������Ԫ��������Ϊ������"));
		return false;
	}

	return true;
}

void ipfModelerProcessChildFracExtentCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		saveName = map["saveName"];
		ext = map["ext"].toInt();
	}
}

QMap<QString, QString> ipfModelerProcessChildFracExtentCheck::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;
	map["ext"] = QString::number(ext);

	return map;
}

void ipfModelerProcessChildFracExtentCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	saveName = map["saveName"];
	ext = map["ext"].toInt();
}

void ipfModelerProcessChildFracExtentCheck::run()
{
	clearOutFiles();
	clearErrList();

	//������
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("դ�����ݷ�Χ���..."), QStringLiteral("ȡ��"), 0, filesIn().size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("դ�����ݷ�Χ���"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	QStringList outList;
	ipfFractalManagement frac(50000);
	foreach(QString var, filesIn())
	{
		dialog.setValue(++prCount);
		QApplication::processEvents();
		if (dialog.wasCanceled())
			return;

		// ���ͼ���Ƿ���ȷ
		QFileInfo info(var);
		QString fileName = info.baseName();

		// ȥ��ǰ��׺ N G/U
		fileName = fileName.mid(1, 10);

		if (!frac.effectiveness(fileName))
		{
			addErrList(fileName + QStringLiteral(": ��Ч��ͼ�š�"));
			continue;
		}

		// ��Ӱ�񣬲���ȡ����
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡӰ��ʧ�ܣ��޷�������"));
			continue;
		}
		QList<double> xyList = ogr.getXY();
		double R = ogr.getPixelSize();
		QString strR = QString::number(R, 'f', 12);
		R = strR.toDouble();
		ogr.close();
		if (xyList.size() != 4)
		{
			addErrList(var + QStringLiteral(": ��ȡӰ��������Χʧ�ܣ��޷�������"));
			continue;
		}

		// ����ͼ������
		QList<QgsPointXY> four;
		if (R < 0.1)
			four = frac.dNToLal(fileName);
		else
			four = frac.dNToXy(fileName);

		if (four.size() != 4)
		{
			addErrList(var + QStringLiteral(": �������ʧ�ܡ�"));
			continue;
		}

		// ������������
		QList<double> extList = ipfFractalManagement::external(four, R, ext);

		// ����Ƚ�
		//QString xy0 = QString::number(xyList.at(0), 'f', 11);
		//QString xy1 = QString::number(xyList.at(1), 'f', 11);
		//QString xy2 = QString::number(xyList.at(2), 'f', 11);
		//QString xy3 = QString::number(xyList.at(3), 'f', 11);
		qulonglong xy0l = xyList.at(0) * 100000000000;
		qulonglong xy1l = xyList.at(1) * 100000000000;
		qulonglong xy2l = xyList.at(2) * 100000000000;
		qulonglong xy3l = xyList.at(3) * 100000000000;

		//QString ext0 = QString::number(extList.at(0), 'f', 11);
		//QString ext1 = QString::number(extList.at(1), 'f', 11);
		//QString ext2 = QString::number(extList.at(2), 'f', 11);
		//QString ext3 = QString::number(extList.at(3), 'f', 11);
		qulonglong ext0l = extList.at(0) * 100000000000;
		qulonglong ext1l = extList.at(1) * 100000000000;
		qulonglong ext2l = extList.at(2) * 100000000000;
		qulonglong ext3l = extList.at(3) * 100000000000;

		bool isbl = false;
		long c0 = abs((long)(xy0l - ext0l));
		long c1 = abs((long)(xy1l - ext1l));
		long c2 = abs((long)(xy2l - ext2l));
		long c3 = abs((long)(xy3l - ext3l));
		if (R < 0.1)
		{
			if (c0 < 2 && c1 < 2 && c2 < 2 && c3 < 2)
				isbl = true;
		}
		else
		{
			if (c0 == 0 && c1 == 0 && c2 == 0 && c3 == 0)
				isbl = true;
		}
		if (isbl)
		{
			outList << fileName + QStringLiteral(": ��Χ��ȷ��");
		}
		else
		{
			QString("%1").arg(var);
			outList << fileName
				+ QStringLiteral(": ��Χ����\n\tͼ�����꣺") + QString("%1, %2, %3, %4").arg(xyList.at(0), 0, 'f', 13).arg(xyList.at(1), 0, 'f', 13).arg(xyList.at(2), 0, 'f', 13).arg(xyList.at(3), 0, 'f', 13)
				+ QStringLiteral("\n\t�������꣺") + QString("%1, %2, %3, %4").arg(extList.at(0), 0, 'f', 13).arg(extList.at(1), 0, 'f', 13).arg(extList.at(2), 0, 'f', 13).arg(extList.at(3), 0, 'f', 13);
		}
	}

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

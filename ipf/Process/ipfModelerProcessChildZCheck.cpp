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

	// ��ȡ������Ϣ
	QFile file(flies);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		addErrList(file.fileName() + QStringLiteral(": �޷���ȡ�ļ���"));
		return;
	}
	QTextStream in(&file);
	while (!in.atEnd())
	{
		QString line = in.readLine();
		QStringList list = line.split(QRegExp("[ ,\t]"), QString::SkipEmptyParts);
		if (list.size() != 4)
			addErrList(line + QStringLiteral(": ��ʽ����ȷ��"));
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
		// ��դ��
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡӰ��Դʧ�ܣ��޷�������"));
			continue;
		}

		// ��鲨��
		if (ogr.getBandSize() != 1)
		{
			addErrList(var + QStringLiteral(": �ù�����Ե��ǵ����θ߳�����ģ�ͣ�������������ȷ��"));
			continue;
		}

		// ���դ��NODATAֵ
		double nodata = ogr.getNodataValue(1);

		// ���դ����Ӿ��η�Χ
		QgsRectangle rect = ogr.getXY();

		// ��������
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

			// �����Ƿ���դ�����Ӿ��η�Χ��
			if (!rect.contains(QgsPointXY(dProjX, dProjY)))
				continue;

			if (ogr.getPixelValue(1, dProjX, dProjY, value))
			{
				// ����NODATA
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
				addErrList(errs.at(0) + QStringLiteral(": ��ѯ��Ԫֵʧ�ܡ�"));
			}
		}

		// ��������ļ�
		QFileInfo info(var);
		QString flie = saveName + "/" + info.baseName() + ".txt";
		QFile outFile(flie);
		if (!outFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
		{
			addErrList(flie + QStringLiteral("�����ļ�ʧ�ܣ�����ֹ��"));
			return;
		}
		QTextStream out(&outFile);
		out << QStringLiteral("��������� X Y Z DsmZ Z-DsmZ") << endl;

		foreach(QString str, outLines)
		{
			out << str << endl;
		}

		count = count / outLines.size();
		count = sqrt(count);
		out << QStringLiteral("�ܵ��� = %1, ������ = %2, �߳������ = ��%3")
			.arg(outLines.size()).arg(maxValue).arg(QString::number(count, 'f', 3));

		outFile.close();
	}
}

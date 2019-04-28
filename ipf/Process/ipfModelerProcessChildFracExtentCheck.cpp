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
	RELEASE(dialog);
}

bool ipfModelerProcessChildFracExtentCheck::checkParameter()
{
	QDir dir = QFileInfo(saveName).dir();
	if (!dir.exists())
	{
		addErrList(QStringLiteral("��Ч������ļ��С�"));
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
	}
}

QMap<QString, QString> ipfModelerProcessChildFracExtentCheck::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;

	return map;
}

void ipfModelerProcessChildFracExtentCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	saveName = map["saveName"];
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
	foreach(QString var, filesIn())
	{
		dialog.setValue(++prCount);
		QApplication::processEvents();
		if (dialog.wasCanceled())
			return;

		QFileInfo info(var);
		QString fileName = info.baseName();

		// ȥ����׺
		fileName = fileName.mid(0, 11);

		// ��Ӱ�񣬲���ȡ����
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡӰ��ʧ�ܣ��޷�������"));
			continue;
		}
		QgsRectangle rect = ogr.getXY();
		double R = ogr.getPixelSize();
		ogr.close();

		int blc = 50000;
		if (R == 16.0)
			blc = 250000;
		ipfFractalManagement frac(blc);

		// ���ͼ���Ƿ���ȷ
		if (!frac.effectiveness(fileName))
		{
			addErrList(fileName + QStringLiteral(": ��Ч��ͼ�š�"));
			continue;
		}

		// ����ͼ������
		QList<QgsPointXY> four;
		four = frac.dNToXy(fileName);

		if (four.size() != 4)
		{
			addErrList(var + QStringLiteral(": �������ʧ�ܡ�"));
			continue;
		}

		int ext = 0;
		if (R == 2.0) // 2��DOM
			ext = 200;
		else if (R == 16.0) // 16��DOM
			ext = 100;
		else if (R == 10.0) // DSM/DEM
			ext = 50;
		else
		{
			outList << fileName + QStringLiteral(": ��Ԫ��С����ȷ����Χ���ʧ�ܣ���");
			continue;
		}

		// ������������
		QList<double> extList = ipfFractalManagement::external(four, R, ext);

		// ����Ƚ�
		bool isbl = false;
		double c0 = rect.xMinimum() - extList.at(0);
		double c1 = rect.yMaximum() - extList.at(1);
		double c2 = rect.xMaximum() - extList.at(2);
		double c3 = rect.yMinimum() - extList.at(3);

		if (c0 == 0.0 && c1 == 0.0 && c2 == 0.0 && c3 == 0.0)
			isbl = true;

		if (isbl)
		{
			outList << fileName + QStringLiteral(": ��Χ��ȷ��");
		}
		else
		{
			QString("%1").arg(var);
			outList << fileName
				+ QStringLiteral(": ��Χ����\n\tͼ�����꣺") + QString("%1, %2, %3, %4").arg(rect.xMinimum(), 0, 'f', 6).arg(rect.yMaximum(), 0, 'f', 6).arg(rect.xMaximum(), 0, 'f', 6).arg(rect.yMinimum(), 0, 'f', 6)
				+ QStringLiteral("\n\t�������꣺") + QString("%1, %2, %3, %4").arg(extList.at(0), 0, 'f', 6).arg(extList.at(1), 0, 'f', 6).arg(extList.at(2), 0, 'f', 6).arg(extList.at(3), 0, 'f', 6);
		}
	}

	printErrToFile(saveName, outList);
}

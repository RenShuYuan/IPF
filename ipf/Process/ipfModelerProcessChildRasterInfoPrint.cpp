#include "ipfModelerProcessChildRasterInfoPrint.h"
#include "../ipfOgr.h"
#include "../ipf/gdal/ipfgdalprogresstools.h"
#include "../../ui/ipfModelerRasterInfoPrintDialog.h"

#include <QDir>
#include <QFileInfo>
#include <QProgressDialog>

ipfModelerProcessChildRasterInfoPrint::ipfModelerProcessChildRasterInfoPrint(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerRasterInfoPrintDialog();
}


ipfModelerProcessChildRasterInfoPrint::~ipfModelerProcessChildRasterInfoPrint()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildRasterInfoPrint::checkParameter()
{
	QDir dir = QFileInfo(saveName).dir();
	if (!dir.exists())
	{
		addErrList(QStringLiteral("��Ч������ļ��С�"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildRasterInfoPrint::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		saveName = map["saveName"];
	}
}

QMap<QString, QString> ipfModelerProcessChildRasterInfoPrint::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;

	return map;
}

void ipfModelerProcessChildRasterInfoPrint::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	saveName = map["saveName"];
}

void ipfModelerProcessChildRasterInfoPrint::run()
{
	clearOutFiles();
	clearErrList();
	QStringList outList;

	//������
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("դ����Ϣ���..."), QStringLiteral("ȡ��"), 0, filesIn().size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("դ����Ϣ���"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	foreach(QString var, filesIn())
	{
		dialog.setValue(++prCount);
		QApplication::processEvents();
		if (dialog.wasCanceled())
			return;

		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": դ�����ݶ�ȡʧ�ܡ�"));
			continue;
		}

		// դ������
		QFileInfo info(var);
		outList << info.baseName();

		// �ֱ��� X Y
		double pSize = ogr.getPixelSize();
		if (pSize == -98765.4)
			outList << QStringLiteral("\t��Ԫ��С: ����������һ�£�");
		else
			outList << QStringLiteral("\t��Ԫ��С: %1").arg(pSize);

		// ������
		outList << QStringLiteral("\t��������: %1").arg(ogr.getBandSize());

		// λ��
		outList << QStringLiteral("\tλ��: ") + ipfGdalProgressTools::enumTypeToString(ogr.getDataType());

		// NODATA
		for (int i = 1; i <= ogr.getBandSize(); ++i)
			outList << QStringLiteral("\tNODATA: %1").arg(ogr.getNodataValue(i));

		// ѹ��
		outList << QStringLiteral("\tѹ����ʽ: %1").arg(ogr.getCompressionName());

		// ͶӰ��Ϣ
		outList << QStringLiteral("\t����ϵͳ: %1").arg(ogr.getProjection());
	}

	// �������
	printErrToFile(saveName, outList);
}

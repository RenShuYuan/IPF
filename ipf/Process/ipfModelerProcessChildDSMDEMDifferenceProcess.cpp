#include "ipfModelerProcessChildDSMDEMDifferenceProcess.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerDSMDEMDifferenceProcessDialog.h"

#include <QProgressDialog>

ipfModelerProcessChildDSMDEMDifferenceProcess::ipfModelerProcessChildDSMDEMDifferenceProcess(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	dialog = new ipfModelerDSMDEMDifferenceProcessDialog();
}

ipfModelerProcessChildDSMDEMDifferenceProcess::~ipfModelerProcessChildDSMDEMDifferenceProcess()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildDSMDEMDifferenceProcess::checkParameter()
{
	if (typeName.isEmpty())
	{
		addErrList(QStringLiteral("��δ���ò�����"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildDSMDEMDifferenceProcess::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		typeName = map["type"];
		threshold = map["threshold"].toDouble();
		if (map["isFillNodata"] == "YES")
			isFillNodata = true;
		else
			isFillNodata = false;
	}
}

QMap<QString, QString> ipfModelerProcessChildDSMDEMDifferenceProcess::getParameter()
{
	QMap<QString, QString> map;

	map["type"] = typeName;
	map["threshold"] = QString::number(threshold);

	if (isFillNodata)
		map["isFillNodata"] = "YES";
	else
		map["isFillNodata"] = "NO";

	return map;
}

void ipfModelerProcessChildDSMDEMDifferenceProcess::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	typeName = map["type"];
	threshold = map["threshold"].toDouble();
	if (map["isFillNodata"] == "YES")
		isFillNodata = true;
	else
		isFillNodata = false;
}

int ipfModelerProcessChildDSMDEMDifferenceProcess::getFilesIndex(const QStringList & lists, const QString & th)
{
	int index = -1;
	QRegExp strExp("(\\\\" + th + ".)");
	for (int i = 0; i < lists.size(); ++i)
	{
		if (lists.at(i).contains(strExp))
		{
			index = i;
			break;
		}
	}
	return index;
}

void ipfModelerProcessChildDSMDEMDifferenceProcess::run()
{
	clearOutFiles();
	clearErrList();

	// �����ļ�������DSM������
	QRegExp regexp("(DSM)");
	QStringList filesDSM;
	for (int i = 0; i < filesIn().size(); ++i)
	{
		QString dsm = filesIn().at(i);
		if (QFileInfo(dsm).baseName().contains(regexp))
		{
			filesDSM << dsm;
		}
	}

	//������
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("ƥ��DEM����..."), QStringLiteral("ȡ��"), 0, 0, nullptr);
	dialog.setWindowTitle(QStringLiteral("ƥ��DEM����"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	ipfGdalProgressTools gdal;

	foreach(QString dsm, filesDSM)
	{
		QString dem = QFileInfo(dsm).baseName();
		QString fileName = dem;
		dem = dem.replace("DSM", "DEM");

		// ƥ��DEM����
		int index = getFilesIndex(filesIn(), dem);
		if (index == -1)
		{
			addErrList( fileName + QStringLiteral(": û���������б���ƥ�䵽��Ӧ��DEM���ݣ���������"));
			continue;
		}
		dem = filesIn().at(index);

		QString var;
		if (typeName == "DSM")
			var = dsm;
		else
			var = dem;

		QString target = ipfApplication::instance()->getTempFormatFile(var, ".vrt");
		QString err = gdal.dsmdemDiffeProcess(dsm, dem, target, typeName, threshold, isFillNodata);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

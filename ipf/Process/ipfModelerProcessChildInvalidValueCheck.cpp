#include "ipfModelerProcessChildInvalidValueCheck.h"
#include "../../ui/ipfModelerInvalidValueCheckDialog.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"

ipfModelerProcessChildInvalidValueCheck::ipfModelerProcessChildInvalidValueCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerInvalidValueCheckDialog();
}

ipfModelerProcessChildInvalidValueCheck::~ipfModelerProcessChildInvalidValueCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildInvalidValueCheck::checkParameter()
{
	if (!QDir(saveName).exists())
	{
		addErrList(QStringLiteral("��Ч������ļ��С�"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildInvalidValueCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();

		invalidValue = map["invalidValue"];
		saveName = map["saveName"];

		if (map["isNegative"] == "YES")
			isNegative = true;
		else
			isNegative = false;
		if (map["isNodata"] == "YES")
			isNodata = true;
		else
			isNodata = false;
	}
}

QMap<QString, QString> ipfModelerProcessChildInvalidValueCheck::getParameter()
{
	QMap<QString, QString> map;

	map["invalidValue"] = invalidValue;
	map["saveName"] = saveName;

	if (isNegative)
		map["isNegative"] = "YES";
	else
		map["isNegative"] = "NO";
	if (isNodata)
		map["isNodata"] = "YES";
	else
		map["isNodata"] = "NO";

	return map;
}

void ipfModelerProcessChildInvalidValueCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	invalidValue = map["invalidValue"];
	saveName = map["saveName"];

	if (map["isNegative"] == "YES")
		isNegative = true;
	else
		isNegative = false;
	if (map["isNodata"] == "YES")
		isNodata = true;
	else
		isNodata = false;
}

void ipfModelerProcessChildInvalidValueCheck::run()
{
	clearOutFiles();
	clearErrList();

	QStringList outList;

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		QString rasterFileName = QFileInfo(var).fileName();
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);

		QString err = gdal.filterInvalidValue(var, target, invalidValue, isNegative, isNodata);
		if (err.isEmpty())
		{
			// ���Ϊimg�ļ�
			QString format = "img";
			QString targetTo = saveName + "\\" + removeDelimiter(target) + '.' + format;
			QString err = gdal.formatConvert(target, targetTo, gdal.enumFormatToString(format), "NONE", "NO", "none");
			if (!err.isEmpty())
			{
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -1��"));
				continue;
			}

			// ���������Сֵ
			double adfMinMax[2];
			ipfOGR ogr(targetTo);
			if (!ogr.isOpen())
			{
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -2��"));
				continue;
			}
			if (ogr.getBandSize() != 1)
			{
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -3��"));
				continue;
			}
			CPLErr cerr = ogr.getRasterBand(1)->ComputeRasterMinMax(TRUE, adfMinMax);
			ogr.close();

			if (cerr != CE_None)
			{
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -4��"));
				continue;
			}

			// ����դ���Ƿ������Чֵ
			if (adfMinMax[0] == 0 && adfMinMax[1] == 0)
			{
				QFile::remove(targetTo);
				outList << rasterFileName + QStringLiteral(": ��ȷ��");
			}
			else
			{
				outList << rasterFileName + QStringLiteral(": ��鵽դ�������д�����Чֵ���������դ���б����Ϊ1��");
			}
		}
		else
			addErrList(var + ": " + err);
	}

	QString outName = saveName + QStringLiteral("/��Чֵ���.txt");
	QFile file(outName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		addErrList(outName + QStringLiteral("���������ļ�ʧ�ܣ�����ֹ��"));
		return;
	}
	QTextStream out(&file);
	foreach(QString str, outList)
		out << str << endl;
	file.close();
}

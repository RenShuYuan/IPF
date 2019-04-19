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
		addErrList(QStringLiteral("无效的输出文件夹。"));
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
			// 输出为img文件
			QString format = "img";
			QString targetTo = saveName + "\\" + removeDelimiter(target) + '.' + format;
			QString err = gdal.formatConvert(target, targetTo, gdal.enumFormatToString(format), "NONE", "NO", "none");
			if (!err.isEmpty())
			{
				addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -1。"));
				continue;
			}

			// 计算最大最小值
			double adfMinMax[2];
			ipfOGR ogr(targetTo);
			if (!ogr.isOpen())
			{
				addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -2。"));
				continue;
			}
			if (ogr.getBandSize() != 1)
			{
				addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -3。"));
				continue;
			}
			CPLErr cerr = ogr.getRasterBand(1)->ComputeRasterMinMax(TRUE, adfMinMax);
			ogr.close();

			if (cerr != CE_None)
			{
				addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -4。"));
				continue;
			}

			// 检查该栅格是否存在无效值
			if (adfMinMax[0] == 0 && adfMinMax[1] == 0)
			{
				QFile::remove(targetTo);
				outList << rasterFileName + QStringLiteral(": 正确。");
			}
			else
			{
				outList << rasterFileName + QStringLiteral(": 检查到栅格数据中存在无效值，并在输出栅格中被标记为1。");
			}
		}
		else
			addErrList(var + ": " + err);
	}

	QString outName = saveName + QStringLiteral("/无效值检查.txt");
	QFile file(outName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		addErrList(outName + QStringLiteral("创建错误文件失败，已终止。"));
		return;
	}
	QTextStream out(&file);
	foreach(QString str, outList)
		out << str << endl;
	file.close();
}

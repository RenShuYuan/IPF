#include "ipfModelerProcessChildInvalidValueCheck.h"
#include "../../ui/ipfModelerInvalidValueCheckDialog.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"

#include <QProgressDialog>

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
		if (map["isShape"] == "YES")
			isShape = true;
		else
			isShape = false;
		if (map["bands_noDiffe"] == "YES")
			bands_noDiffe = true;
		else
			bands_noDiffe = false;
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
	if (isShape)
		map["isShape"] = "YES";
	else
		map["isShape"] = "NO";
	if (bands_noDiffe)
		map["bands_noDiffe"] = "YES";
	else
		map["bands_noDiffe"] = "NO";

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
	if (map["isShape"] == "YES")
		isShape = true;
	else
		isShape = false;
	if (map["bands_noDiffe"] == "YES")
		bands_noDiffe = true;
	else
		bands_noDiffe = false;
}

void ipfModelerProcessChildInvalidValueCheck::run()
{
	clearOutFiles();
	clearErrList();

	QStringList outList;
	ipfGdalProgressTools gdal;
	
	//进度条
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("数据处理..."), QStringLiteral("取消"), 0, filesIn().size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("数据处理"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	foreach(QString var, filesIn())
	{
		dialog.setValue(++prCount);
		QApplication::processEvents();

		QString rasterFileName = QFileInfo(var).fileName();
		//QString target = ipfApplication::instance()->getTempVrtFile(var);

		QString target = ipfApplication::instance()->getTempFormatFile(var, ".tif");
		QString err = gdal.filterInvalidValue(var, target, invalidValue, isNegative, isNodata, bands_noDiffe);
		if (err.isEmpty())
		{
			// 计算最大最小值
			ipfOGR ogr(target);
			if (!ogr.isOpen())
			{
				addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -2。"));
				continue;
			}

			CPLErr cerr = ogr.ComputeMinMax(IPF_ZERO);
			QString wkt = ogr.getProjection();

			if (cerr == CE_Warning)
			{
				// 输出文件
				QString format = "tif";
				QString targetTo = saveName + "\\" + ipfApplication::instance()->removeDelimiter(target) + '.' + format;
				QString err = gdal.formatConvert(target, targetTo, gdal.enumFormatToString(format), "NONE", "NO", "0");
				if (!err.isEmpty())
				{
					addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -1。"));
					continue;
				}
				outList << rasterFileName + QStringLiteral(": 检查到栅格数据中存在无效值，并在输出栅格中被标记为1。");

				// 是否输出矢量
				if (isShape)
				{
					// 分割栅格，提升栅格转矢量的效率
					QStringList clipRasers;
					if (!ogr.splitRaster(BLOCKSIZE_VECTOR, clipRasers))
					{
						addErrList(rasterFileName + QStringLiteral(": 输出错误矢量失败，已跳过。"));
						continue;
					}

					// 创建矢量图层 ----->
					QString vectorFile = targetTo.mid(0, targetTo.size()-3) + "shp";
					if (!ipfOGR::createrVectorFile(vectorFile, QgsWkbTypes::Polygon, QgsFields(), wkt))
					{
						addErrList(rasterFileName + QStringLiteral(": 创建错误矢量文件失败，已跳过。"));
						continue;
					}

					// 栅格转矢量 ----->
					ipfGdalProgressTools gdal_v;
					gdal_v.setProgressTitle(QStringLiteral("提取矢量范围"));
					gdal_v.setProgressSize(clipRasers.size());
					gdal_v.showProgressDialog();
					for (int i = 0; i < clipRasers.size(); ++i)
					{
						QString clipRaster = clipRasers.at(i);
						QString err = gdal_v.rasterToVector(clipRaster, vectorFile, 0);
						if (!err.isEmpty())
						{
							addErrList(rasterFileName + ": " + err);
							continue;
						}
					}
					// 栅格转矢量 -----<
				}
			}
			else if (cerr == CE_None)
				outList << rasterFileName + QStringLiteral(": 正确。");
			else
				addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -4。"));
		}
		else
			addErrList(var + ": " + err);
	}

	QString outName = saveName + QStringLiteral("/无效值检查.txt");
	printErrToFile(outName, outList);
}

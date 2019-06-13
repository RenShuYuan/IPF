#include "ipfModelerProcessChildSpikePointCheck.h"
#include "../../ui/ipfModelerSpikePointCheckDialog.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"

#include <QProgressDialog>

ipfModelerProcessChildSpikePointCheck::ipfModelerProcessChildSpikePointCheck(QObject * parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerSpikePointCheckDialog();
}

ipfModelerProcessChildSpikePointCheck::~ipfModelerProcessChildSpikePointCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildSpikePointCheck::checkParameter()
{
	//if (threshold < 0)
	//{
	//	addErrList(QStringLiteral("检测参数不应小于0。"));
	//	return false;
	//}
	return true;
}

void ipfModelerProcessChildSpikePointCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		threshold = map["threshold"].toDouble();
		saveName = map["saveName"];
	}
}

QMap<QString, QString> ipfModelerProcessChildSpikePointCheck::getParameter()
{
	QMap<QString, QString> map;

	map["threshold"] = QString::number(threshold);
	map["saveName"] = saveName;
	return map;
}

void ipfModelerProcessChildSpikePointCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	threshold = map["threshold"].toDouble();
	saveName = map["saveName"];
}

void ipfModelerProcessChildSpikePointCheck::run()
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
		QString target = saveName + "\\" + rasterFileName;
		QString err = gdal.stdevp3x3Alg(var, target, threshold);

		if (err.isEmpty())
		{
			// 计算最大最小值
			ipfOGR ogr(target);
			if (!ogr.isOpen())
			{
				addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -2。"));
				continue;
			}

			QString wkt = ogr.getProjection();

			//if (cerr == CE_Warning)
			//{
			//	// 输出文件
			//	QString format = "tif";
			//	QString targetTo = saveName + "\\" + removeDelimiter(target) + '.' + format;
			//	QString err = gdal.formatConvert(target, targetTo, gdal.enumFormatToString(format), "NONE", "NO", "0");
			//	if (!err.isEmpty())
			//	{
			//		addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -1。"));
			//		continue;
			//	}
			//	outList << rasterFileName + QStringLiteral(": 检查到栅格数据中存在跳点，并在输出栅格中被标记为1。");

				// 是否输出矢量
				if (true)
				{
					// 分割栅格，提升栅格转矢量的效率
					QStringList clipRasers;
					if (!ogr.splitRaster(1024, clipRasers))
					{
						addErrList(rasterFileName + QStringLiteral(": 输出错误矢量失败，已跳过。"));
						continue;
					}

					// 创建矢量图层 ----->
					QString vectorFile = target.mid(0, target.size() - 3) + "shp";
					if (!ipfOGR::createrVectorlayer(vectorFile, QgsWkbTypes::Polygon, QgsFields(), wkt))
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
			//}
			//else if (cerr == CE_None)
			//	outList << rasterFileName + QStringLiteral(": 正确。");
			//else
			//	addErrList(rasterFileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -4。"));
		}
		else
			addErrList(var + ": " + err);
	}
	//QString outName = QStringLiteral("d:/检查jj.txt");
	//printErrToFile(outName, outList);
}

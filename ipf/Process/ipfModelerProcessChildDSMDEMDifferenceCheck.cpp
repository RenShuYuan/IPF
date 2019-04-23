#include "ipfModelerProcessChildDSMDEMDifferenceCheck.h"
#include "../ipfOgr.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerPrintErrRasterDialog.h"
#include "qgsrastercalculator.h"
#include "ipfFlowManage.h"

#include "QgsRasterLayer.h"

#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

ipfModelerProcessChildDSMDEMDifferenceCheck::ipfModelerProcessChildDSMDEMDifferenceCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	dialog = new ipfModelerPrintErrRasterDialog();
}

ipfModelerProcessChildDSMDEMDifferenceCheck::~ipfModelerProcessChildDSMDEMDifferenceCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildDSMDEMDifferenceCheck::checkParameter()
{
	clearErrList();

	if (!QDir(saveName).exists())
	{
		addErrList(QStringLiteral("无效的输出文件夹。"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildDSMDEMDifferenceCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		saveName = map["saveName"];
	}
}

QMap<QString, QString> ipfModelerProcessChildDSMDEMDifferenceCheck::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;
	return map;
}

void ipfModelerProcessChildDSMDEMDifferenceCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	saveName = map["saveName"];
}

void ipfModelerProcessChildDSMDEMDifferenceCheck::run()
{
	clearOutFiles();
	clearErrList();

	// 查找文件名包含DSM的数据
	QRegExp regexp("(DSM)");
	QStringList filesDSM;
	for (int i=0; i< filesIn().size(); ++i)
	{
		QString dsm = filesIn().at(i);
		if (QFileInfo(dsm).baseName().contains(regexp))
		{
			filesDSM << dsm;
		}
	}

	//进度条
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("匹配DEM数据..."), QStringLiteral("取消"), 0, filesDSM.size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("匹配DEM数据"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	QStringList outList;
	for (int i = 0; i < filesDSM.size(); ++i)
	{
		QString dsm = filesDSM.at(i);
		QString dem = QFileInfo(dsm).baseName();
		QString fileName = dem;
		dem = dem.replace("DSM", "DEM");

		// 匹配DEM数据
		int index = getFilesIndex(filesIn(), dem);
		if (index == -1)
		{
			outList << fileName + QStringLiteral(": 没有在数据列表中匹配到对应的DEM数据。");
			continue;
		}
		dem = filesIn().at(index);

		// 计算差值栅格
		QString raster;
		QString err = compareRastersDiff(dsm, dem, raster);
		if (!err.isEmpty())
			addErrList(fileName + ": " + err);
		else
		{
			if (raster.isEmpty())
			{
				outList << QStringLiteral("警告: ") + dsm + " " + dem
					+ QStringLiteral(", 该两套数据名称匹配，但并未提取到差值数据，请核实地理位置。");
			}

			// 检查是否接边
			ipfOGR ogr(raster);
			if (!ogr.isOpen())
			{
				addErrList(fileName + QStringLiteral(": 输出检查结果失败，请自行核查该数据 -1。"));
				continue;
			}

			CPLErr err = ogr.ComputeMinMax(IPF_PLUS);
			ogr.close();
			if (err == CE_None)
			{
				QFile::remove(raster);
				outList << fileName + QStringLiteral(": 差值正确。");
			}
			else if (err == CE_Warning)
				outList << fileName + QStringLiteral(": 差值错误，请结合错误文件核查。");
			else
				outList << fileName + QStringLiteral(": 差值计算失败。");
		}

		dialog.setValue(++prCount);
		QApplication::processEvents();
		if (dialog.wasCanceled())
			return;
	}

	QString savefileName = saveName + QStringLiteral("/DSMDEM差值检查.txt");
	printErrToFile(savefileName, outList);
}

QString ipfModelerProcessChildDSMDEMDifferenceCheck::compareRastersDiff(const QString & oneRaster, const QString & twoRaster, QString &raster)
{
	// 获得栅格名称
	QFileInfo oneInfo(oneRaster);
	QString oneFileName = oneInfo.baseName();
	QFileInfo twoInfo(twoRaster);
	QString twoFileName = twoInfo.baseName();

	// 打开栅格
	QgsRasterLayer* oneLayer = new QgsRasterLayer(oneRaster, oneFileName, "gdal");
	QgsRasterLayer* twoLayer = new QgsRasterLayer(twoRaster, twoFileName, QString("gdal"));
	if (!oneLayer->isValid())
		return oneFileName + QStringLiteral(": 栅格数据读取失败。");
	if (!twoLayer->isValid())
		return twoFileName + QStringLiteral(": 栅格数据读取失败。");

	// 获取波段数
	int oneBandSize = oneLayer->bandCount();
	int twoBandSize = twoLayer->bandCount();
	if (oneBandSize != 1 || twoBandSize != 1)
	{
		RELEASE(oneLayer);
		RELEASE(twoLayer);
		return oneFileName + "@" + twoFileName + QStringLiteral("波段数量不正确。");
	}

	// 获取栅格相交的范围、行列数
	QgsRectangle oneBox = oneLayer->extent();
	QgsRectangle twoBox = twoLayer->extent();
	QgsRectangle box = oneBox.intersect(&twoBox);

	double pixel = oneLayer->rasterUnitsPerPixelX();
	QString strR = QString::number(pixel, 'f', 11);
	pixel = strR.toDouble();

	int mNColumns = (box.xMaximum() - box.xMinimum()) / pixel;
	int mNRows = (box.yMaximum() - box.yMinimum()) / pixel;

	if (pixel < 0.1) mNColumns += 1;

	double xxx = box.xMinimum() + mNColumns * pixel;
	double yyy = box.yMinimum() + mNRows * pixel;

	QgsRectangle bbox(box.xMinimum(), box.yMinimum(), xxx, yyy);

	// 单波段处理
	QString outErr;

	// 构建QgsRasterCalculatorEntry
	QgsRasterCalculatorEntry oneEntry;
	oneEntry.raster = oneLayer;
	oneEntry.bandNumber = 1;
	oneEntry.ref = oneFileName + "@1";

	QgsRasterCalculatorEntry twoEntry;
	twoEntry.raster = twoLayer;
	twoEntry.bandNumber = 1;
	twoEntry.ref = twoFileName + "@1";

	QVector<QgsRasterCalculatorEntry> entries;
	entries << oneEntry << twoEntry;

	// 构建QgsRasterCalculator
	QString outFile = saveName + "/" + oneEntry.ref + "@" + twoEntry.ref + ".tif";
	QgsRasterCalculator rc(oneEntry.ref + " - " + twoEntry.ref, outFile
		, ipfGdalProgressTools::enumFormatToString("tif")
		, bbox, oneLayer->crs(), mNColumns, mNRows, entries);

	// 开始处理
	QProgressDialog p(QStringLiteral("计算"), QStringLiteral("取消"), 0, 0);
	p.setWindowModality(Qt::WindowModal);
	p.setMaximum(100.0);
	QgsFeedback feedback;
	connect(&feedback, &QgsFeedback::progressChanged, &p, &QProgressDialog::setValue);
	connect(&p, &QProgressDialog::canceled, &feedback, &QgsFeedback::cancel);
	QgsRasterCalculator::Result res = static_cast<QgsRasterCalculator::Result>(rc.processCalculation(&feedback));

	RELEASE(oneLayer);
	RELEASE(twoLayer);

	switch (res)
	{
	case QgsRasterCalculator::Success:
		raster = outFile;
		break;
	case QgsRasterCalculator::CreateOutputError:
		raster = QString();
		outErr = QStringLiteral("不能创建输出文件。");
		break;
	case QgsRasterCalculator::InputLayerError:
		raster = QString();
		outErr = QStringLiteral("无法读取输入栅格图层。");
		break;
	case QgsRasterCalculator::Canceled:
		raster = QString();
		outErr = QStringLiteral("已取消计算。");
		break;
	case QgsRasterCalculator::ParserError:
		raster = QString();
		outErr = QStringLiteral("无法解析栅格公式。");
		break;
	case QgsRasterCalculator::MemoryError:
		raster = QString();
		outErr = QStringLiteral("内存不足，无法运行。");
		break;
	}

	return outErr;
}

int ipfModelerProcessChildDSMDEMDifferenceCheck::getFilesIndex(const QStringList & lists, const QString & th)
{
	int index = -1;
	QRegExp strExp("(\\" + th + ".)");
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
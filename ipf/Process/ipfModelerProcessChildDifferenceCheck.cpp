#include "ipfModelerProcessChildDifferenceCheck.h"
#include "../ipfOgr.h"
#include "../ipfFractalmanagement.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerOutDialog.h"
#include "qgsrastercalculator.h"
#include "ipfFlowManage.h"

#include "QgsRasterLayer.h"

#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

ipfModelerProcessChildDifferenceCheck::ipfModelerProcessChildDifferenceCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	out = new ipfModelerOutDialog();
	map = out->getParameter();
}

ipfModelerProcessChildDifferenceCheck::~ipfModelerProcessChildDifferenceCheck()
{
	RELEASE(out);
}

bool ipfModelerProcessChildDifferenceCheck::checkParameter()
{
	bool isbl = true;
	clearErrList();

	if (ipfGdalProgressTools::enumFormatToString(format)
		== QStringLiteral("other"))
	{
		isbl = false;
		addErrList(QStringLiteral("错误或不支持的数据格式。"));
	}
	if (!QDir(outPath).exists())
	{
		isbl = false;
		addErrList(QStringLiteral("无效的输出文件夹。"));
	}

	return isbl;
}

void ipfModelerProcessChildDifferenceCheck::setParameter()
{
	if (out->exec())
	{
		map = out->getParameter();
		format = map["format"];
		outPath = map["outPath"];
		compress = map["compress"];
		isTfw = map["isTfw"];
		noData = map["noData"];
	}
}

QMap<QString, QString> ipfModelerProcessChildDifferenceCheck::getParameter()
{
	QMap<QString, QString> map;
	map["format"] = format;
	map["outPath"] = outPath;
	map["compress"] = compress;
	map["isTfw"] = isTfw;
	map["noData"] = noData;

	return map;
}

void ipfModelerProcessChildDifferenceCheck::setDialogParameter(QMap<QString, QString> map)
{
	out->setParameter(map);

	format = map["format"];
	outPath = map["outPath"];
	compress = map["compress"];
	isTfw = map["isTfw"];
	noData = map["noData"];
}

QString ipfModelerProcessChildDifferenceCheck::compareRastersDiff(
	  const QString & oneRaster
	, const QString & twoRaster
	, QStringList & returnRasters)
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
	if (oneBandSize != twoBandSize)
	{
		RELEASE(oneLayer);
		RELEASE(twoLayer);
		return oneFileName + ":" + twoFileName + QStringLiteral("波段数量不一致。");
	}

	// 获取栅格相交的范围、行列数
	QgsRectangle oneBox = oneLayer->extent();
	QgsRectangle twoBox = twoLayer->extent();
	QgsRectangle box = oneBox.intersect(&twoBox);

	double pixel = oneLayer->rasterUnitsPerPixelX();
	QString strR = QString::number(pixel, 'f', 11);
	pixel = strR.toDouble();

	int mNColumns = (box.xMaximum()-box.xMinimum()) / pixel;
	int mNRows = (box.yMaximum() - box.yMinimum()) / pixel;

	if (pixel < 0.1) mNColumns += 1;

	double xxx = box.xMinimum() + mNColumns * pixel;
	double yyy = box.yMinimum() + mNRows * pixel;

	QgsRectangle bbox(box.xMinimum(), box.yMinimum(), xxx, yyy);

	// 分波段处理
	QString outErr;
	for (int i = 0; i < oneBandSize; ++i)
	{
		// 构建QgsRasterCalculatorEntry
		QgsRasterCalculatorEntry oneEntry;
		oneEntry.raster = oneLayer;
		oneEntry.bandNumber = i+1;
		oneEntry.ref = oneFileName + "@" + QString::number(oneEntry.bandNumber);

		QgsRasterCalculatorEntry twoEntry;
		twoEntry.raster = twoLayer;
		twoEntry.bandNumber = i + 1;
		twoEntry.ref = twoFileName + "@" + QString::number(twoEntry.bandNumber);

		QVector<QgsRasterCalculatorEntry> entries;
		entries << oneEntry << twoEntry;

		// 构建QgsRasterCalculator
		QString outFile = outPath + "/" + oneEntry.ref + "@" + twoEntry.ref + "." + format;
		QgsRasterCalculator rc(oneEntry.ref + " - " + twoEntry.ref, outFile
			, ipfGdalProgressTools::enumFormatToString(format)
			, bbox, oneLayer->crs(), mNColumns, mNRows, entries);

		// 开始处理
		QProgressDialog p(QStringLiteral("计算"), QStringLiteral("取消"), 0, 0);
		p.setWindowModality(Qt::WindowModal);
		p.setMaximum(100.0);
		QgsFeedback feedback;
		connect(&feedback, &QgsFeedback::progressChanged, &p, &QProgressDialog::setValue);
		connect(&p, &QProgressDialog::canceled, &feedback, &QgsFeedback::cancel);
		QgsRasterCalculator::Result res = static_cast<QgsRasterCalculator::Result>(rc.processCalculation(&feedback));

		switch (res)
		{
		case QgsRasterCalculator::Success:
			returnRasters << outFile;
			break;
		case QgsRasterCalculator::CreateOutputError:
			outErr = QStringLiteral("不能创建输出文件。");
			break;
		case QgsRasterCalculator::InputLayerError:
			outErr = QStringLiteral("无法读取输入栅格图层。");
			break;
		case QgsRasterCalculator::Canceled:
			outErr = QStringLiteral("已取消计算。");
			break;
		case QgsRasterCalculator::ParserError:
			outErr = QStringLiteral("无法解析栅格公式。");
			break;
		case QgsRasterCalculator::MemoryError:
			outErr = QStringLiteral("内存不足，无法运行。");
			break;
		}
	}

	RELEASE(oneLayer);
	RELEASE(twoLayer);

	return outErr;
}

bool ipfModelerProcessChildDifferenceCheck::chackRasterVaule0(const QString & file)
{
	ipfOGR org(file);
	if (!org.isOpen())
		return false;

	float *pDataBuffer=0;
	if (!org.readRasterIO(&pDataBuffer))
	{
		org.close();
		return false;
	}
		
	double nodata = org.getNodataValue(1);
	QList<int> list = org.getYXSize();
	int count = list.at(0) * list.at(1);

	org.close();

	for (int i = 0; i <count; ++i)
	{
		if (pDataBuffer[i] != 0 && pDataBuffer[i] != nodata)
		{
			delete[] pDataBuffer; pDataBuffer = 0;
			return false;
		}
	}

	delete[] pDataBuffer; pDataBuffer = 0;
	return true;
}

int ipfModelerProcessChildDifferenceCheck::getFilesIndex(const QStringList & lists, const QString & th)
{
	int index = -1;
	QRegExp strExp("(" + th + ")(DSMU.|DOMU.|DEMU.|U.)");
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

void ipfModelerProcessChildDifferenceCheck::run()
{
	clearOutFiles();
	clearErrList();

	QStringList files;
	ipfFractalManagement frac;
	foreach(QString var, filesIn())
	{
		// 获得文件名称
		QFileInfo info(var);
		QString fileName = info.baseName();

		// 提取其中的标准图号
		fileName = fileName.mid(0, 11);

		// 检查是否为标准分幅图号
		if (!frac.effectiveness(fileName))
			addErrList(fileName + QStringLiteral(": 不是有效的标准分幅图号。"));
		else
			files << var;
	}

	//进度条
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("匹配接边图幅..."), QStringLiteral("取消"), 0, files.size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("匹配接边图幅"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	QStringList outList;
	for (int i = 0; i < files.size(); ++i)
	{
		QString var = files.at(i);

		// 从路径中获得图号
		QFileInfo info(var);
		QString fileName = info.baseName();

		// 去掉前后缀
		fileName = fileName.mid(0, 11);

		// 计算西、北、西北三幅图的图号
		QStringList jbList;
		QString northWestFrac = frac.getAdjacentFrac(fileName, 1);
		QString northFrac = frac.getAdjacentFrac(fileName, 2);
		QString westFrac = frac.getAdjacentFrac(fileName, 4);
		QString westSouthFrac = frac.getAdjacentFrac(fileName, 6);

		jbList << northWestFrac << northFrac << westFrac << westSouthFrac;

		for (int i = 0; i < jbList.size(); ++i)
		{
			QString file = jbList.at(i);
			int index = getFilesIndex(files, file);
			if (index != -1)
			{
				QStringList returnRasters;

				// 计算与西边图幅的差值栅格
				QString err = compareRastersDiff(var, files.at(index), returnRasters);
				if (!err.isEmpty())
					addErrList(var + ": " + err);
				else
				{
					if (returnRasters.isEmpty())
					{
						outList << QStringLiteral("警告: ") + fileName + " " + file
							+ QStringLiteral(", 该两幅图具有相邻关系，但并未提取到接边数据，疑似换带，请核实。");
					}

					// 检查是否接边
					for (int i=0; i<returnRasters.size(); ++i)
					{
						QFileInfo info(returnRasters.at(i));
						//if (chackRasterVaule0(returnRasters.at(i)))
						if (chackRasterVaule0(returnRasters.at(i)))
						{
							QFile::remove(returnRasters.at(i));
							outList << info.baseName() + QStringLiteral(": 接边正确。");
						}
						else
						{
							outList << info.baseName() + QStringLiteral(": 接边错误。");
						}
					}
				}
			}
		}

		dialog.setValue(++prCount);
		QApplication::processEvents();
		if (dialog.wasCanceled())
			return;
	}

	QString saveName = outPath + QStringLiteral("/接边检查.txt");
	QFile file(saveName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		addErrList(saveName + QStringLiteral("创建错误文件失败，已终止。"));
		return;
	}
	QTextStream out(&file);
	foreach(QString str, outList)
		out << str << endl;
	file.close();
}

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
		addErrList(QStringLiteral("�����֧�ֵ����ݸ�ʽ��"));
	}
	if (!QDir(outPath).exists())
	{
		isbl = false;
		addErrList(QStringLiteral("��Ч������ļ��С�"));
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
	// ���դ������
	QFileInfo oneInfo(oneRaster);
	QString oneFileName = oneInfo.baseName();
	QFileInfo twoInfo(twoRaster);
	QString twoFileName = twoInfo.baseName();

	// ��դ��
	QgsRasterLayer* oneLayer = new QgsRasterLayer(oneRaster, oneFileName, "gdal");
	QgsRasterLayer* twoLayer = new QgsRasterLayer(twoRaster, twoFileName, QString("gdal"));
	if (!oneLayer->isValid())
		return oneFileName + QStringLiteral(": դ�����ݶ�ȡʧ�ܡ�");
	if (!twoLayer->isValid())
		return twoFileName + QStringLiteral(": դ�����ݶ�ȡʧ�ܡ�");
	
	// ��ȡ������
	int oneBandSize = oneLayer->bandCount();
	int twoBandSize = twoLayer->bandCount();
	if (oneBandSize != twoBandSize)
	{
		RELEASE(oneLayer);
		RELEASE(twoLayer);
		return oneFileName + ":" + twoFileName + QStringLiteral("����������һ�¡�");
	}

	// ��ȡդ���ཻ�ķ�Χ��������
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

	// �ֲ��δ���
	QString outErr;
	for (int i = 0; i < oneBandSize; ++i)
	{
		// ����QgsRasterCalculatorEntry
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

		// ����QgsRasterCalculator
		QString outFile = outPath + "/" + oneEntry.ref + "@" + twoEntry.ref + "." + format;
		QgsRasterCalculator rc(oneEntry.ref + " - " + twoEntry.ref, outFile
			, ipfGdalProgressTools::enumFormatToString(format)
			, bbox, oneLayer->crs(), mNColumns, mNRows, entries);

		// ��ʼ����
		QProgressDialog p(QStringLiteral("����"), QStringLiteral("ȡ��"), 0, 0);
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
			outErr = QStringLiteral("���ܴ�������ļ���");
			break;
		case QgsRasterCalculator::InputLayerError:
			outErr = QStringLiteral("�޷���ȡ����դ��ͼ�㡣");
			break;
		case QgsRasterCalculator::Canceled:
			outErr = QStringLiteral("��ȡ�����㡣");
			break;
		case QgsRasterCalculator::ParserError:
			outErr = QStringLiteral("�޷�����դ��ʽ��");
			break;
		case QgsRasterCalculator::MemoryError:
			outErr = QStringLiteral("�ڴ治�㣬�޷����С�");
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
		// ����ļ�����
		QFileInfo info(var);
		QString fileName = info.baseName();

		// ��ȡ���еı�׼ͼ��
		fileName = fileName.mid(0, 11);

		// ����Ƿ�Ϊ��׼�ַ�ͼ��
		if (!frac.effectiveness(fileName))
			addErrList(fileName + QStringLiteral(": ������Ч�ı�׼�ַ�ͼ�š�"));
		else
			files << var;
	}

	//������
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("ƥ��ӱ�ͼ��..."), QStringLiteral("ȡ��"), 0, files.size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("ƥ��ӱ�ͼ��"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	QStringList outList;
	for (int i = 0; i < files.size(); ++i)
	{
		QString var = files.at(i);

		// ��·���л��ͼ��
		QFileInfo info(var);
		QString fileName = info.baseName();

		// ȥ��ǰ��׺
		fileName = fileName.mid(0, 11);

		// ��������������������ͼ��ͼ��
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

				// ����������ͼ���Ĳ�ֵդ��
				QString err = compareRastersDiff(var, files.at(index), returnRasters);
				if (!err.isEmpty())
					addErrList(var + ": " + err);
				else
				{
					if (returnRasters.isEmpty())
					{
						outList << QStringLiteral("����: ") + fileName + " " + file
							+ QStringLiteral(", ������ͼ�������ڹ�ϵ������δ��ȡ���ӱ����ݣ����ƻ��������ʵ��");
					}

					// ����Ƿ�ӱ�
					for (int i=0; i<returnRasters.size(); ++i)
					{
						QFileInfo info(returnRasters.at(i));
						//if (chackRasterVaule0(returnRasters.at(i)))
						if (chackRasterVaule0(returnRasters.at(i)))
						{
							QFile::remove(returnRasters.at(i));
							outList << info.baseName() + QStringLiteral(": �ӱ���ȷ��");
						}
						else
						{
							outList << info.baseName() + QStringLiteral(": �ӱߴ���");
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

	QString saveName = outPath + QStringLiteral("/�ӱ߼��.txt");
	QFile file(saveName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		addErrList(saveName + QStringLiteral("���������ļ�ʧ�ܣ�����ֹ��"));
		return;
	}
	QTextStream out(&file);
	foreach(QString str, outList)
		out << str << endl;
	file.close();
}

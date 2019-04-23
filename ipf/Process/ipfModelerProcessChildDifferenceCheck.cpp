#include "ipfModelerProcessChildDifferenceCheck.h"
#include "../ipfOgr.h"
#include "../ipfFractalmanagement.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerPrintErrRasterDialog.h"
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
	dialog = new ipfModelerPrintErrRasterDialog();
}

ipfModelerProcessChildDifferenceCheck::~ipfModelerProcessChildDifferenceCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildDifferenceCheck::checkParameter()
{
	clearErrList();

	if (!QDir(saveName).exists())
	{
		addErrList(QStringLiteral("��Ч������ļ��С�"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildDifferenceCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		saveName = map["saveName"];
	}
}

QMap<QString, QString> ipfModelerProcessChildDifferenceCheck::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;
	return map;
}

void ipfModelerProcessChildDifferenceCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	saveName = map["saveName"];
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
		QString outFile = saveName + "/" + oneEntry.ref + "@" + twoEntry.ref + ".tif";
		QgsRasterCalculator rc(oneEntry.ref + " - " + twoEntry.ref, outFile
			, ipfGdalProgressTools::enumFormatToString("tif")
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
						ipfOGR org(returnRasters.at(i), true);
						if (!org.isOpen())
						{
							outList << info.baseName() + QStringLiteral(": �޷���ȡ�����ļ��������³��ԡ�");
						}
						CPLErr err = org.ComputeMinMax(IPF_ZERO);
						org.close();
						if (err == CE_None)
						{
							QFile::remove(returnRasters.at(i));
							outList << info.baseName() + QStringLiteral(": �ӱ���ȷ��");
						}
						else if (err == CE_Warning)
						{
							outList << info.baseName() + QStringLiteral(": �ӱߴ���");
						}
						else
						{
							outList << info.baseName() + QStringLiteral(": ����ӱ߲�ֵ�쳣���������Ҫ�����ڽӱ������Ϊnodata����£���˲顣");
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

	QString savefileName = saveName + QStringLiteral("/�ӱ߼��.txt");
	printErrToFile(savefileName, outList);
}

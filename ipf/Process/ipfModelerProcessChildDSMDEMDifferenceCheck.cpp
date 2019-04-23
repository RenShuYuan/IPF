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
		addErrList(QStringLiteral("��Ч������ļ��С�"));
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

	// �����ļ�������DSM������
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

	//������
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("ƥ��DEM����..."), QStringLiteral("ȡ��"), 0, filesDSM.size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("ƥ��DEM����"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	QStringList outList;
	for (int i = 0; i < filesDSM.size(); ++i)
	{
		QString dsm = filesDSM.at(i);
		QString dem = QFileInfo(dsm).baseName();
		QString fileName = dem;
		dem = dem.replace("DSM", "DEM");

		// ƥ��DEM����
		int index = getFilesIndex(filesIn(), dem);
		if (index == -1)
		{
			outList << fileName + QStringLiteral(": û���������б���ƥ�䵽��Ӧ��DEM���ݡ�");
			continue;
		}
		dem = filesIn().at(index);

		// �����ֵդ��
		QString raster;
		QString err = compareRastersDiff(dsm, dem, raster);
		if (!err.isEmpty())
			addErrList(fileName + ": " + err);
		else
		{
			if (raster.isEmpty())
			{
				outList << QStringLiteral("����: ") + dsm + " " + dem
					+ QStringLiteral(", ��������������ƥ�䣬����δ��ȡ����ֵ���ݣ����ʵ����λ�á�");
			}

			// ����Ƿ�ӱ�
			ipfOGR ogr(raster);
			if (!ogr.isOpen())
			{
				addErrList(fileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -1��"));
				continue;
			}

			CPLErr err = ogr.ComputeMinMax(IPF_PLUS);
			ogr.close();
			if (err == CE_None)
			{
				QFile::remove(raster);
				outList << fileName + QStringLiteral(": ��ֵ��ȷ��");
			}
			else if (err == CE_Warning)
				outList << fileName + QStringLiteral(": ��ֵ�������ϴ����ļ��˲顣");
			else
				outList << fileName + QStringLiteral(": ��ֵ����ʧ�ܡ�");
		}

		dialog.setValue(++prCount);
		QApplication::processEvents();
		if (dialog.wasCanceled())
			return;
	}

	QString savefileName = saveName + QStringLiteral("/DSMDEM��ֵ���.txt");
	printErrToFile(savefileName, outList);
}

QString ipfModelerProcessChildDSMDEMDifferenceCheck::compareRastersDiff(const QString & oneRaster, const QString & twoRaster, QString &raster)
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
	if (oneBandSize != 1 || twoBandSize != 1)
	{
		RELEASE(oneLayer);
		RELEASE(twoLayer);
		return oneFileName + "@" + twoFileName + QStringLiteral("������������ȷ��");
	}

	// ��ȡդ���ཻ�ķ�Χ��������
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

	// �����δ���
	QString outErr;

	// ����QgsRasterCalculatorEntry
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

	RELEASE(oneLayer);
	RELEASE(twoLayer);

	switch (res)
	{
	case QgsRasterCalculator::Success:
		raster = outFile;
		break;
	case QgsRasterCalculator::CreateOutputError:
		raster = QString();
		outErr = QStringLiteral("���ܴ�������ļ���");
		break;
	case QgsRasterCalculator::InputLayerError:
		raster = QString();
		outErr = QStringLiteral("�޷���ȡ����դ��ͼ�㡣");
		break;
	case QgsRasterCalculator::Canceled:
		raster = QString();
		outErr = QStringLiteral("��ȡ�����㡣");
		break;
	case QgsRasterCalculator::ParserError:
		raster = QString();
		outErr = QStringLiteral("�޷�����դ��ʽ��");
		break;
	case QgsRasterCalculator::MemoryError:
		raster = QString();
		outErr = QStringLiteral("�ڴ治�㣬�޷����С�");
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
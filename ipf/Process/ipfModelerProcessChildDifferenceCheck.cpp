#include "ipfModelerProcessChildDifferenceCheck.h"
#include "../ipfOgr.h"
#include "../ipfFractalmanagement.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerDiffeCheckDialog.h"
#include "../../ui/ipfProgress.h"
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
	dialog = new ipfModelerDiffeCheckDialog();
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
		valueMax = map["valueMax"].toDouble();
	}
}

QMap<QString, QString> ipfModelerProcessChildDifferenceCheck::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;
	map["valueMax"] = QString::number(valueMax);
	return map;
}

void ipfModelerProcessChildDifferenceCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	saveName = map["saveName"];
	valueMax = map["valueMax"].toDouble();
}

QString ipfModelerProcessChildDifferenceCheck::compareRastersDiff(
	  const QString & oneRaster
	, const QString & twoRaster
	, QStringList & returnRasters)
{
	QString outErr;

	int mNColumns = 0;
	int mNRows = 0;

	// ���դ������
	QFileInfo oneInfo(oneRaster);
	QString oneFileName = oneInfo.baseName();
	QFileInfo twoInfo(twoRaster);
	QString twoFileName = twoInfo.baseName();

	// ��դ��
	QgsRasterLayer* oneLayer = new QgsRasterLayer(oneRaster, oneFileName, "gdal");
	QgsRasterLayer* twoLayer = new QgsRasterLayer(twoRaster, twoFileName, "gdal");
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
		return oneFileName + ":" + twoFileName + QStringLiteral("������������һ�¡�");
	}

	QgsCoordinateReferenceSystem oneCrs = oneLayer->crs();
	QgsCoordinateReferenceSystem twoCrs = twoLayer->crs();
	double nodata = twoLayer->dataProvider()->sourceNoDataValue(1);
	if (::isnan(nodata))
		nodata = 0;

	// ��ͬ������Ҫ��������
	if (oneCrs != twoCrs)
	{
		QgsCoordinateTransform ct(oneCrs, twoCrs);
		if (ct.isValid())
		{
			ipfGdalProgressTools gdal;
			QString one_srs = oneCrs.authid();
			QString two_srs = twoCrs.authid();
			QString target = ipfFlowManage::instance()->getTempVrtFile(twoRaster);
			QString err = gdal.transform(twoRaster, target, two_srs, one_srs, "bilinear", nodata);
			if (err.isEmpty())
			{
				RELEASE(twoLayer);
				twoLayer = new QgsRasterLayer(target, twoFileName, QString("gdal"));
				if (!twoLayer->isValid())
				{
					RELEASE(oneLayer);
					RELEASE(twoLayer);
					return twoRaster + ": ��̬ͶӰʧ�ܣ���������";
				}
				outErr = QStringLiteral("@�����ӱ�@");
			}
			else
			{
				RELEASE(oneLayer);
				RELEASE(twoLayer);
				return twoRaster + ": " + err;
			}
		}
		else
		{
			RELEASE(oneLayer);
			RELEASE(twoLayer);
			return oneFileName + ":" + twoFileName + QStringLiteral("��ͶӰ��һ�£����޷�����ת����ϵ�����ʵ��");
		}
	}

	// ��ȡդ���ཻ�ķ�Χ��������
	QgsRectangle oneBox = oneLayer->extent();
	QgsRectangle twoBox = twoLayer->extent();
	QgsRectangle box = oneBox.intersect(&twoBox);

	double pixel = oneLayer->rasterUnitsPerPixelX();
	QString strR = QString::number(pixel, 'f', 11);
	pixel = strR.toDouble();

	mNColumns = (box.xMaximum() - box.xMinimum()) / pixel;
	mNRows = (box.yMaximum() - box.yMinimum()) / pixel;

	if (pixel < 0.1) mNColumns += 1;

	double xxx = box.xMinimum() + mNColumns * pixel;
	double yyy = box.yMinimum() + mNRows * pixel;

	QgsRectangle bbox(box.xMinimum(), box.yMinimum(), xxx, yyy);

	// �ֲ��δ���
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
		QgsCoordinateTransformContext context;
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

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, files.size());
	proDialog.setRangeChild(0, 4);
	proDialog.show();

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
			proDialog.setValue(i + 1);
			if (proDialog.wasCanceled())
				return;

			QString file = jbList.at(i);
			int index = getFilesIndex(files, file);
			if (index != -1)
			{
				QStringList returnRasters;

				// ����������ͼ���Ĳ�ֵդ��
				QString err = compareRastersDiff(var, files.at(index), returnRasters);
				if (!err.isEmpty() && err != QStringLiteral("@�����ӱ�@"))
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
						ipfOGR org(returnRasters.at(i));
						if (!org.isOpen())
						{
							outList << info.baseName() + QStringLiteral(": �޷���ȡ�����ļ��������³��ԡ�");
							continue;
						}

						if (err == QStringLiteral("@�����ӱ�@"))
						{
							QgsPointXY point;
							CPLErr cErr = org.ComputeMinMax(IPF_NONE, point);

							if (cErr == CE_None)
							{
								if (abs(point.x()) > valueMax || abs(point.y()) > valueMax)
								{
									outList << info.baseName() + QStringLiteral(": �ӱߴ���");
								}
								else
								{
									QFile::remove(returnRasters.at(i));
									outList << info.baseName() + QStringLiteral(": �ӱ���ȷ��");
								}
							}
							else
							{
								QFile::remove(returnRasters.at(i));
								outList << info.baseName() + QStringLiteral(": ����ӱ߲�ֵ�쳣���������Ҫ�������ص������Ϊnodata����£���˲顣");
								continue;
							}
						}
						else
						{
							CPLErr cErr = org.ComputeMinMax(IPF_ZERO);

							if (cErr == CE_None)
							{
								QFile::remove(returnRasters.at(i));
								outList << info.baseName() + QStringLiteral(": �ӱ���ȷ��");
							}
							else if (cErr == CE_Warning)
								outList << info.baseName() + QStringLiteral(": �ӱߴ���");
							else
							{
								QFile::remove(returnRasters.at(i));
								outList << info.baseName() + QStringLiteral(": ����ӱ߲�ֵ�쳣���������Ҫ�����ڽӱ������Ϊnodata����£���˲顣");
							}
						}
					}
				}
			}
		}
	}

	QString savefileName = saveName + QStringLiteral("/�ӱ߼��.txt");
	printErrToFile(savefileName, outList);
}

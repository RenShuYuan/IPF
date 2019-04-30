#include "ipfModelerProcessChildCreateMetadata.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../gdal/ipfExcel.h"
#include "ipfapplication.h"
#include "ipfOgr.h"
#include "ipfProjectionTransformation.h"
#include "../ui/ipfModelerCreateMetadataDialog.h"
#include "../ipfFractalmanagement.h"

#include <QFile>
#include <QProgressDialog>

ipfModelerProcessChildCreateMetadata::ipfModelerProcessChildCreateMetadata(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerCreateMetadataDialog();
}

ipfModelerProcessChildCreateMetadata::~ipfModelerProcessChildCreateMetadata()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildCreateMetadata::checkParameter()
{
	bool isbl = true;

	QFileInfo info(sample);
	if (!info.exists())
	{
		isbl = false;
		addErrList(info.fileName() + QStringLiteral(": ��Ч��Ԫ���������ļ���"));
	}

	QDir dir(outPath);
	if (!dir.exists())
	{
		isbl = false;
		addErrList(QStringLiteral("��Ч������ļ��С�"));
	}

	return isbl;
}

void ipfModelerProcessChildCreateMetadata::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		metaDataType = map["metaDataType"];
		sample = map["sample"];
		outPath = map["outPath"];
	}
}

QMap<QString, QString> ipfModelerProcessChildCreateMetadata::getParameter()
{
	QMap<QString, QString> map;
	map["metaDataType"] = metaDataType;
	map["outPath"] = outPath;
	map["sample"] = sample;

	return map;
}

void ipfModelerProcessChildCreateMetadata::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	metaDataType = map["metaDataType"];
	outPath = map["outPath"];
	sample = map["sample"];
}

void ipfModelerProcessChildCreateMetadata::createZjMetaData(const QString & var)
{
	// �����ļ����жϸ������Ƿ���ȫɫ
	// ����ȫɫӰ���Զ�ȥ�Ҷ����
	QString muxFile;
	QFileInfo rInfo(var);
	if (rInfo.baseName().back() == 'P')
	{
		muxFile = var;
		muxFile.replace(var.size() - 5, 1, 'M');
		QFileInfo mInfo(muxFile);
		if (!mInfo.exists())
		{
			addErrList(var + QStringLiteral(": δ�ҵ���Ӧ�Ķ����Ӱ��"));
			return;
		}
	}
	else
		return;

	// �����������µ�Ԫ����
	QFileInfo info(var);
	QString fileName = info.fileName();
	fileName = fileName.remove(fileName.size() - 5, 5); // ȥ�����ƺ�ġ�P��
	QString target = outPath + "\\" + fileName + ".xls";
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": Ԫ���ݴ���ʧ�ܡ�"));
		return;
	}

	// ��excel
	ipfExcel excel;
	QString err = excel.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}

	// ��д����
	// 2. ��������С
	QString pSize = ipfApplication::dataAmount(var);
	QString mSize = ipfApplication::dataAmount(muxFile);
	excel.editCell("B6", QVariant(pSize + '/' + mSize));

	ipfOGR ogrP(var);
	if (ogrP.isOpen())
	{
		// 3. ������������
		QList<double> xyList = ogrP.getXYcenter();
		excel.editCell("B11", QVariant(xyList.at(3)));	// ����X
		excel.editCell("B12", QVariant(xyList.at(0)));	// ����Y
		excel.editCell("B13", QVariant(xyList.at(1)));	// ����X
		excel.editCell("B14", QVariant(xyList.at(0)));	// ����Y
		excel.editCell("B15", QVariant(xyList.at(1)));	// ����X
		excel.editCell("B16", QVariant(xyList.at(2)));	// ����Y
		excel.editCell("B17", QVariant(xyList.at(3)));	// ����X
		excel.editCell("B18", QVariant(xyList.at(2)));	// ����Y

		// 4. ���������߼�����
		QString strProjection = ogrP.getProjection();
		OGRSpatialReference oSRS;
		oSRS.SetFromUserInput(strProjection.toStdString().c_str());
		double d_centralMeridian = oSRS.GetProjParm(SRS_PP_CENTRAL_MERIDIAN);
		int zoneNo = ipfProjectionTransformation::getWgs84Bandwidth(d_centralMeridian);
		excel.editCell("B23", QVariant(d_centralMeridian));
		excel.editCell("B25", QVariant(zoneNo));
	}
	else
	{
		addErrList(var + QStringLiteral(": �޷���ȡ���漰Ӱ�������Ϣδ��д��"));
		excel.close();
	}

	// 5. ����ż���ȡʱ��
	QString orbitCode = rInfo.fileName().mid(3, 6);
	QString data = rInfo.fileName().mid(9, 8);
	excel.editCell("B30", QVariant(orbitCode));
	excel.editCell("B31", QVariant(data));
	excel.editCell("B36", QVariant(orbitCode));
	excel.editCell("B37", QVariant(data));

	excel.close();

/*
	/// Ԫ���ݰ��
	// �����������µ�Ԫ����
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString target = outPath + "\\" + fileName;
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": Ԫ���ݴ���ʧ�ܡ�"));
		return;
	}

	// ��Դexcel������ȡ����
	QStringList srcList;
	ipfExcel excel;
	QString err = excel.open(var);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}
	for (int i=0; i<54; ++i)
	{
		srcList << excel.getCell(QString("B%1").arg(i+1));
	}
	excel.close();

	// ��Ŀ��excel����д������
	ipfExcel excelNew;
	err = excelNew.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}
	for (int i = 0; i < srcList.size(); ++i)
	{
		excelNew.editCell(QString("B%1").arg(i + 1), QVariant(srcList.at(i)));
	}
	excelNew.close(); */
}

void ipfModelerProcessChildCreateMetadata::createDsmMetaData(const QString & var)
{
	// �����������µ�Ԫ����
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString baseName = info.baseName();
	fileName = fileName.mid(0, 11);
	QString target = outPath + "\\" + baseName + ".xls";
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": Ԫ���ݴ���ʧ�ܡ�"));
		return;
	}

	// ��excel
	ipfExcel excel;
	QString err = excel.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}

	// ��д����
	// 2. ͼ��
	excel.editCell("B6", QVariant(fileName));

	// 3. ��������С
	QString pSize = ipfApplication::dataAmount(var);
	excel.editCell("B7", QVariant(pSize));

	ipfOGR ogrP(var);
	if (ogrP.isOpen())
	{
		// 4. ����������
		QList<int> list = ogrP.getYXSize();
		excel.editCell("B10", QVariant(list.at(0)));
		excel.editCell("B11", QVariant(list.at(1)));

		// 5. ��ʼ����x, y����
		QgsRectangle rect = ogrP.getXY();
		double midSize = ogrP.getPixelSize() / 2;
		excel.editCell("B14", QVariant(rect.yMaximum() - midSize));
		excel.editCell("B15", QVariant(rect.xMinimum() + midSize));

		// 6. ���������߼�����
		QString strProjection = ogrP.getProjection();
		OGRSpatialReference oSRS;
		oSRS.SetFromUserInput(strProjection.toStdString().c_str());
		double d_centralMeridian = oSRS.GetProjParm(SRS_PP_CENTRAL_MERIDIAN);
		int zoneNo = ipfProjectionTransformation::getWgs84Bandwidth(d_centralMeridian);
		excel.editCell("B20", QVariant(d_centralMeridian));
		excel.editCell("B22", QVariant(zoneNo));
	}
	else
	{
		addErrList(var + QStringLiteral(": �޷���ȡ���漰DSM�����Ϣδ��д��"));
		excel.close();
	}

	excel.close();

/*	/// Ԫ���ݰ��
	// �����������µ�Ԫ����
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString target = outPath + "\\" + fileName;
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": Ԫ���ݴ���ʧ�ܡ�"));
		return;
	}

	// ��Դexcel������ȡ����
	QStringList srcList;
	ipfExcel excel;
	QString err = excel.open(var);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}
	for (int i = 0; i < 42; ++i)
	{
		srcList << excel.getCell(QString("B%1").arg(i + 1));
	}
	excel.close();

	// ��Ŀ��excel����д������
	ipfExcel excelNew;
	err = excelNew.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}
	for (int i = 0; i < srcList.size(); ++i)
	{
		excelNew.editCell(QString("B%1").arg(i + 1), QVariant(srcList.at(i)));
	}
	excelNew.close();
	*/
}

void ipfModelerProcessChildCreateMetadata::createDemMetaData(const QString & var)
{
	// �����������µ�Ԫ����
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString baseName = info.baseName();
	fileName = fileName.mid(0, 11);
	QString target = outPath + "\\" + baseName + ".xls";
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": Ԫ���ݴ���ʧ�ܡ�"));
		return;
	}

	// ��excel
	ipfExcel excel;
	QString err = excel.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}

	// ��д����
	// 2. ͼ��
	excel.editCell("B6", QVariant(fileName));

	// 3. ��������С
	QString pSize = ipfApplication::dataAmount(var);
	excel.editCell("B7", QVariant(pSize));

	ipfOGR ogrP(var);
	if (ogrP.isOpen())
	{
		// 4. ����������
		QList<int> list = ogrP.getYXSize();
		excel.editCell("B10", QVariant(list.at(0)));
		excel.editCell("B11", QVariant(list.at(1)));

		// 5. ��ʼ����x, y����
		QgsRectangle rect = ogrP.getXY();
		double midSize = ogrP.getPixelSize() / 2;
		excel.editCell("B14", QVariant(rect.yMaximum() - midSize));
		excel.editCell("B15", QVariant(rect.xMinimum() + midSize));

		// 6. ���������߼�����
		QString strProjection = ogrP.getProjection();
		OGRSpatialReference oSRS;
		oSRS.SetFromUserInput(strProjection.toStdString().c_str());
		double d_centralMeridian = oSRS.GetProjParm(SRS_PP_CENTRAL_MERIDIAN);
		int zoneNo = ipfProjectionTransformation::getWgs84Bandwidth(d_centralMeridian);
		excel.editCell("B20", QVariant(d_centralMeridian));
		excel.editCell("B22", QVariant(zoneNo));
	}
	else
	{
		addErrList(var + QStringLiteral(": �޷���ȡ���漰DSM�����Ϣδ��д��"));
		excel.close();
	}

	excel.close();

/*	/// Ԫ���ݰ��
	// �����������µ�Ԫ����
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString target = outPath + "\\" + fileName;
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": Ԫ���ݴ���ʧ�ܡ�"));
		return;
	}

	// ��Դexcel������ȡ����
	QStringList srcList;
	ipfExcel excel;
	QString err = excel.open(var);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}
	for (int i = 0; i < 42; ++i)
	{
		srcList << excel.getCell(QString("B%1").arg(i + 1));
	}
	excel.close();

	// ��Ŀ��excel����д������
	ipfExcel excelNew;
	err = excelNew.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}
	for (int i = 0; i < srcList.size(); ++i)
	{
		excelNew.editCell(QString("B%1").arg(i + 1), QVariant(srcList.at(i)));
	}
	excelNew.close();
	*/
}

void ipfModelerProcessChildCreateMetadata::createDomMetaData(const QString & var)
{
	// �����������µ�Ԫ����
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString baseName = fileName.mid(0, 14);
	fileName = fileName.mid(0, 11);
	QString target = outPath + "\\" + baseName + ".xls";
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": Ԫ���ݴ���ʧ�ܡ�"));
		return;
	}

	// ��excel
	ipfExcel excel;
	QString err = excel.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}

	// ��д����
	// 1. ͼ��
	excel.editCell("B6", QVariant(fileName));

	// 2. ��������С
	QString pSize = ipfApplication::dataAmount(var);
	excel.editCell("B7", QVariant(pSize));

	ipfOGR ogr(var);
	if (ogr.isOpen())
	{
		// 3. ����ֱ���
		double R = 0.0;
		R = ogr.getPixelSize();
		QString pixelSize = QString::number(R, 'f', 1);
		excel.editCell("B9", QVariant(pixelSize));

		// 3. ����λ��
		QString pixelByte = QString::number(ogr.getBandSize() * 8);
		excel.editCell("B11", QVariant(pixelByte));

		// 4. ��������
		int blc = 50000;
		if (R == 16.0)
			blc = 250000;
		ipfFractalManagement frac(blc);
		QList<QgsPointXY> four = frac.dNToXy(fileName);
		
		excel.editCell("B13", QVariant(QString::number(four.at(0).y(), 'f', 2)));	// ����X
		excel.editCell("B14", QVariant(QString::number(four.at(0).x(), 'f', 2)));	// ����Y
		excel.editCell("B15", QVariant(QString::number(four.at(1).y(), 'f', 2)));	// ����X
		excel.editCell("B16", QVariant(QString::number(four.at(1).x(), 'f', 2)));	// ����Y
		excel.editCell("B17", QVariant(QString::number(four.at(2).y(), 'f', 2)));	// ����X
		excel.editCell("B18", QVariant(QString::number(four.at(2).x(), 'f', 2)));	// ����Y
		excel.editCell("B19", QVariant(QString::number(four.at(3).y(), 'f', 2)));	// ����X
		excel.editCell("B20", QVariant(QString::number(four.at(3).x(), 'f', 2)));	// ����Y

		// 5. ���������߼�����
		QString strProjection = ogr.getProjection();
		OGRSpatialReference oSRS;
		oSRS.SetFromUserInput(strProjection.toStdString().c_str());
		double d_centralMeridian = oSRS.GetProjParm(SRS_PP_CENTRAL_MERIDIAN);
		int zoneNo = ipfProjectionTransformation::getWgs84Bandwidth(d_centralMeridian);
		excel.editCell("B25", QVariant(d_centralMeridian));
		excel.editCell("B27", QVariant(zoneNo));
	}
	else
	{
		addErrList(var + QStringLiteral(": �޷���ȡ���漰DOM�����Ϣδ��д��"));
	}

	excel.close();

/*	/// Ԫ���ݰ��
	// �����������µ�Ԫ����
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString target = outPath + "\\" + fileName;
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": Ԫ���ݴ���ʧ�ܡ�"));
		return;
	}

	// ��Դexcel������ȡ����
	QStringList srcList;
	ipfExcel excel;
	QString err = excel.open(var);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}
	for (int i = 0; i < 73; ++i)
	{
		srcList << excel.getCell(QString("B%1").arg(i + 1));
	}
	excel.close();

	// ��Ŀ��excel����д������
	ipfExcel excelNew;
	err = excelNew.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}
	for (int i = 0; i < srcList.size(); ++i)
	{
		excelNew.editCell(QString("B%1").arg(i + 1), QVariant(srcList.at(i)));
	}
	excelNew.close();
	*/
}

void ipfModelerProcessChildCreateMetadata::run()
{
	clearOutFiles();
	clearErrList();

	QStringList files = filesIn();

	//������
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("����Ԫ����..."), QStringLiteral("ȡ��"), 0, files.size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("����Ԫ����"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	//foreach(QString var, filesIn())
	//{
	//	if (metaDataType == QStringLiteral("����Ԫ����"))
	//		createZjMetaData(var);
	//	else if (metaDataType == QStringLiteral("DSMԪ����"))
	//		createDsmMetaData(var);
	//	else if (metaDataType == QStringLiteral("DEMԪ����"))
	//		createDemMetaData(var);
	//	else if (metaDataType == QStringLiteral("DOMԪ����"))
	//		createDomMetaData(var);

	//	dialog.setValue(++prCount);
	//	QApplication::processEvents();
	//	if (dialog.wasCanceled())
	//		return;
	//}

			// ���ʹ��OpenMP������
#pragma omp parallel for
	for (int i=0; i< files.size(); ++i)
	{
		QString var = files.at(i);

		if (metaDataType == QStringLiteral("����Ԫ����"))
			createZjMetaData(var);
		else if (metaDataType == QStringLiteral("DSMԪ����"))
			createDsmMetaData(var);
		else if (metaDataType == QStringLiteral("DEMԪ����"))
			createDemMetaData(var);
		else if (metaDataType == QStringLiteral("DOMԪ����"))
			createDomMetaData(var);

#pragma omp critical
		{
			if (++prCount < files.size())
			{
				dialog.setValue(prCount);
				QApplication::processEvents();
			}
		}
	}
}

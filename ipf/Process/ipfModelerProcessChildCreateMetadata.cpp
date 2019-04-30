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
		addErrList(info.fileName() + QStringLiteral(": 无效的元数据样本文件。"));
	}

	QDir dir(outPath);
	if (!dir.exists())
	{
		isbl = false;
		addErrList(QStringLiteral("无效的输出文件夹。"));
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
	// 根据文件名判断该数据是否是全色
	// 利用全色影像自动去找多光谱
	QString muxFile;
	QFileInfo rInfo(var);
	if (rInfo.baseName().back() == 'P')
	{
		muxFile = var;
		muxFile.replace(var.size() - 5, 1, 'M');
		QFileInfo mInfo(muxFile);
		if (!mInfo.exists())
		{
			addErrList(var + QStringLiteral(": 未找到对应的多光谱影像。"));
			return;
		}
	}
	else
		return;

	// 从样本创建新的元数据
	QFileInfo info(var);
	QString fileName = info.fileName();
	fileName = fileName.remove(fileName.size() - 5, 5); // 去掉名称后的“P”
	QString target = outPath + "\\" + fileName + ".xls";
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": 元数据创建失败。"));
		return;
	}

	// 打开excel
	ipfExcel excel;
	QString err = excel.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}

	// 填写内容
	// 2. 数据量大小
	QString pSize = ipfApplication::dataAmount(var);
	QString mSize = ipfApplication::dataAmount(muxFile);
	excel.editCell("B6", QVariant(pSize + '/' + mSize));

	ipfOGR ogrP(var);
	if (ogrP.isOpen())
	{
		// 3. 四至中心坐标
		QList<double> xyList = ogrP.getXYcenter();
		excel.editCell("B11", QVariant(xyList.at(3)));	// 西南X
		excel.editCell("B12", QVariant(xyList.at(0)));	// 西南Y
		excel.editCell("B13", QVariant(xyList.at(1)));	// 西北X
		excel.editCell("B14", QVariant(xyList.at(0)));	// 西北Y
		excel.editCell("B15", QVariant(xyList.at(1)));	// 东北X
		excel.editCell("B16", QVariant(xyList.at(2)));	// 东北Y
		excel.editCell("B17", QVariant(xyList.at(3)));	// 东南X
		excel.editCell("B18", QVariant(xyList.at(2)));	// 东南Y

		// 4. 中央子午线及带号
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
		addErrList(var + QStringLiteral(": 无法读取，涉及影像相关信息未填写。"));
		excel.close();
	}

	// 5. 轨道号及获取时间
	QString orbitCode = rInfo.fileName().mid(3, 6);
	QString data = rInfo.fileName().mid(9, 8);
	excel.editCell("B30", QVariant(orbitCode));
	excel.editCell("B31", QVariant(data));
	excel.editCell("B36", QVariant(orbitCode));
	excel.editCell("B37", QVariant(data));

	excel.close();

/*
	/// 元数据搬家
	// 从样本创建新的元数据
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString target = outPath + "\\" + fileName;
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": 元数据创建失败。"));
		return;
	}

	// 打开源excel，并读取数据
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

	// 打开目标excel，并写入数据
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
	// 从样本创建新的元数据
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString baseName = info.baseName();
	fileName = fileName.mid(0, 11);
	QString target = outPath + "\\" + baseName + ".xls";
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": 元数据创建失败。"));
		return;
	}

	// 打开excel
	ipfExcel excel;
	QString err = excel.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}

	// 填写内容
	// 2. 图号
	excel.editCell("B6", QVariant(fileName));

	// 3. 数据量大小
	QString pSize = ipfApplication::dataAmount(var);
	excel.editCell("B7", QVariant(pSize));

	ipfOGR ogrP(var);
	if (ogrP.isOpen())
	{
		// 4. 格网行列数
		QList<int> list = ogrP.getYXSize();
		excel.editCell("B10", QVariant(list.at(0)));
		excel.editCell("B11", QVariant(list.at(1)));

		// 5. 起始格网x, y坐标
		QgsRectangle rect = ogrP.getXY();
		double midSize = ogrP.getPixelSize() / 2;
		excel.editCell("B14", QVariant(rect.yMaximum() - midSize));
		excel.editCell("B15", QVariant(rect.xMinimum() + midSize));

		// 6. 中央子午线及带号
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
		addErrList(var + QStringLiteral(": 无法读取，涉及DSM相关信息未填写。"));
		excel.close();
	}

	excel.close();

/*	/// 元数据搬家
	// 从样本创建新的元数据
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString target = outPath + "\\" + fileName;
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": 元数据创建失败。"));
		return;
	}

	// 打开源excel，并读取数据
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

	// 打开目标excel，并写入数据
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
	// 从样本创建新的元数据
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString baseName = info.baseName();
	fileName = fileName.mid(0, 11);
	QString target = outPath + "\\" + baseName + ".xls";
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": 元数据创建失败。"));
		return;
	}

	// 打开excel
	ipfExcel excel;
	QString err = excel.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}

	// 填写内容
	// 2. 图号
	excel.editCell("B6", QVariant(fileName));

	// 3. 数据量大小
	QString pSize = ipfApplication::dataAmount(var);
	excel.editCell("B7", QVariant(pSize));

	ipfOGR ogrP(var);
	if (ogrP.isOpen())
	{
		// 4. 格网行列数
		QList<int> list = ogrP.getYXSize();
		excel.editCell("B10", QVariant(list.at(0)));
		excel.editCell("B11", QVariant(list.at(1)));

		// 5. 起始格网x, y坐标
		QgsRectangle rect = ogrP.getXY();
		double midSize = ogrP.getPixelSize() / 2;
		excel.editCell("B14", QVariant(rect.yMaximum() - midSize));
		excel.editCell("B15", QVariant(rect.xMinimum() + midSize));

		// 6. 中央子午线及带号
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
		addErrList(var + QStringLiteral(": 无法读取，涉及DSM相关信息未填写。"));
		excel.close();
	}

	excel.close();

/*	/// 元数据搬家
	// 从样本创建新的元数据
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString target = outPath + "\\" + fileName;
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": 元数据创建失败。"));
		return;
	}

	// 打开源excel，并读取数据
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

	// 打开目标excel，并写入数据
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
	// 从样本创建新的元数据
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString baseName = fileName.mid(0, 14);
	fileName = fileName.mid(0, 11);
	QString target = outPath + "\\" + baseName + ".xls";
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": 元数据创建失败。"));
		return;
	}

	// 打开excel
	ipfExcel excel;
	QString err = excel.open(target);
	if (!err.isEmpty())
	{
		addErrList(target + ": " + err);
		return;
	}

	// 填写内容
	// 1. 图号
	excel.editCell("B6", QVariant(fileName));

	// 2. 数据量大小
	QString pSize = ipfApplication::dataAmount(var);
	excel.editCell("B7", QVariant(pSize));

	ipfOGR ogr(var);
	if (ogr.isOpen())
	{
		// 3. 地面分辨率
		double R = 0.0;
		R = ogr.getPixelSize();
		QString pixelSize = QString::number(R, 'f', 1);
		excel.editCell("B9", QVariant(pixelSize));

		// 3. 像素位数
		QString pixelByte = QString::number(ogr.getBandSize() * 8);
		excel.editCell("B11", QVariant(pixelByte));

		// 4. 四至坐标
		int blc = 50000;
		if (R == 16.0)
			blc = 250000;
		ipfFractalManagement frac(blc);
		QList<QgsPointXY> four = frac.dNToXy(fileName);
		
		excel.editCell("B13", QVariant(QString::number(four.at(0).y(), 'f', 2)));	// 西南X
		excel.editCell("B14", QVariant(QString::number(four.at(0).x(), 'f', 2)));	// 西南Y
		excel.editCell("B15", QVariant(QString::number(four.at(1).y(), 'f', 2)));	// 西北X
		excel.editCell("B16", QVariant(QString::number(four.at(1).x(), 'f', 2)));	// 西北Y
		excel.editCell("B17", QVariant(QString::number(four.at(2).y(), 'f', 2)));	// 东北X
		excel.editCell("B18", QVariant(QString::number(four.at(2).x(), 'f', 2)));	// 东北Y
		excel.editCell("B19", QVariant(QString::number(four.at(3).y(), 'f', 2)));	// 东南X
		excel.editCell("B20", QVariant(QString::number(four.at(3).x(), 'f', 2)));	// 东南Y

		// 5. 中央子午线及带号
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
		addErrList(var + QStringLiteral(": 无法读取，涉及DOM相关信息未填写。"));
	}

	excel.close();

/*	/// 元数据搬家
	// 从样本创建新的元数据
	QFileInfo info(var);
	QString fileName = info.fileName();
	QString target = outPath + "\\" + fileName;
	if (!QFile::copy(sample, target))
	{
		addErrList(target + QStringLiteral(": 元数据创建失败。"));
		return;
	}

	// 打开源excel，并读取数据
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

	// 打开目标excel，并写入数据
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

	//进度条
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("制作元数据..."), QStringLiteral("取消"), 0, files.size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("制作元数据"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	//foreach(QString var, filesIn())
	//{
	//	if (metaDataType == QStringLiteral("整景元数据"))
	//		createZjMetaData(var);
	//	else if (metaDataType == QStringLiteral("DSM元数据"))
	//		createDsmMetaData(var);
	//	else if (metaDataType == QStringLiteral("DEM元数据"))
	//		createDemMetaData(var);
	//	else if (metaDataType == QStringLiteral("DOM元数据"))
	//		createDomMetaData(var);

	//	dialog.setValue(++prCount);
	//	QApplication::processEvents();
	//	if (dialog.wasCanceled())
	//		return;
	//}

			// 这句使用OpenMP来加速
#pragma omp parallel for
	for (int i=0; i< files.size(); ++i)
	{
		QString var = files.at(i);

		if (metaDataType == QStringLiteral("整景元数据"))
			createZjMetaData(var);
		else if (metaDataType == QStringLiteral("DSM元数据"))
			createDsmMetaData(var);
		else if (metaDataType == QStringLiteral("DEM元数据"))
			createDemMetaData(var);
		else if (metaDataType == QStringLiteral("DOM元数据"))
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

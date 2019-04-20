﻿#include "ipfOgr.h"
#include "gdal\ipfgdalprogresstools.h"

#include <QByteArray>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>


#include "QgsCoordinateReferenceSystem.h"
#include "QgsVectorFileWriter.h"

ipfOGR::ipfOGR() : poDataset(nullptr)
{
}

ipfOGR::ipfOGR(const QString &fileName)
	: poDataset(nullptr)
{
    //已只读方式打开影像文件
	poDataset = (GDALDataset*)GDALOpenEx(fileName.toStdString().c_str(), GDAL_OF_RASTER, NULL, NULL, NULL);
}

ipfOGR::~ipfOGR()
{
    if (poDataset)
	{
		GDALClose((GDALDatasetH)poDataset);
		poDataset = nullptr;
	}
}

bool ipfOGR::open(const QString &fileName)
{
    //已只读方式打开影像文件
	poDataset = (GDALDataset*)GDALOpenEx(fileName.toStdString().c_str(), GDAL_OF_RASTER, NULL, NULL, NULL);
    if (poDataset)
        return true;
    else
        return false;
}

void ipfOGR::close()
{
	if (poDataset)
	{
		GDALClose((GDALDatasetH)poDataset);
		poDataset = nullptr;
	}	
}

bool ipfOGR::isOpen()
{
    if (poDataset)
        return true;
    else
        return false;
}

QList<double> ipfOGR::getXY()
{
	QList<double> xyList;
    if (poDataset==NULL)
        return xyList;

    //获得影像像素大小
    GDALRasterBand *poBand_1 = poDataset->GetRasterBand(1);
    int xSize = poBand_1->GetXSize();
    int ySize = poBand_1->GetYSize();

    double pro[6];
    poDataset->GetGeoTransform(pro);

    xyList << pro[0] << pro[3] << pro[0] + pro[1] * xSize << pro[3] + pro[5] * ySize;

    return xyList;
}

QList<double> ipfOGR::getXYcenter()
{
	QList<double> xyList;
	if (poDataset == NULL)
		return xyList;

	//获得影像像素大小
	GDALRasterBand *poBand_1 = poDataset->GetRasterBand(1);
	int xSize = poBand_1->GetXSize();
	int ySize = poBand_1->GetYSize();

	double pro[6];
	poDataset->GetGeoTransform(pro);
	double midPixelSize = pro[1] / 2;

	xyList
		<< pro[0] + midPixelSize
		<< pro[3] - midPixelSize
		<< pro[0] + pro[1] * xSize - midPixelSize
		<< pro[3] + pro[5] * ySize + midPixelSize;

	return xyList;
}

bool ipfOGR::Projection2ImageRowCol(double dProjX, double dProjY, int & iCol, int & iRow)
{
	double adfGeoTransform[6];
	poDataset->GetGeoTransform(adfGeoTransform);
	try
	{
		double dTemp = adfGeoTransform[1] * adfGeoTransform[5] - adfGeoTransform[2] * adfGeoTransform[4];
		double dCol = 0.0, dRow = 0.0;
		dCol = (adfGeoTransform[5] * (dProjX - adfGeoTransform[0]) -
			adfGeoTransform[2] * (dProjY - adfGeoTransform[3])) / dTemp + 0.5;
		dRow = (adfGeoTransform[1] * (dProjY - adfGeoTransform[3]) -
			adfGeoTransform[4] * (dProjX - adfGeoTransform[0])) / dTemp + 0.5;

		iCol = static_cast<int>(dCol);
		iRow = static_cast<int>(dRow);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool ipfOGR::getPixelValue(int &iCol, int &iRow, double & value)
{
	GDALRasterBand *poBand = poDataset->GetRasterBand(1);
	double buffer[1] = {0};
	if (GDALRasterIO(poBand, GF_Read, iCol, iRow, 1, 1, buffer, 1, 1, GDT_CFloat64, 0, 0) == CE_None)
	{
		value = buffer[0];
		return true;
	}
	return false;
}

QList<int> ipfOGR::getYXSize()
{
	QList<int> list;
	list << poDataset->GetRasterYSize();
	list << poDataset->GetRasterXSize();
	return list;
}

const char* ipfOGR::getProjection()
{
    if (poDataset==NULL)
        return "";

	return poDataset->GetProjectionRef();
}

double ipfOGR::getPixelSize()
{
    double pro[6];
    poDataset->GetGeoTransform(pro);
	double sizeX = fabs(pro[1]);
    return sizeX;
}

int ipfOGR::getDataType()
{
    if (poDataset->GetRasterCount() == 0)
        return 0;
    GDALRasterBand *poBand_1 = poDataset->GetRasterBand(1);
    GDALDataType datatype= poBand_1->GetRasterDataType();

    if (datatype==::GDT_Byte)   return 8;
    else if (datatype==::GDT_UInt16 )   return 16;
    else if (datatype==::GDT_Int16 )   return 16;
    else if (datatype==::GDT_UInt32 )   return 32;
    else if (datatype==::GDT_Int32 )   return 32;
    else    return 0;
}

GDALDataType ipfOGR::getDataType_y()
{
	if (poDataset->GetRasterCount() == 0)
		return ::GDT_Unknown;
	GDALRasterBand *poBand_1 = poDataset->GetRasterBand(1);
	GDALDataType datatype = poBand_1->GetRasterDataType();
	return datatype;
}

GDALRasterBand * ipfOGR::getRasterBand(const int nBand)
{
	if (poDataset == NULL) return 0;
	return poDataset->GetRasterBand(nBand);
}

bool ipfOGR::readRasterIO(float ** pDataBuffer)
{
	if (poDataset != NULL)
	{
		if (poDataset->GetRasterCount() > 0)
		{
			GDALRasterBand* pBand = poDataset->GetRasterBand(1);
			QList<int> xyList = getYXSize();
			int xSize = xyList.at(1);
			int ySize = xyList.at(0);
			*pDataBuffer = new float[xSize*ySize];
			int err = pBand->RasterIO(GF_Read, 0, 0, xSize, ySize, *pDataBuffer, xSize, ySize, GDT_Float32, 0, 0);
			if (err == CE_None)
				return true;
			else
				return false;
		}
	}
	return false;
}

bool ipfOGR::readRasterIO(void * pDataBuffer, int qsX, int qsY, int xSize, int ySize, GDALDataType type)
{
	if (poDataset == NULL) return false;
	if (poDataset->GetRasterCount() == 0) return false;

	int nBands = poDataset->GetRasterCount();

	//定义读取输入图像波段顺序
	int *pBandMaps = new int[nBands];
	for (int b = 0; b < nBands; ++b)
		pBandMaps[b] = b + 1;

	int nPixelSpace = GDALGetDataTypeSizeBytes(type) * nBands;
	int nLineSpace = GDALGetDataTypeSizeBytes(type) * nBands * xSize;
	int nBandSpace = GDALGetDataTypeSizeBytes(type);

	int err = poDataset->RasterIO(GF_Read, qsX, qsY, xSize, ySize, pDataBuffer, xSize, ySize,
		type, nBands, pBandMaps, nPixelSpace, nLineSpace, nBandSpace);

	delete [] pBandMaps;

	if (err == CE_None)
		return true;
	else
		return false;
}

double ipfOGR::getNodataValue(const int iBand)
{
	GDALRasterBand* pBand = poDataset->GetRasterBand(iBand);
	if (pBand)
		return pBand->GetNoDataValue();

	return 0;
}

int ipfOGR::getBandSize()
{
	if (poDataset == NULL) return 0;
	return poDataset->GetRasterCount();
}

QString ipfOGR::getCompressionName()
{
	QString str;
	if (poDataset == NULL) return 0;
	char** info = poDataset->GetMetadata("Image_Structure");
	if (info != NULL && *info != NULL)
	{
		for (int i = 0; info[i] != NULL; i++)
		{
			QString str = info[i];
			QStringList list = str.split('=');
			if (!list.isEmpty() && list.at(0) == "COMPRESSION")
			{
				str = list.at(1);
				break;
			}
		}
	}
	return str;
}

GDALDataset* ipfOGR::createNewRaster(const QString &file, int nBands, GDALDataType type)
{
	if (poDataset == NULL) return 0;

	GDALDataset *poDataset_target = nullptr;
	GDALDriver *poDriver = nullptr;

	double adfGeoTransform[6];
	poDataset->GetGeoTransform(adfGeoTransform);
	int nXSize = poDataset->GetRasterXSize();
	int nYSize = poDataset->GetRasterYSize();

	if (nBands == -1)
		nBands = poDataset->GetRasterCount();

	if (type == GDT_Unknown)
		type = poDataset->GetRasterBand(1)->GetRasterDataType();

	QFileInfo info(file);
	poDriver = GetGDALDriverManager()->GetDriverByName(ipfGdalProgressTools::enumFormatToString(info.suffix()).toStdString().c_str());
	char **papszMetadata = poDriver->GetMetadata();

	poDataset_target = poDriver->Create(file.toStdString().c_str(), nXSize, nYSize, nBands, type, papszMetadata);
	if (poDataset_target == NULL)
		return nullptr;
	poDataset_target->SetGeoTransform(adfGeoTransform);
	poDataset_target->SetProjection(poDataset->GetProjectionRef());

	return poDataset_target;
}

bool ipfOGR::isCompression()
{
	if (poDataset == NULL) return false;

	char** info = poDataset->GetMetadata("Image_Structure");
	if (info != NULL && *info != NULL)
	{
		for (int i = 0; info[i] != NULL; i++)
		{
			QString str = info[i];
			QStringList list = str.split('=');
			if (!list.isEmpty() && list.at(0) == "COMPRESSION")
			{
				return true;
			}
		}
	}
	return false;
}

void * ipfOGR::newTypeSpace(const GDALDataType type, long size)
{
	void *buffData = nullptr;

	switch (type) {
	case GDT_Byte:
		buffData = new GByte[size]; break;
	case GDT_UInt16:
		buffData = new GUInt16[size]; break;
	case GDT_Int16:
		buffData = new GInt16[size]; break;
	case GDT_UInt32:
		buffData = new GUInt32[size]; break;
	case GDT_Int32:
		buffData = new GInt32[size]; break;
	case GDT_Float32:
		buffData = new float[size]; break;
	case GDT_Float64:
		buffData = new double[size]; break;
	default:
		buffData = new GByte[size]; break;
	}

	return buffData;
}

bool ipfOGR::createrShape(const QString & layerName, QgsWkbTypes::Type geometryType, const QgsFields &fields, const QString & wkt)
{
	// 参照坐标系
	QgsCoordinateReferenceSystem mCrs;
	mCrs.createFromWkt(wkt);

	QgsVectorFileWriter::WriterError error;
	QgsVectorFileWriter newShape(layerName, "system", fields, geometryType, mCrs, "ESRI Shapefile");
	error = newShape.hasError();
	if (error == QgsVectorFileWriter::NoError)
		return true;
	return false;
}

bool ipfOGR::rasterDelete(const QString &file)
{
	if (poDataset == NULL) return false;

	GDALDriver *pDriver = poDataset->GetDriver();
	if (pDriver == NULL)
		return false;

	if (pDriver->Delete(file.toStdString().c_str()) == CE_None)
		return true;
	else
		return false;
}

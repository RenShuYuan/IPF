#include "ipfOgr.h"
#include "gdal\ipfgdalprogresstools.h"
#include "Process\ipfFlowManage.h"

#include <QByteArray>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>


#include "QgsCoordinateReferenceSystem.h"
#include "qgsvectorlayer.h"
#include "QgsVectorFileWriter.h"

ipfOGR::ipfOGR() : poDataset(nullptr)
{
}

ipfOGR::ipfOGR(const QString &fileName, bool isUpdata)
	: poDataset(nullptr)
	, isInit(false)
{
    if (isUpdata)
		poDataset = (GDALDataset*)GDALOpenEx(fileName.toStdString().c_str(), GDAL_OF_RASTER | GDAL_OF_UPDATE, NULL, NULL, NULL);
	else
		poDataset = (GDALDataset*)GDALOpenEx(fileName.toStdString().c_str(), GDAL_OF_RASTER, NULL, NULL, NULL);

	if (poDataset)
		isInit = initGeoTransform();
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
	{
		isInit = initGeoTransform();
		return true;
	}        
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

QgsRectangle ipfOGR::getXY()
{
	QgsRectangle rect;
	if (!isInit)
		return rect;

    //获得影像像素大小
    int xSize = poDataset->GetRasterXSize();
    int ySize = poDataset->GetRasterYSize();

	// xMin << yMax << xMax << yMin;
	rect.setXMinimum(adfGeoTransform[0]);
	rect.setYMaximum(adfGeoTransform[3]);
	rect.setXMaximum(adfGeoTransform[0] + adfGeoTransform[1] * xSize);
	rect.setYMinimum(adfGeoTransform[3] + adfGeoTransform[5] * ySize);

    return rect;
}

QList<double> ipfOGR::getXYcenter()
{
	QList<double> xyList;
	if (!isInit)
		return xyList;

	//获得影像像素大小
	int xSize = poDataset->GetRasterXSize();
	int ySize = poDataset->GetRasterYSize();

	double midPixelSize = adfGeoTransform[1] / 2;

	xyList
		<< adfGeoTransform[0] + midPixelSize
		<< adfGeoTransform[3] - midPixelSize
		<< adfGeoTransform[0] + adfGeoTransform[1] * xSize - midPixelSize
		<< adfGeoTransform[3] + adfGeoTransform[5] * ySize + midPixelSize;

	return xyList;
}

bool ipfOGR::setGeoXy(const double x, const double y)
{
	if (poDataset->GetAccess() != GA_Update || !isInit)
		return false;

	double pro[6];
	poDataset->GetGeoTransform(pro);

	pro[0] = x;
	pro[3] = y;
	if (poDataset->SetGeoTransform(pro) == CE_None)
	{
		adfGeoTransform[0] = pro[0];
		adfGeoTransform[3] = pro[3];
		return true;
	}
	else
		return false;
}

bool ipfOGR::Projection2ImageRowCol(double dProjX, double dProjY, int & iCol, int & iRow)
{
	if (!isInit)
		return false;

	iCol = (int)floor(
		adfInvGeoTransform[0]
		+ adfInvGeoTransform[1] * dProjX
		+ adfInvGeoTransform[2] * dProjY);
	iRow = (int)floor(
		adfInvGeoTransform[3]
		+ adfInvGeoTransform[4] * dProjX
		+ adfInvGeoTransform[5] * dProjY);

	if (iCol < 0 || iRow < 0 || iCol > poDataset->GetRasterXSize() || iRow > poDataset->GetRasterYSize())
		return false;
	return true;
}

bool ipfOGR::getPixelValue(const int & band, const int &iCol, const int &iRow, double &value)
{
	if (poDataset->GetRasterCount() < band)
		return false;

	double tmp[1];
	GDALRasterBand *poBand = poDataset->GetRasterBand(band);
	if (GDALRasterIO(poBand, GF_Read, iCol, iRow, 1, 1, &tmp, 1, 1, GDT_CFloat64, 0, 0) == CE_None)
	{
		value = tmp[0];
		return true;
	}
	return false;
}

bool ipfOGR::getPixelValue(const int & band, const double & xGeo, const double & yGeo, double & value)
{
	int iCol = 0, iRow = 0;
	if (!Projection2ImageRowCol(xGeo, yGeo, iCol, iRow))
		return false;

	if (!getPixelValue(band, iCol, iRow, value))
		return false;
	return true;
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
	return poDataset->GetProjectionRef();
}

double ipfOGR::getPixelSize()
{
	if (!isInit)
		return 0;
    return fabs(adfGeoTransform[1]);
}

int ipfOGR::getDataType()
{
    if (poDataset->GetRasterCount() < 1)
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
	if (poDataset->GetRasterCount() < 1)
		return ::GDT_Unknown;
	GDALRasterBand *poBand_1 = poDataset->GetRasterBand(1);
	GDALDataType datatype = poBand_1->GetRasterDataType();
	return datatype;
}

GDALRasterBand * ipfOGR::getRasterBand(const int nBand)
{
	return poDataset->GetRasterBand(nBand);
}

bool ipfOGR::readRasterIO(float ** pDataBuffer, const int bandNo)
{
	if (poDataset->GetRasterCount() > 0 && poDataset->GetRasterCount() <= bandNo)
	{
		GDALRasterBand* pBand = poDataset->GetRasterBand(bandNo);
		int xSize = poDataset->GetRasterXSize();
		int ySize = poDataset->GetRasterYSize();
		*pDataBuffer = new float[xSize*ySize];
		int err = pBand->RasterIO(GF_Read, 0, 0, xSize, ySize, *pDataBuffer, xSize, ySize, GDT_Float32, 0, 0);
		if (err == CE_None)
			return true;
		else
			return false;
	}
	return false;
}

bool ipfOGR::readRasterIO(void * pDataBuffer, int qsX, int qsY, int xSize, int ySize, GDALDataType type)
{
	if (poDataset->GetRasterCount() < 1)
		return false;

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

	RELEASE_ARRAY(pBandMaps);

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
	return poDataset->GetRasterCount();
}

QString ipfOGR::getCompressionName()
{
	QString str;
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

GDALDataset* ipfOGR::createNewRaster(const QString &file, const QString nodata, int nBands, GDALDataType type)
{
	if (!isInit)
		return nullptr;

	GDALDataset *poDataset_target = nullptr;
	GDALDriver *poDriver = nullptr;

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
	
	double mNodata = 0.0;
	if (nodata == IPF_NODATA_NONE)
		mNodata = getNodataValue(1);
	else
		mNodata = nodata.toDouble();
	for (int i = 1; i<=nBands; ++i)
		poDataset_target->GetRasterBand(i)->SetNoDataValue(mNodata);

	return poDataset_target;
}

GDALDataset * ipfOGR::createParametersRaster(const QString & file)
{
	if (!isInit)
		return nullptr;

	GDALDataset *poDataset_target = nullptr;
	GDALDriver *poDriver = nullptr;

	int nXSize = 1;
	int nYSize = poDataset->GetRasterYSize();

	QFileInfo info(file);
	poDriver = GetGDALDriverManager()->GetDriverByName(ipfGdalProgressTools::enumFormatToString(info.suffix()).toStdString().c_str());
	char **papszMetadata = poDriver->GetMetadata();

	poDataset_target = poDriver->Create(file.toStdString().c_str(), nXSize, nYSize, 1, GDT_Byte, papszMetadata);
	if (poDataset_target == NULL)
		return nullptr;

	poDataset_target->SetGeoTransform(adfGeoTransform);
	poDataset_target->SetProjection(poDataset->GetProjectionRef());
	poDataset_target->GetRasterBand(1)->SetNoDataValue(getNodataValue(1));

	return poDataset_target;
}

bool ipfOGR::isCompression()
{
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

bool ipfOGR::splitShp(const QString & shpName, QStringList & shps)
{
	QgsVectorLayer *layer = new QgsVectorLayer(shpName, "vector");
	if (!layer || !layer->isValid())
		return false;

	QgsFeature f;
	QgsFeatureIterator it = layer->getFeatures();
	while (it.nextFeature(f))
	{
		if (f.isValid())
		{
			// 创建新shp
			QString new_shp = ipfFlowManage::instance()->getTempFormatFile(shpName, ".shp");
			if (!createrShape(new_shp, layer->wkbType(), layer->fields(), layer->crs().toWkt()))
			{
				RELEASE(layer);
				return false;
			}

			// 添加要素
			QgsVectorLayer *layerClip = new QgsVectorLayer(new_shp, "vector");
			if (!layerClip || !layerClip->isValid())
			{
				RELEASE(layer);
				return false;
			}
			layerClip->startEditing();
			layerClip->addFeature(f);
			layerClip->commitChanges();
			shps << new_shp;
			RELEASE(layerClip);
		}
	}
	RELEASE(layer);
	return true;

}

CPLErr ipfOGR::shpEnvelope(const QString & shpFile, QgsRectangle & rect)
{
	QgsVectorLayer *layer = new QgsVectorLayer(shpFile, "vector");
	if (!layer || !layer->isValid() || layer->featureCount() < 1)
		return CE_Failure;
	QgsFeature f = layer->getFeature(0);
	QgsGeometry geo = f.geometry();

	QgsRectangle rectRtr = getXY();
	QgsRectangle rectShp = layer->extent();
	RELEASE(layer);

	// 检查是否具有相交关系
	if (!geo.intersects(rectRtr))
		return CE_Warning;

	double tmp = 0.0;
	double R = getPixelSize();
	double midR = R / 2;

	// XMinimum
	if (rectRtr.xMinimum() < rectShp.xMinimum())
	{
		double JXMin = round(rectShp.xMinimum() / R) * R;
		if (JXMin >= rectShp.xMinimum())
			tmp = JXMin - R;
		else
			tmp = JXMin;
		tmp = tmp + midR;
		if (tmp < JXMin)
			rect.setXMinimum(tmp + midR);
		else
			rect.setXMinimum(tmp - midR);
	}
	else
	{
		rect.setXMinimum(rectRtr.xMinimum());
	}

	// XMaximum
	if (rectRtr.xMaximum() > rectShp.xMaximum())
	{
		double JXMax = round(rectShp.xMaximum() / R) * R;
		if (JXMax >= rectShp.xMaximum())
			tmp = JXMax - R;
		else
			tmp = JXMax;
		tmp = tmp + midR;
		if (tmp < JXMax)
			rect.setXMaximum(tmp + midR);
		else
			rect.setXMaximum(tmp - midR);
	}
	else
	{
		rect.setXMaximum(rectRtr.xMaximum());
	}

	// YMinimum
	if (rectRtr.yMinimum() < rectShp.yMinimum())
	{
		double JYMin = round(rectShp.yMinimum() / R) * R;
		if (JYMin >= rectShp.yMinimum())
			tmp = JYMin - R;
		else
			tmp = JYMin;
		tmp = tmp + midR;
		if (tmp < JYMin)
			rect.setYMinimum(tmp + midR);
		else
			rect.setYMinimum(tmp - midR);
	} 
	else
	{
		rect.setYMinimum(rectRtr.yMinimum());
	}

	// YMaximum
	if (rectRtr.yMaximum() > rectShp.yMaximum())
	{
		double JYMax = round(rectShp.yMaximum() / R) * R;
		if (JYMax >= rectShp.yMaximum())
			tmp = JYMax - R;
		else
			tmp = JYMax;
		tmp = tmp + midR;
		if (tmp < JYMax)
			rect.setYMaximum(tmp + midR);
		else
			rect.setYMaximum(tmp - midR);
	} 
	else
	{
		rect.setYMaximum(rectRtr.yMaximum());
	}

	return CE_None;
}
/*
CPLErr ipfOGR::shpEnvelope(const QgsGeometry & geometry, QgsRectangle & rect)
{
	if (geometry.isEmpty())
		return CE_Warning;

	QgsRectangle rectRtr = getXY();
	QgsRectangle rectShp = geometry.boundingBox();

	// 检查是否具有相交关系
	if (!rectRtr.intersects(rectShp))
		return CE_Warning;

	double tmp = 0.0;
	double R = getPixelSize();
	double midR = R / 2;

	// XMinimum
	if (rectRtr.xMinimum() < rectShp.xMinimum())
	{
		double JXMin = round(rectShp.xMinimum() / R) * R;
		if (JXMin >= rectShp.xMinimum())
			tmp = JXMin - R;
		else
			tmp = JXMin;
		tmp = tmp + midR;
		if (tmp < JXMin)
			rect.setXMinimum(tmp + midR);
		else
			rect.setXMinimum(tmp - midR);
	}
	else
	{
		rect.setXMinimum(rectRtr.xMinimum());
	}

	// XMaximum
	if (rectRtr.xMaximum() > rectShp.xMaximum())
	{
		double JXMax = round(rectShp.xMaximum() / R) * R;
		if (JXMax >= rectShp.xMaximum())
			tmp = JXMax - R;
		else
			tmp = JXMax;
		tmp = tmp + midR;
		if (tmp < JXMax)
			rect.setXMaximum(tmp + midR);
		else
			rect.setXMaximum(tmp - midR);
	}
	else
	{
		rect.setXMaximum(rectRtr.xMaximum());
	}

	// YMinimum
	if (rectRtr.yMinimum() < rectShp.yMinimum())
	{
		double JYMin = round(rectShp.yMinimum() / R) * R;
		if (JYMin >= rectShp.yMinimum())
			tmp = JYMin - R;
		else
			tmp = JYMin;
		tmp = tmp + midR;
		if (tmp < JYMin)
			rect.setYMinimum(tmp + midR);
		else
			rect.setYMinimum(tmp - midR);
	}
	else
	{
		rect.setYMinimum(rectRtr.yMinimum());
	}

	// YMaximum
	if (rectRtr.yMaximum() > rectShp.yMaximum())
	{
		double JYMax = round(rectShp.yMaximum() / R) * R;
		if (JYMax >= rectShp.yMaximum())
			tmp = JYMax - R;
		else
			tmp = JYMax;
		tmp = tmp + midR;
		if (tmp < JYMax)
			rect.setYMaximum(tmp + midR);
		else
			rect.setYMaximum(tmp - midR);
	}
	else
	{
		rect.setYMaximum(rectRtr.yMaximum());
	}

	return CE_None;
}
*/
bool ipfOGR::rasterDelete(const QString &file)
{
	GDALDriver *pDriver = poDataset->GetDriver();
	if (pDriver == NULL)
		return false;

	if (pDriver->Delete(file.toStdString().c_str()) == CE_None)
		return true;
	else
		return false;
}

CPLErr ipfOGR::ComputeMinMax(IPF_COMPUTE_TYPE type, QgsPointXY &point)
{
	CPLErr err = CE_None;

	if (getBandSize() != 1)
		return CE_Failure;

	double adfMinMax[2];
	err = getRasterBand(1)->ComputeRasterMinMax(FALSE, adfMinMax);
	if (err != CE_None)
		return err;

	if (type == IPF_ZERO)
	{
		if (adfMinMax[0] == 0 && adfMinMax[1] == 0)
			return CE_None;
		else
			return CE_Warning;
	}
	else if (type == IPF_EQUAL)
	{
		if (adfMinMax[0] == adfMinMax[1])
			return CE_None;
		else
			return CE_Warning;
	}
	else if (type == IPF_PLUS)
	{
		if (adfMinMax[0] >= 0 && adfMinMax[1] >= 0)
			return CE_None;
		else
			return CE_Warning;
	}
	else if (type == IPF_MINUS)
	{
		if (adfMinMax[0] <= 0 && adfMinMax[1] <= 0)
			return CE_None;
		else
			return CE_Warning;
	}
	else if (type == IPF_NONE)
	{
		point.setX(adfMinMax[0]);
		point.setY(adfMinMax[1]);
		return CE_None;
	}

	return err;
}

CPLErr ipfOGR::writeBlock(int band, int nXBlockOff, int nYBlockOff, double * pImage)
{
	if (poDataset->GetAccess() != GA_Update)
		return CE_Warning;
	if (band > getBandSize())
		return CE_Warning;

	CPLErr err = poDataset->GetRasterBand(band)->WriteBlock(nXBlockOff, nYBlockOff, pImage);
	return err;
}

bool ipfOGR::initGeoTransform()
{
	if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None
		&& GDALInvGeoTransform(adfGeoTransform, adfInvGeoTransform))
		return true;
	else
		return false;
}

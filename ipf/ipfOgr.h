#ifndef IPFOGR_H
#define IPFOGR_H

#include "head.h"
#include "qgsfields.h"

class ipfOGR
{
public:
	ipfOGR();
    ipfOGR(const QString &fileName);
    ~ipfOGR();

    bool open(const QString &fileName);
	void close();
    bool isOpen();

    // 返回指定影像的四至角点坐标
	QList<double> getXY();

	// 返回指定影像的四至中心点坐标
	QList<double> getXYcenter();

	bool Projection2ImageRowCol(double dProjX, double dProjY, int &iCol, int &iRow);

	// 返回坐标对应的像元值
	bool getPixelValue(int &iCol, int &iRow, double &value);

	//返回影像行列数
	QList<int> getYXSize();

    //返回影像的投影信息
    const char* getProjection();

    //返回影像像元大小
    double getPixelSize();

    //返回影像位深
    int getDataType();
	GDALDataType getDataType_y();

	// 返回对应波段对象
	GDALRasterBand * getRasterBand(const int nBand);

	bool readRasterIO(float **pDataBuffer);
	bool readRasterIO(void *pDataBuffer, int qsX, int qsY, int xSize, int ySize, GDALDataType type = GDT_Byte);

	// 获得NODATA值
	double getNodataValue(const int iBand);

	// 返回波段数
	int getBandSize();

	// 返回影像压缩信息
	QString getCompressionName();

	// 创建一个新栅格
	GDALDataset* createNewRaster(const QString &file, int nBands = -1, GDALDataType type = GDT_Unknown);

	// 检查影像是否被压缩
	bool isCompression();

	// 创建对应类型的数组
	static void* newTypeSpace(const GDALDataType type, long size);

	// 创建shp文件 文件名、文件类型、字段列表、投影编号
	static bool createrShape(const QString & layerName, QgsWkbTypes::Type geometryType, const QgsFields &fields, const QString & wkt);

	// 删除栅格数据
	bool rasterDelete(const QString &file);

private:
    GDALDataset* poDataset;
};

#endif // IPFOGR_H

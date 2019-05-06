#ifndef IPFOGR_H
#define IPFOGR_H

#include "head.h"
#include "qgsfields.h"
#include "qgswkbtypes.h"

class ipfOGR
{
public:
	ipfOGR();
    ipfOGR(const QString &fileName, bool isUpdata = false);
    ~ipfOGR();

    bool open(const QString &fileName);
	void close();
    bool isOpen();

    // 返回指定栅格的四至角点坐标
	QgsRectangle getXY();

	// 返回指定栅格的四至中心点坐标
	QList<double> getXYcenter();

	// 设置栅格的左上角坐标
	bool setGeoXy(const double x, const double y);

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

	bool readRasterIO(float **pDataBuffer, const int bandNo = 1);
	bool readRasterIO(void *pDataBuffer, int qsX, int qsY, int xSize, int ySize, GDALDataType type = GDT_Byte);

	// 获得NODATA值
	double getNodataValue(const int iBand);

	// 返回波段数
	int getBandSize();

	// 返回影像压缩信息
	QString getCompressionName();

	// 创建一个新栅格
	GDALDataset* createNewRaster(const QString &file, const QString nodata = IPF_NODATA_NONE, int nBands = -1, GDALDataType type = GDT_Unknown);

	// 检查影像是否被压缩
	bool isCompression();

	// 创建对应类型的数组
	static void* newTypeSpace(const GDALDataType type, long size);

	// 创建shp文件 文件名、文件类型、字段列表、投影编号
	static bool createrShape(const QString & layerName, QgsWkbTypes::Type geometryType, const QgsFields &fields, const QString & wkt);

	// 分割矢量面，保存到临时文件中
	static bool splitShp(const QString & shpName, QStringList & shps);

	// 计算矢量裁切后的四至范围
	CPLErr shpEnvelope(const QString & shpFile, QgsRectangle &rect);

	// 删除栅格数据
	bool rasterDelete(const QString &file);

	// 使用统计值方法检查栅格数据是否为0
	CPLErr ComputeMinMax(IPF_COMPUTE_TYPE type, QgsPointXY &point = QgsPointXY());
private:
    GDALDataset* poDataset;
};

#endif // IPFOGR_H

#ifndef IPFPROJECTIONTRANSFORMATION_H
#define IPFPROJECTIONTRANSFORMATION_H

#include "head.h"
#include "qgspointxy.h"
#include "qgscoordinatetransform.h"
#include "qgscoordinatereferencesystem.h"

class ipfProjectionTransformation : public QgsCoordinateTransform
{
public:
	enum errType
	{
		eOK,					// 正常
		eSourceCrsErr,			// 源参照坐标系无效
		eDestCrsErr,			// 目标参照坐标系无效
		eSouDestInvalid,		// 源或目标参照坐标系无效
		eTransformInvalid,		// 投影变换创建失败
		eNotSupportGCS,			// 不支持的地理坐标系
		eOther,					// 其他未知错误
	};

    ipfProjectionTransformation();
    ipfProjectionTransformation(const QgsCoordinateReferenceSystem& theSource,
                                         const QgsCoordinateReferenceSystem& theDest);

    // 投影变换
	ipfProjectionTransformation::errType prjTransform(QgsPointXY &p);

    // 检查是否是有效的地理、投影坐标系（CGCS2000、BJ1954、XIAN1980）
    static bool isValidGCS(const QString &authid);
    static bool isValidPCS(const int postgisSrid);

    // 是否对应投影坐标系
	static bool isSourceWGS1984Prj(const int postgisSrid);
    static bool isSourceCGCS2000Prj(const int postgisSrid);
    static bool isSourceXian1980Prj(const int postgisSrid);
    static bool isSourceBeijing1954Prj(const int postgisSrid);

    // 输入中央经线，返回PROJ4格式的高斯投影字符串
    static QString createProj4Cgcs2000Gcs( const double cm );
    static QString createProj4Xian1980Gcs( const double cm );
    static QString createProj4Beijing1954Gcs( const double cm );

    // 十进制转度
    static void sjzToDfm(QgsPointXY &p);
    static void sjzToDfm(double &num);

    // 度分秒转秒
    static void dfmToMM(QgsPointXY &p);

    // 输入经度(十进制), 返回带号(3度分带)
    static int getBandwidth3(double djd);

    // 输入经度(十进制), 返回中央经线(3度分带)
    static int getCentralmeridian3(double djd);
	
	// 输入经度(十进制), 返回wgs84带号
	static int getWgs84Bandwidth(double djd);
    
	// 创建目标投影参考坐标系
	ipfProjectionTransformation::errType createTargetCrs(const QgsPointXY point);

    // 返回地理坐标系，必须是支持的3种投影坐标系之一
    static QgsCoordinateReferenceSystem getGCS(const QgsCoordinateReferenceSystem& sourceCrs);

private:
	// 输入中央经线、带宽，返回对应分带的EPSG编号
    long getPCSauthid_CGCS2000(const QgsPointXY point, const int bw);
	long getPCSauthid_Wgs84Gcs(const QgsPointXY point);

private:
    QSettings mSettings;
};

#endif // IPFPROJECTIONTRANSFORMATION_H

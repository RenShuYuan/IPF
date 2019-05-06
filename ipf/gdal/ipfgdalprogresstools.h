#ifndef IPFGDALPROGRESSTOOLS_H
#define IPFGDALPROGRESSTOOLS_H

#include "head.h"
#include "qgspoint.h"
#include <QProgressBar>

class ipfProgress;

// 像元值位数保留
extern int IPF_DECIMAL;

// 像元值替换
extern double IPF_VALUE_OLD_1;
extern double IPF_VALUE_NEW_1;
extern double IPF_VALUE_OLD_2;
extern double IPF_VALUE_NEW_2;
extern bool   IPF_BANDS_NODIFFE;

// 分离无效值
extern double IPF_NODATA;
extern double IPF_BACKGROUND;

// 无效值检查
extern QList<double> IPF_BANSNODATA;
extern QList<double> IPF_INVALIDVALUE;
extern bool IPF_ISNEGATIVE;
extern bool IPF_ISNODATA;
extern bool IPF_BANDS_NODIFFE_CHECK;

// DSMDEM差值处理
extern double IPF_DSM_NODATA;
extern double IPF_DEM_NODATA;
extern QString IPF_DSMDEM_TYPE;
extern double IPF_THRESHOLD;
extern bool IPF_FILLNODATA;

// 范围赋值
extern double IPF_RANGE_VALUE;
extern double IPF_RANGE_NODATA;

#define PI 3.14159265

struct cs {
	ipfProgress * pdialog;
	QProgressBar *pProcess;
};

class ipfGdalProgressTools
{
public:
	enum errType
	{
		eOK,					// 正常
		eSourceOpenErr,			// 数据源无法打开
		eParameterErr,			// 参数错误, 无法进行下一步处理
		eParameterNull,			// 参数为null
		eSourceParameterNull,	// 数据源参数为null
		eDestParameterNull,		// 目标数据参数为null
		eSouDestDiff,			// 源数据和目标数据必须不同
		eOutputDatasetExists,	// 输出数据集已经存在
		eFormatErr,				// 错误或不支持的数据格式
		eSubdatasets,			// 包含子数据
		eTransformErr,			// 无法进行投影变换
		eRowColErr,				// 像元位置异常
		eUnChanged,				// 未改变
		eNotCreateDest,			// 无法创建目标文件
		eUserTerminated,		// 运行已被用户终止
		eOther,					// 其他未知错误
	};

	/**
	* @brief 3x3模板数据结构体
	* @根据不同的需求应重新设计
	*/
	typedef struct
	{
		/*! 3x3模板算子 */
		;
	} AlgData3x3;

    ipfGdalProgressTools();
    ~ipfGdalProgressTools();

	//static ipfGdalProgressTools *instance() { return smInstance; }

	QStringList getErrList() { return errList; };

	// 返回空闲物理内存大小, 字节
	qulonglong getFreePhysicalMemory();

	// 返回CPU核数
	int getNoOfProcessors();

	// 查询本地像元位置
	QString locationPixelInfo(const QString& source, const double x,const double y, int &iRow, int &iCol);
	QString locationPixelInfo(const QString& source, const QString& srs, const double x, const double y, int &iRow, int &iCol);

	// 格式转换
	QString formatConvert(const QString& source, const QString& target, const QString& format
						, const QString &compress, const QString &isTfw, const QString &noData);
	
	// 位深转换
	QString typeConvert(const QString& source, const QString& target, const QString& type);
	QString typeConvertNoScale(const QString& source, const QString& target, const QString& type);

	// 使用坐标范围裁切栅格
	QString proToClip_Translate(const QString& source, const QString& target, const QgsRectangle& rect);
	QString proToClip_Translate_src(const QString& source, const QString& target, const QList<int> list);
	QString proToClip_Warp(const QString& source, const QString& target, const QList<double> list);

	// 使用矢量数据裁切栅格
	QString AOIClip(const QString& source, const QString& target, const QString &vectorName);

	// 创建快视图（降样）， b:需要降低多少倍
	QString quickView(const QString& source, const QString& target, const int b);

	// 重采样
	QString resample(const QString& source, const QString& target, const double res, const QString &resampling_method);

	// 镶嵌
	QString mosaic_Warp(const QStringList &sourceList, const QString& target);
	QString mosaic_Buildvrt(const QStringList &sourceList, const QString& target);

	// 合成波段
	QString mergeBand(const QStringList &sourceList, const QString& target);

	// 投影变换
	QString transform(const QString& source, const QString& target, const QString& s_srs, const QString& t_srs, const QString &resampling_method, const double nodata = 0);

	// 保留栅格值小数位数
	QString pixelDecimal(const QString &source, const QString &target, const int decimal);

	QString slopCalculation_S2(const QString &source, const QString &target);

	// 处理DSM/DEM差值 type = DSM or DSM or DSMDEM
	QString dsmdemDiffeProcess(const QString &dsm, const QString &dem, const QString &outRaster, const QString &type, const double threshold, const bool isFillNodata);

	// 通过统一赋值分离有效值与无效值
	QString extractRasterRange(const QString &source, const QString &target, const double background);

	// 过滤栅格指定无效值
	QString filterInvalidValue(const QString &source, const QString &target, const QString invalidString, const bool isNegative, const bool isNodata, const bool bands_noDiffe);

	// 修改栅格值
	QString pixelModifyValue(const QString &source, const QString &target, const double valueOld_1, const double valueNew_1, const double valueOld_2, const double valueNew_2, const bool bands_noDiffe);

	// 填充栅格值
	QString pixelFillValue(const QString &source, const QString &target, const double nodata, const double value);

	// 创建金字塔
	QString buildOverviews(const QString &source);

	// 栅格转矢量
	QString rasterToVector(const QString &rasterFile, const QString &vectorFile, const int index);

	// 分隔矢量
	QString splitShp(const QString &shpName, QStringList &shps);

	// 融合矢量
	QString mergeVector(const QString& outPut, const QString& ogrLayer, const QString& fieldName);

	// 使用3x3滑动窗口计算栅格的标准差
	QString stdevp3x3Alg(const QString &source, const QString &target, const int iband = 1);

	static QString enumErrTypeToString(const ipfGdalProgressTools::errType err);
	static QString enumTypeToString(const int value);
	static QString enumFormatToString(const QString &format);

	void showProgressDialog();
	void hideProgressDialog();
	void pulsValueTatal();
	void setProgressTitle(const QString& label);
	void setProgressSize( const int max );

private:
    /**
    * \brief 调用GDALTranslate函数处理数据
    *
    * 该函数与GDALTranslate实用工具功能一致。
    *
    * @param QString 输入与GDALTranslate实用工具一致的参数
    *
    * @return
    */
	errType ipfGDALTranslate(const QString &str);

	/**
	* \brief 调用GDALWarp函数处理数据
	*
	* 该函数与GDALWarp实用工具功能一致。
	*
	* @param QString 输入与GDALWarp实用工具一致的参数
	*
	* @return
	*/
	errType ipfGDALWarp(const QString &str);

	/**
	* \brief 调用GDALbuildvrt函数处理数据
	*
	* 该函数与GDALbuildvrt实用工具功能一致。
	*
	* @param QString 输入与GDALbuildvrt实用工具一致的参数
	*
	* @return
	*/
	errType ipfGDALbuildvrt(const QString &str);

	/**
	* \brief 调用GDALlocationinfo函数计算像元坐标
	*
	* 该函数与GDALbuildvrt实用工具功能一致。
	*
	* @param QString 输入与GDALlocationinfo实用工具一致的参数
	*
	* @return
	*/
	errType ipfGDALlocationinfo(const QString &str, int &iRow, int &iCol);

	/**
	* \brief 调用GDALogr2ogr矢量处理函数
	*
	* 该函数与GDALogr2ogr实用工具功能一致。
	*
	* @param QString 输入与GDALogr2ogr实用工具一致的参数
	*
	* @return
	*/
	errType ipfGDALOgrToOgr(const QString &str);

	/**
	* \brief 调用GDALrasterize处理函数
	*
	* 该函数与GDALrasterize实用工具功能一致。
	*
	* @param QString 输入与GDALrasterize实用工具一致的参数
	*
	* @return
	*/
	errType ipfGDALrasterize(const QString &str);

    /**
    * \brief 分割QString字符串
    *
    * 该函数用于将QString字符串以空格分割，并复制到char**中返回。
    *
    * @param QString 以空格分开的字符串
    * @param doneSize 成功分割的字符串数量
    *
    * @return 返回char**，类似于main()参数。
    */
    int QStringToChar(const QString& str, char ***argv);

	/**
	* @brief 3*3模板算法回调函数
	* @param pafWindow			3*3窗口值数组
	* @param fDstNoDataValue	结果NoData值
	* @param pData				算法回调函数参数
	* @return 3*3窗口计算结果值
	*/
	typedef float(*GDALGeneric3x3ProcessingAlg) (float* pafWindow, float fDstNoDataValue, void* pData);

	/**
	* @brief 3*3模板计算处理函数
	* @param hSrcBand		输入图像波段
	* @param hDstBand		输出图像波段
	* @param pfnAlg			算法回调函数
	* @param pData			算法回调函数参数
	* @param pfnProgress	进度条回调函数
	* @param pProgressData	进度条回调函数参数
	*/
	errType GDALGeneric3x3Processing(GDALRasterBandH hSrcBand, GDALRasterBandH hDstBand,
		GDALGeneric3x3ProcessingAlg pfnAlg, void* pData,
		GDALProgressFunc pfnProgress, void * pProgressData);

	GDALDataset* createParametersRaster();
private:
	ipfProgress * proDialog;
	QStringList errList;

	//static ipfGdalProgressTools *smInstance;
};

#endif // IPFGDALPROGRESSTOOLS_H

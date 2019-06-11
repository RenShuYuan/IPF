#ifndef HEAD_H
#define HEAD_H

// 释放指针
#define RELEASE(x)  if(x!=NULL) {delete x; x = NULL;}
#define RELEASE_ARRAY(x)  if(x!=NULL) {delete [] x; x = NULL;}

// Qt
#include <QSettings>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QMessageBox>
#include <QFileDialog>
#include <QUUid>
#include <QTime>

// GDAL
#include "cpl_string.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "ogr_p.h"
#include "ipf/gdal/commonutils.h"
#include "ipf/gdal/gdal_utils_priv.h"

// QGis
#include "qgsrectangle.h"
#include "qgspoint.h"

// OpenGL
#define GL_RED                            0x1903
#define GL_RGB                            0x1907
#define GL_LUMINANCE                      0x1909
#define GL_STENCIL_INDEX                  0x1901
#define GL_DEPTH_COMPONENT                0x1902
#define GL_INTENSITY8                     0x804B
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_FLOAT                          0x1406

typedef enum
{
	IPF_ZERO = 0,
	IPF_EQUAL = 1,
	IPF_PLUS = 2,
	IPF_MINUS = 3,
	IPF_NONE = 4
} IPF_COMPUTE_TYPE;

#define PI 3.14159265

// 全球项目分幅产品分辨率 Global geographic information
#define GGI_DOM_2M		2.0
#define GGI_DOM_16M		16.0
#define GGI_DEM			10.0
#define GGI_DSM			10.0

// IPF
static const QString IPF_NODATA_NONE("none");
static const QString NAME_DELIMITER(QStringLiteral("@,@"));

// 参数模板中各参数位置
/*像元值位数保留*/
static const int P_DECIMAL = 0;

// IPF模块名称
static const QString MODELER_IN(QStringLiteral("输入"));
static const QString MODELER_OUT(QStringLiteral("输出"));

static const QString MODELER_TYPECONVERT(QStringLiteral("位深转换"));
static const QString MODELER_FRACCLIP(QStringLiteral("分幅裁切"));
static const QString MODELER_CLIP_VECTOR(QStringLiteral("AOI裁切"));
static const QString MODELER_QUICKVIEW(QStringLiteral("降分辨率"));
static const QString MODELER_RESAMPLE(QStringLiteral("重采样"));
static const QString MODELER_MOSAIC(QStringLiteral("镶嵌"));
static const QString MODELER_TRANSFORM(QStringLiteral("投影变换"));
static const QString MODELER_PIXELDECIMALS(QStringLiteral("像元值位数保留"));
static const QString MODELER_EXCEL_METADATA(QStringLiteral("创建元数据"));
static const QString MODELER_TFW(QStringLiteral("创建TFW"));
static const QString MODELER_BUILDOVERVIEWS(QStringLiteral("创建金字塔"));
static const QString MODELER_RECTPOSITION(QStringLiteral("处理栅格起始坐标"));
static const QString MODELER_FRACDIFFERCHECK(QStringLiteral("标准分幅栅格接边检查"));
static const QString MODELER_FRACEXTENTCHECK(QStringLiteral("标准分幅栅格范围检查"));
static const QString MODELER_PIXEL_REPLACE(QStringLiteral("像元值替换"));
static const QString MODELER_WATERFLATTENCHECK(QStringLiteral("高程模型水平检查"));
static const QString MODELER_PROJECTIONCHECK(QStringLiteral("栅格投影检查"));
static const QString MODELER_ZCHECK(QStringLiteral("高程模型精度检查"));
static const QString MODELER_RASTERINFOPRINT(QStringLiteral("栅格基本信息输出"));
static const QString MODELER_INVALIDVALUECHECK(QStringLiteral("无效值检查"));
static const QString MODELER_DEMGROSSERRORCHECK(QStringLiteral("数字高程模型粗差检查"));
static const QString MODELER_VEGETATION_EXTRACTION(QStringLiteral("植被提取"));
static const QString MODELER_WATERS_EXTRACTION(QStringLiteral("水域提取"));
static const QString MODELER_SETNODATA(QStringLiteral("设置NODATA"));
static const QString MODELER_DSMDEMDIFFECHECK(QStringLiteral("DSM-DEM差值检查"));
static const QString MODELER_DSMDEMDIFFEPROCESS(QStringLiteral("DSM-DEM差值处理"));
static const QString MODELER_RANGEMOIDFYVALUE(QStringLiteral("范围赋值"));
static const QString MODELER_SEAMOIDFYVALUE(QStringLiteral("海域赋值"));
static const QString MODELER_SPIKEPOINTCHECK(QStringLiteral("高程模型跳点检测"));

static const QString MODELER_SLOPCALCULATION(QStringLiteral("标准偏差-测试"));
static const QString MODELER_FRACEXTENTPROCESS(QStringLiteral("测试专用"));
static const QString MODELER_EXTRACT_RASTER_RANGE(QStringLiteral("提取栅格有效范围-测试"));

#endif // HEAD_H

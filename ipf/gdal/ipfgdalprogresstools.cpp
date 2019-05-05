#include <QApplication>
#include <QFile>
#include <QDir>
#include "ipfGdalProgressTools.h"
#include "../Process/ipfFlowManage.h"
#include "../ipfOgr.h"
#include "ipfProgress.h"
#include "gdal_rat.h"
#include <gdal_vrt.h>
#include <windows.h>
#include <omp.h>
#include <cmath>

#include "QgsCoordinateReferenceSystem.h"

int IPF_DECIMAL = 0;

double IPF_VALUE_OLD_1 = 0.0;
double IPF_VALUE_NEW_1 = 0.0;
double IPF_VALUE_OLD_2 = 0.0;
double IPF_VALUE_NEW_2 = 0.0;

double IPF_NODATA = 0.0;
QList<double> IPF_BANSNODATA;
double IPF_BACKGROUND = 0.0;
QList<double> IPF_INVALIDVALUE;
bool IPF_ISNEGATIVE = false;
bool IPF_ISNODATA = false;

double IPF_DSM_NODATA = 0.0;
double IPF_DEM_NODATA = 0.0;
QString IPF_DSMDEM_TYPE = "DSM";
double IPF_THRESHOLD = 1;
bool IPF_FILLNODATA = true;

double IPF_RANGE_VALUE = 0.0;
double IPF_RANGE_NODATA = 0.0;

//ipfGdalProgressTools *ipfGdalProgressTools::smInstance = nullptr;

/************************************************************************/
/*                 GDALVectorTranslateOptionsForBinaryNew()             */
/************************************************************************/

static GDALVectorTranslateOptionsForBinary *GDALVectorTranslateOptionsForBinaryNew(void)
{
	return (GDALVectorTranslateOptionsForBinary*)CPLCalloc(1, sizeof(GDALVectorTranslateOptionsForBinary));
}

/************************************************************************/
/*                  GDALVectorTranslateOptionsForBinaryFree()           */
/************************************************************************/

static void GDALVectorTranslateOptionsForBinaryFree(GDALVectorTranslateOptionsForBinary* psOptionsForBinary)
{
	if (psOptionsForBinary)
	{
		CPLFree(psOptionsForBinary->pszDataSource);
		CPLFree(psOptionsForBinary->pszDestDataSource);
		CSLDestroy(psOptionsForBinary->papszOpenOptions);
		CPLFree(psOptionsForBinary->pszFormat);
		CPLFree(psOptionsForBinary);
	}
}

/* -------------------------------------------------------------------- */
/*                  CheckDestDataSourceNameConsistency()                */
/* -------------------------------------------------------------------- */

static
void CheckDestDataSourceNameConsistency(const char* pszDestFilename,
	const char* pszDriverName)
{
	int i;
	char* pszDestExtension = CPLStrdup(CPLGetExtension(pszDestFilename));

	if (EQUAL(pszDriverName, "GMT"))
		pszDriverName = "OGR_GMT";
	CheckExtensionConsistency(pszDestFilename, pszDriverName);

	static const char* apszBeginName[][2] = { { "PG:"      , "PostgreSQL" },
											   { "MySQL:"   , "MySQL" },
											   { "CouchDB:" , "CouchDB" },
											   { "GFT:"     , "GFT" },
											   { "MSSQL:"   , "MSSQLSpatial" },
											   { "ODBC:"    , "ODBC" },
											   { "OCI:"     , "OCI" },
											   { "SDE:"     , "SDE" },
											   { "WFS:"     , "WFS" },
											   { NULL, NULL }
	};

	for (i = 0; apszBeginName[i][0] != NULL; i++)
	{
		if (EQUALN(pszDestFilename, apszBeginName[i][0], strlen(apszBeginName[i][0])) &&
			!EQUAL(pszDriverName, apszBeginName[i][1]))
		{
			CPLError(CE_Warning, CPLE_AppDefined,
				"The target file has a name which is normally recognized by the %s driver,\n"
				"but the requested output driver is %s. Is it really what you want ?\n",
				apszBeginName[i][1],
				pszDriverName);
			break;
		}
	}

	CPLFree(pszDestExtension);
}

/************************************************************************/
/*                       GDALTranslateOptionsForBinaryNew()             */
/************************************************************************/

static GDALTranslateOptionsForBinary *GDALTranslateOptionsForBinaryNew(void)
{
    return (GDALTranslateOptionsForBinary*) CPLCalloc(  1, sizeof(GDALTranslateOptionsForBinary) );
}

/************************************************************************/
/*                       GDALTranslateOptionsForBinaryFree()            */
/************************************************************************/

static void GDALTranslateOptionsForBinaryFree( GDALTranslateOptionsForBinary* psOptionsForBinary )
{
    if( psOptionsForBinary )
    {
        CPLFree(psOptionsForBinary->pszSource);
        CPLFree(psOptionsForBinary->pszDest);
        CSLDestroy(psOptionsForBinary->papszOpenOptions);
        CPLFree(psOptionsForBinary->pszFormat);
        CPLFree(psOptionsForBinary);
    }
}

/************************************************************************/
/*                       GDALWarpAppOptionsForBinaryNew()             */
/************************************************************************/

static GDALWarpAppOptionsForBinary *GDALWarpAppOptionsForBinaryNew(void)
{
	return (GDALWarpAppOptionsForBinary*)CPLCalloc(1, sizeof(GDALWarpAppOptionsForBinary));
}

/************************************************************************/
/*                       GDALWarpAppOptionsForBinaryFree()            */
/************************************************************************/

static void GDALWarpAppOptionsForBinaryFree(GDALWarpAppOptionsForBinary* psOptionsForBinary)
{
	if (psOptionsForBinary)
	{
		CSLDestroy(psOptionsForBinary->papszSrcFiles);
		CPLFree(psOptionsForBinary->pszDstFilename);
		CSLDestroy(psOptionsForBinary->papszOpenOptions);
		CSLDestroy(psOptionsForBinary->papszDestOpenOptions);
		CPLFree(psOptionsForBinary->pszFormat);
		CPLFree(psOptionsForBinary);
	}
}

/************************************************************************/
/*                       GDALBuildVRTOptionsForBinaryNew()              */
/************************************************************************/

static GDALBuildVRTOptionsForBinary *GDALBuildVRTOptionsForBinaryNew(void)
{
	return (GDALBuildVRTOptionsForBinary*)CPLCalloc(1, sizeof(GDALBuildVRTOptionsForBinary));
}

/************************************************************************/
/*                       GDALBuildVRTOptionsForBinaryFree()            */
/************************************************************************/

static void GDALBuildVRTOptionsForBinaryFree(GDALBuildVRTOptionsForBinary* psOptionsForBinary)
{
	if (psOptionsForBinary)
	{
		CSLDestroy(psOptionsForBinary->papszSrcFiles);
		CPLFree(psOptionsForBinary->pszDstFilename);
		CPLFree(psOptionsForBinary);
	}
}

/************************************************************************/
/*                       GDALRasterizeOptionsForBinaryNew()             */
/************************************************************************/

static GDALRasterizeOptionsForBinary *GDALRasterizeOptionsForBinaryNew(void)
{
	return static_cast<GDALRasterizeOptionsForBinary *>(
		CPLCalloc(1, sizeof(GDALRasterizeOptionsForBinary)));
}

/************************************************************************/
/*                       GDALRasterizeOptionsForBinaryFree()            */
/************************************************************************/

static void GDALRasterizeOptionsForBinaryFree(
	GDALRasterizeOptionsForBinary* psOptionsForBinary)
{
	if (psOptionsForBinary)
	{
		CPLFree(psOptionsForBinary->pszSource);
		CPLFree(psOptionsForBinary->pszDest);
		CPLFree(psOptionsForBinary->pszFormat);
		CPLFree(psOptionsForBinary);
	}
}

/************************************************************************/
/*                             SanitizeSRS                              */
/************************************************************************/

static char *SanitizeSRS(const char *pszUserInput)

{
	CPLErrorReset();

	OGRSpatialReferenceH hSRS = OSRNewSpatialReference(NULL);

	char *pszResult = NULL;
	if (OSRSetFromUserInput(hSRS, pszUserInput) == OGRERR_NONE)
		OSRExportToWkt(hSRS, &pszResult);
	else
	{
		CPLError(CE_Failure, CPLE_AppDefined,
			"Translating source or target SRS failed:\n%s",
			pszUserInput);
		exit(1);
	}

	OSRDestroySpatialReference(hSRS);

	return pszResult;
}

/*
* \brief 调用GDAL进度条接口
*
* 该函数用于将GDAL算法中的进度信息导出到CProcessBase基类中，供给界面显示
*
* @param dfComplete 完成进度值，其取值为 0.0 到 1.0 之间
* @param pszMessage 进度信息
* @param pProgressArg   CProcessBase的指针
*
* @return 返回TRUE表示继续计算，否则为取消
*/
int ALGTermProgress(double dfComplete, const char *pszMessage, void *pProgressArg)
{
	if (pProgressArg != NULL)
	{
		ipfProgress * pProcess = (ipfProgress*)pProgressArg;
		pProcess->setValue((int)(dfComplete * 100));

		if (pProcess->wasCanceled())
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	else
		return TRUE;
}

/*
*	3x3模板算子 ---------------->
*/

float stdevpAlg(float* pafWindow, float fDstNoDataValue, void* pData)
{
	float arvge = 0.0;
	float devsq = 0.0;
	float stdevp = 0.0;

	//AlgData3x3* psData = (AlgData3x3*)pData;
	
	// 3x3
	//for (int i=0; i<9; ++i)
	//	arvge += pafWindow[i];
	//arvge = arvge / 9;

	//for (int i = 0; i < 9; ++i)
	//	devsq += pow(pafWindow[i] - arvge, 2);

	//stdevp = sqrt(devsq / 9);
	
	// 横向扫描
	//arvge = pafWindow[1] + pafWindow[4] + pafWindow[7];
	//arvge = arvge / 3;

	//for (int i = 0; i < 3; ++i)
	//	devsq += pow(pafWindow[i] - arvge, 2);

	//stdevp = sqrt(devsq / 3);

	// 获得相邻八个像元的R值

	// 对R值进行加权平均

	// 使用加权平均后的R值结合NIR计算NDVI

	return stdevp;
}

/*
*	3x3模板算子 ----------------<
*/

/*
*	vrt算法 ---------------->
*/

// 按指定位数保留小数算法
CPLErr pixelDecimalFunction(void **papoSources, int nSources, void *pData, int nXSize, int nYSize,
	GDALDataType eSrcType, GDALDataType eBufType, int nPixelSpace, int nLineSpace)
{
	// ---- Init ----
	if (nSources != 1) return CE_Failure;

	// ---- Set pixels ----
	for (int iLine = 0; iLine < nYSize; iLine++)
	{
		for (int iCol = 0; iCol < nXSize; iCol++)
		{
			int ii = iLine * nXSize + iCol;
			/* 使用SRCVAL获取源栅格的像素 */
			double x0 = SRCVAL(papoSources[0], eSrcType, ii);
			if (x0 != 0)
				x0 = ((float)((long)(x0*IPF_DECIMAL))) / IPF_DECIMAL;

			// write
			GDALCopyWords(&x0, GDT_Float64, 0,
				((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
				eBufType, nPixelSpace, 1);
		}
	}

	// ---- Return success ----
	return CE_None;
}

// 通过统一赋值分离有效值与无效值
CPLErr pixelModifyUnBackGroundFunction(void **papoSources, int nSources, void *pData, int nXSize, int nYSize,
	GDALDataType eSrcType, GDALDataType eBufType, int nPixelSpace, int nLineSpace)
{
	int ii = 0, iLine = 0, iCol = 0;
	double x0 = 0.0;

	// ---- Set pixels ----
	for (iLine = 0; iLine < nYSize; iLine++) // 遍历行
	{
		for (iCol = 0; (iCol + nSources) < nXSize; iCol += nSources) // 遍历列
		{
			bool bl = true;
			for (int i = 0; i < nSources; ++i) // 遍历波段
			{
				ii = iLine * nXSize + iCol + i;
				x0 = SRCVAL(papoSources[0], eSrcType, ii); // 获得像元值
				if (x0 != IPF_BACKGROUND)
				{
					bl = false;
					break;
				}
			}
			if (bl)
				x0 = IPF_NODATA;
			else
				x0 = 100;

			// write
			GDALCopyWords(&x0, GDT_Float64, 0,
				((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
				eBufType, nPixelSpace, 1);
		}
	}

	// ---- Return success ----
	return CE_None;
}

CPLErr pixelSlopFunction_S2(void **papoSources, int nSources, void *pData, int nXSize, int nYSize,
	GDALDataType eSrcType, GDALDataType eBufType, int nPixelSpace, int nLineSpace)
{
	int ii = 0, iLine = 0, iCol = 0;
	int index_red = 2; // 对应papoSources的波段索引，从0开始
	int index_nir = 3; // 对应papoSources的波段索引，从0开始
	int index[9];
	double matrix[9];
	double x0 = 0.0;
	double red = 0.0;
	double nir = 0.0;

	if (nSources != 4) return CE_Failure;

	// ---- Set pixels ----
	for (iLine = 0; iLine < nYSize; iLine++) // 遍历行
	{
		for (iCol = 0; iCol < nXSize; iCol++) // 遍历列
		{
			// 判断像元是否在栅格边缘
			if (iLine == 0 || iLine == nYSize -1
				|| iCol == 0 || iCol == nXSize -1)
			{
				continue;
			}
			else
			{
				index[0] = (iLine - 1) * nXSize + (iCol - 1);
				index[1] = (iLine - 1) * nXSize + iCol;
				index[2] = (iLine - 1) * nXSize + (iCol + 1);
				index[3] = iLine * nXSize + (iCol - 1);
				index[4] = iLine * nXSize + iCol;
				index[5] = iLine * nXSize + (iCol + 1);
				index[6] = (iLine + 1) * nXSize + (iCol - 1);
				index[7] = (iLine + 1) * nXSize + iCol;
				index[8] = (iLine + 1) * nXSize + (iCol + 1);
			}

			// 获得red波段值
			red = SRCVAL(papoSources[index_red], eSrcType, index[4]);

			// 获得 3x3 的NIR波段值
			matrix[0] = SRCVAL(papoSources[index_nir], eSrcType, index[0]);
			matrix[1] = SRCVAL(papoSources[index_nir], eSrcType, index[1]);
			matrix[2] = SRCVAL(papoSources[index_nir], eSrcType, index[2]);
			matrix[3] = SRCVAL(papoSources[index_nir], eSrcType, index[3]);
			matrix[4] = SRCVAL(papoSources[index_nir], eSrcType, index[4]);
			matrix[5] = SRCVAL(papoSources[index_nir], eSrcType, index[5]);
			matrix[6] = SRCVAL(papoSources[index_nir], eSrcType, index[6]);
			matrix[7] = SRCVAL(papoSources[index_nir], eSrcType, index[7]);
			matrix[8] = SRCVAL(papoSources[index_nir], eSrcType, index[8]);
			
			// NIR加权平均计算
			double weight = (1 - 0.2) / 8;
			nir = (matrix[0] * weight + matrix[1] * weight + matrix[2] * weight
				+ matrix[3] * weight + matrix[4] * 0.2 + matrix[5] * weight
				+ matrix[6] * weight + matrix[7] * weight + matrix[8] * weight) / 1;

			// 计算NDVI
			x0 = (nir - red) / (nir + red);

			// write
			GDALCopyWords(&x0, GDT_Float64, 0,
				((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
				eBufType, nPixelSpace, 1);
		}
	}

	// ---- Return success ----
	return CE_None;
}

CPLErr pixelDSMDEMDiffProcessFunction(void **papoSources, int nSources, void *pData, int nXSize, int nYSize,
	GDALDataType eSrcType, GDALDataType eBufType, int nPixelSpace, int nLineSpace)
{
	int ii = 0, iLine = 0, iCol = 0;
	int index_b1 = 0;
	int index_b2 = 1;
	double band1Value = 0.0;
	double band2Value = 0.0;
	int index = 0;
	double x0 = 0.0;

	if (nSources != 2) return CE_Failure;

	// ---- Set pixels ----
	for (iLine = 0; iLine < nYSize; iLine++) // 遍历行
	{
		for (iCol = 0; iCol < nXSize; iCol++) // 遍历列
		{
			index = iLine * nXSize + iCol;
			band1Value = SRCVAL(papoSources[index_b1], eSrcType, index);
			band2Value = SRCVAL(papoSources[index_b2], eSrcType, index);

			if (IPF_DSMDEM_TYPE == "DSM")
			{
				if (band1Value == IPF_DSM_NODATA)
				{
					if ((band2Value != IPF_DEM_NODATA) && IPF_FILLNODATA)
					{
						x0 = band2Value;
					}
					else
					{
						x0 = band1Value;
					}
				}
				else
				{
					if (band2Value == IPF_DEM_NODATA)
					{
						x0 = band1Value;
					}
					else
					{
						double diffVlaue = band1Value - band2Value;
						if ((diffVlaue < 0) && (abs(diffVlaue) < IPF_THRESHOLD))
							x0 = band2Value;
						else
							x0 = band1Value;
					}
				}
			}
			else // IPF_DSMDEM_TYPE == "DEM"
			{
				if (band1Value == IPF_DEM_NODATA)
				{
					if ((band2Value != IPF_DSM_NODATA) && IPF_FILLNODATA)
					{
						x0 = band2Value;
					}
					else
					{
						x0 = band1Value;
					}
				}
				else
				{
					if (band2Value == IPF_DSM_NODATA)
					{
						x0 = band1Value;
					}
					else
					{
						double diffVlaue = band2Value - band1Value;
						if ((diffVlaue < 0) && (abs(diffVlaue) < IPF_THRESHOLD))
							x0 = band2Value;
						else
							x0 = band1Value;
					}
				}
			}

			// write
			GDALCopyWords(&x0, GDT_Float64, 0,
				((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
				eBufType, nPixelSpace, 1);
		}
	}

	// ---- Return success ----
	return CE_None;
}

// 替换像元值
CPLErr pixelModifyValueFunction(void **papoSources, int nSources, void *pData, int nXSize, int nYSize,
	GDALDataType eSrcType, GDALDataType eBufType, int nPixelSpace, int nLineSpace)
{
	int ii = 0, iLine = 0, iCol = 0;
	double x0 = 0.0;

	// ---- Init ----
	if (nSources != 1) return CE_Failure;

	// ---- Set pixels ----
	for (iLine = 0; iLine < nYSize; iLine++)
	{
		for (iCol = 0; iCol < nXSize; iCol++)
		{
			ii = iLine * nXSize + iCol;
			/* 使用SRCVAL获取源栅格的像素 */
			x0 = SRCVAL(papoSources[0], eSrcType, ii);
			if (IPF_VALUE_OLD_1 != IPF_VALUE_NEW_1)
			{
				if (x0 == IPF_VALUE_OLD_1)
					x0 = IPF_VALUE_NEW_1;
				else if (IPF_VALUE_OLD_2 != IPF_VALUE_NEW_2)
				{
					if (x0 == IPF_VALUE_OLD_2)
						x0 = IPF_VALUE_NEW_2;
				}
			}

			// write
			GDALCopyWords(&x0, GDT_Float64, 0,
				((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
				eBufType, nPixelSpace, 1);
		}
	}

	// ---- Return success ----
	return CE_None;
}

// 填充固定值
CPLErr pixelFillValueFunction(void **papoSources, int nSources, void *pData, int nXSize, int nYSize,
	GDALDataType eSrcType, GDALDataType eBufType, int nPixelSpace, int nLineSpace)
{
	int ii = 0, iLine = 0, iCol = 0;
	double x0 = 0.0;

	// ---- Init ----
	if (nSources != 1) return CE_Failure;

	// ---- Set pixels ----
	for (iLine = 0; iLine < nYSize; iLine++)
	{
		for (iCol = 0; iCol < nXSize; iCol++)
		{
			ii = iLine * nXSize + iCol;
			/* 使用SRCVAL获取源栅格的像素 */
			x0 = SRCVAL(papoSources[0], eSrcType, ii);
			if (x0 == IPF_RANGE_NODATA)
				x0 = IPF_RANGE_NODATA;
			else
				x0 = IPF_RANGE_VALUE;

			// write
			GDALCopyWords(&x0, GDT_Float64, 0,
				((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
				eBufType, nPixelSpace, 1);
		}
	}

	// ---- Return success ----
	return CE_None;
}

// 过滤无效值
CPLErr pixelInvalidValue(void **papoSources, int nSources, void *pData, int nXSize, int nYSize,
	GDALDataType eSrcType, GDALDataType eBufType, int nPixelSpace, int nLineSpace)
{
	int iLine = 0, iCol = 0;

	// ---- Set pixels ----
	for (iLine = 0; iLine < nYSize; iLine++) // 遍历行
	{
		for (iCol = 0; iCol < nXSize; iCol++) // 遍历列
		{
			// 获得当前位置所有波段值
			double *valueList = new double[nSources];
			int index = iLine * nXSize + iCol;
			for (int i = 0; i < nSources; ++i)
			{
				valueList[i] = SRCVAL(papoSources[i], eSrcType, index);
			}

			int x0 = 0;

			// 检查无效数据、异常值
			for (int i = 0; i < nSources; ++i)
			{
				if (::isnan(valueList[i]))
				{
					x0 = 1;
					break;
				}
			}

			// 检查负值
			if (IPF_ISNEGATIVE && !x0)
			{
				for (int i = 0; i < nSources; ++i)
				{
					if (valueList[i] < 0)
					{
						x0 = 1;
						break;
					}
				}
			}

			// 检查NODATA
			if (IPF_ISNODATA && !x0)
			{
				for (int i = 0; i < nSources; ++i)
				{
					if (valueList[i] == IPF_BANSNODATA.at(i))
					{
						x0 = 1;
						break;
					}
				}
			}

			// 检查无效枚举值
			if (!x0 && !IPF_INVALIDVALUE.isEmpty())
			{
				for (int i = 0; i < nSources; ++i)
				{
					if (IPF_INVALIDVALUE.contains(valueList[i]))
					{
						x0 = 1;
						break;
					}
				}
			}

			// write
			GDALCopyWords(&x0, GDT_Byte, 0,
				((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
				eBufType, nPixelSpace, 1);

			RELEASE_ARRAY(valueList);
		}
	}

	// ---- Return success ----
	return CE_None;
}

/*
*	vrt算法 ----------------<
*/

ipfGdalProgressTools::ipfGdalProgressTools()
{
	//smInstance = this;

	// 初始化进度条
	proDialog = new ipfProgress();
	proDialog->setAttribute(Qt::WA_DeleteOnClose, true);
	proDialog->setRangeChild(0, 100);

	// 注册用于处理VRT的算法
	GDALAddDerivedBandPixelFunc("pixelDecimalFunction", pixelDecimalFunction);
	GDALAddDerivedBandPixelFunc("pixelModifyValueFunction", pixelModifyValueFunction);
	GDALAddDerivedBandPixelFunc("pixelModifyUnBackGroundFunction", pixelModifyUnBackGroundFunction); 
	GDALAddDerivedBandPixelFunc("pixelDSMDEMDiffProcessFunction", pixelDSMDEMDiffProcessFunction);
	GDALAddDerivedBandPixelFunc("pixelInvalidValue", pixelInvalidValue); 
	GDALAddDerivedBandPixelFunc("pixelFillValueFunction", pixelFillValueFunction);
	GDALAddDerivedBandPixelFunc("pixelSlopFunction_S2", pixelSlopFunction_S2);
}

ipfGdalProgressTools::~ipfGdalProgressTools()
{
    delete proDialog;
}

qulonglong ipfGdalProgressTools::getFreePhysicalMemory()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	qulonglong size = statex.ullAvailPhys;

	return size;
}

int ipfGdalProgressTools::getNoOfProcessors()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

QString ipfGdalProgressTools::locationPixelInfo(const QString & source, const double x, const double y, int & iRow, int & iCol)
{
	QString strArgv = QString("%1 -geoloc %2 %3").arg(source).arg(x).arg(y);

	ipfGdalProgressTools::errType err = ipfGDALlocationinfo(strArgv, iRow, iCol);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::locationPixelInfo(const QString & source, const QString & srs, const double x, const double y, int & iRow, int & iCol)
{
	QString strArgv = QString("%1 %2 %3 %4").arg(source).arg(srs).arg(x).arg(y);

	ipfGdalProgressTools::errType err = ipfGDALlocationinfo(strArgv, iRow, iCol);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::formatConvert(const QString &source, const QString &target, const QString &format
											, const QString &compress, const QString &isTfw, const QString &noData)
{
	QString strArgv;
	if (format == QStringLiteral("GTiff"))
	{
		if (noData == "none")
			strArgv = QString("-co TFW=%1 -co COMPRESS=%2 -of %3 %4 %5").arg(isTfw).arg(compress).arg(format).arg(source).arg(target);
		else
			strArgv = QString("-co TFW=%1 -co COMPRESS=%2 -a_nodata %3 -of %4 %5 %6").arg(isTfw).arg(compress).arg(noData).arg(format).arg(source).arg(target);
	}
	else
	{
		if (noData == "none")
			strArgv = QString("-of %1 %2 %3").arg(format).arg(source).arg(target);
		else
			strArgv = QString("-a_nodata %1 -of %2 %3 %4").arg(noData).arg(format).arg(source).arg(target);
	}

	ipfGdalProgressTools::errType err = ipfGDALTranslate(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::typeConvert(const QString & source, const QString & target, const QString & type)
{
	QString strArgv = QString("-ot %1 -scale -of VRT %2 %3").arg(type).arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALTranslate(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::typeConvertNoScale(const QString & source, const QString & target, const QString & type)
{
	QString strArgv = QString("-ot %1 -of VRT %2 %3").arg(type).arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALTranslate(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::proToClip_Translate(const QString & source, const QString & target, const QgsRectangle & rect)
{
	QString strArgv = QString("-projwin %1 %2 %3 %4 -of VRT %5 %6")
		.arg(rect.xMinimum()).arg(rect.yMaximum())
		.arg(rect.xMaximum()).arg(rect.yMinimum())
		.arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALTranslate(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::proToClip_Translate_src(const QString & source, const QString & target, const QList<int> list)
{
	QString strArgv = QString("-srcwin %1 %2 %3 %4 -of VRT %5 %6")
		.arg(list.at(0)).arg(list.at(1))
		.arg(list.at(2)).arg(list.at(3))
		.arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALTranslate(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::proToClip_Warp(const QString & source, const QString & target, const QList<double> list)
{
	QString strArgv = QString("gdalwarp -multi -co NUM_THREADS=ALL_CPUS -wo NUM_THREADS=ALL_CPUS -oo NUM_THREADS=ALL_CPUS -doo NUM_THREADS=ALL_CPUS -overwrite -te %1 %2 %3 %4 -of VRT %5 %6")
		.arg(list.at(0), 0, 'f', 11).arg(list.at(3), 0, 'f', 11)
		.arg(list.at(2), 0, 'f', 11).arg(list.at(1), 0, 'f', 11)
		.arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALWarp(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::AOIClip(const QString & source, const QString & target, const QString & vectorName)
{
	QString strArgv;

	strArgv = QString("gdalwarp -multi -co NUM_THREADS=ALL_CPUS -wo NUM_THREADS=ALL_CPUS -oo NUM_THREADS=ALL_CPUS -doo NUM_THREADS=ALL_CPUS -cutline %1 -of VRT %2 %3")
		.arg(vectorName).arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALWarp(strArgv);
	QString str = enumErrTypeToString(err);

	return str;
}

QString ipfGdalProgressTools::quickView(const QString & source, const QString & target, const int b)
{
	double resolution = 100;
	for (int i = 0; i < b; ++i)
		resolution = resolution * 0.5;
	QString rStr = QString("%1%").arg(QString::number(resolution));
	
	QString strArgv = QString("-outsize %1 %2 %3 %4")
							.arg(rStr).arg(rStr).arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALTranslate(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::resample(const QString & source, const QString & target, const double res, const QString &resampling_method)
{
	QString strArgv = QString("gdalwarp -multi -co NUM_THREADS=ALL_CPUS -wo NUM_THREADS=ALL_CPUS -oo NUM_THREADS=ALL_CPUS -doo NUM_THREADS=ALL_CPUS -overwrite -r %1 -tr %2 %3 -of VRT %4 %5")
		.arg(resampling_method).arg(res, 0, 'f', 15).arg(res, 0, 'f', 15).arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALWarp(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::mosaic_Warp(const QStringList & sourceList, const QString & target)
{
	QString source;
	foreach (QString file, sourceList)
		source = source + ' ' + file;

	QString strArgv = QString("gdalwarp -multi -co NUM_THREADS=ALL_CPUS -wo NUM_THREADS=ALL_CPUS -oo NUM_THREADS=ALL_CPUS -doo NUM_THREADS=ALL_CPUS -overwrite -of VRT %1 %2").arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALWarp(strArgv);
	QString str = enumErrTypeToString(err);

	return str;
}

QString ipfGdalProgressTools::mosaic_Buildvrt(const QStringList & sourceList, const QString & target)
{
	QString source;
	foreach(QString file, sourceList)
		source = source + ' ' + file;

	QString strArgv = QString("gdalbuildvrt %1 %2").arg(target).arg(source);

	ipfGdalProgressTools::errType err = ipfGDALbuildvrt(strArgv);
	QString str = enumErrTypeToString(err);

	return str;
}

QString ipfGdalProgressTools::mergeBand(const QStringList & sourceList, const QString & target)
{
	QString source;
	foreach(QString file, sourceList)
		source = source + ' ' + file;

	QString strArgv = QString("gdalbuildvrt -separate %1 %2").arg(target).arg(source);

	ipfGdalProgressTools::errType err = ipfGDALbuildvrt(strArgv);
	QString str = enumErrTypeToString(err);

	return str;
}

QString ipfGdalProgressTools::transform(const QString & source, const QString & target
	, const QString & s_srs, const QString & t_srs
	, const QString &resampling_method
	, const double nodata)
{
	QString strArgv = QString("gdalwarp -multi -co NUM_THREADS=ALL_CPUS -wo NUM_THREADS=ALL_CPUS -oo NUM_THREADS=ALL_CPUS -doo NUM_THREADS=ALL_CPUS -srcnodata %1 -overwrite -r %2 -s_srs %3 -t_srs %4 -of VRT %5 %6")
		.arg(nodata).arg(resampling_method).arg(s_srs).arg(t_srs).arg(source).arg(target);

	ipfGdalProgressTools::errType err = ipfGDALWarp(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

ipfGdalProgressTools::errType ipfGdalProgressTools::ipfGDALTranslate(const QString &str)
{
    QStringList list = str.split(' ', QString::SkipEmptyParts);
    int argc = list.size();
    if ( argc==0 )
        return eParameterNull;

    char **argv=(char **)malloc(sizeof(char*)*argc);
    for (int var = 0; var < argc; ++var)
    {
        std::string str = list.at(var).toStdString();
        const char* p = str.c_str();
        size_t cSize = strlen(p);
        char *c = (char*)malloc(sizeof(char*)*cSize);
        strncpy(c, p, cSize);
        c[cSize] = '\0';
        argv[var] = c;
    }

    // 提前设置配置信息
    EarlySetConfigOptions(argc, argv);

    // GDAL通用命令行处理
    argc = GDALGeneralCmdLineProcessor(argc, &argv, 0);
    if (argc < 1)
		return eParameterErr;

    // 通过设置获得最佳性能
    if (CPLGetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", NULL) == NULL)
    {
#if defined(__MACH__) && defined(__APPLE__)
        // On Mach, the default limit is 256 files per process
        // TODO We should eventually dynamically query the limit for all OS
        CPLSetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", "100");
#else
        CPLSetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", "2048");
#endif
    }

    GDALTranslateOptionsForBinary* psOptionsForBinary = GDALTranslateOptionsForBinaryNew();
    GDALTranslateOptions *psOptions = GDALTranslateOptionsNew(argv, psOptionsForBinary);

    // 释放字符串列表
    CSLDestroy(argv);

    if (psOptions == NULL)
        return eParameterNull;

    if (psOptionsForBinary->pszSource == NULL)
		return eSourceParameterNull;

    if (psOptionsForBinary->pszDest == NULL)
		return eDestParameterNull;

	// 是否禁用标准输出
	if (strcmp(psOptionsForBinary->pszDest, "/vsistdout/") == 0)
	{
		psOptionsForBinary->bQuiet = TRUE;
	}
	if (!(psOptionsForBinary->bQuiet))
	{
		GDALTranslateOptionsSetProgress(psOptions, ALGTermProgress, proDialog);
	}

    // 检查输出格式，如果不正确，则将列出所支持的格式
    if (psOptionsForBinary->pszFormat)
    {
        GDALDriverH hDriver = GDALGetDriverByName(psOptionsForBinary->pszFormat);
        if (hDriver == NULL)
        {
            GDALTranslateOptionsFree(psOptions);
            GDALTranslateOptionsForBinaryFree(psOptionsForBinary);
			return eFormatErr;
        }
    }

    GDALDatasetH hDataset, hOutDS;
    int bUsageError;

    // 尝试打开数据源
    hDataset = GDALOpenEx(psOptionsForBinary->pszSource, GDAL_OF_RASTER, NULL,
        (const char* const*)psOptionsForBinary->papszOpenOptions, NULL);

    if (hDataset == NULL)
        return eSourceOpenErr;

    // 处理子数据集
    if (!psOptionsForBinary->bCopySubDatasets
        && GDALGetRasterCount(hDataset) == 0
        && CSLCount(GDALGetMetadata(hDataset, "SUBDATASETS")) > 0)
    {
		GDALClose(hDataset);
        return eSubdatasets;
    }

    if (psOptionsForBinary->bCopySubDatasets &&
        CSLCount(GDALGetMetadata(hDataset, "SUBDATASETS")) > 0)
    {
        char **papszSubdatasets = GDALGetMetadata(hDataset, "SUBDATASETS");
        char *pszSubDest = (char *)CPLMalloc(strlen(psOptionsForBinary->pszDest) + 32);

        CPLString osPath = CPLGetPath(psOptionsForBinary->pszDest);
        CPLString osBasename = CPLGetBasename(psOptionsForBinary->pszDest);
        CPLString osExtension = CPLGetExtension(psOptionsForBinary->pszDest);
        CPLString osTemp;

        const char* pszFormat = NULL;
        if (CSLCount(papszSubdatasets) / 2 < 10)
        {
            pszFormat = "%s_%d";
        }
        else if (CSLCount(papszSubdatasets) / 2 < 100)
        {
            pszFormat = "%s_%002d";
        }
        else
        {
            pszFormat = "%s_%003d";
        }

        const char* pszDest = pszSubDest;

        for (int i = 0; papszSubdatasets[i] != NULL; i += 2)
        {
            char* pszSource = CPLStrdup(strstr(papszSubdatasets[i], "=") + 1);
            osTemp = CPLSPrintf(pszFormat, osBasename.c_str(), i / 2 + 1);
            osTemp = CPLFormFilename(osPath, osTemp, osExtension);
            strcpy(pszSubDest, osTemp.c_str());

            hDataset = GDALOpenEx(pszSource, GDAL_OF_RASTER, NULL,
                (const char* const*)psOptionsForBinary->papszOpenOptions, NULL);
            CPLFree(pszSource);

            hOutDS = GDALTranslate(pszDest, hDataset, psOptions, &bUsageError);

            if (bUsageError)
                return eOther;
            if (hOutDS == NULL)
                break;
            GDALClose(hOutDS);
        }

        GDALClose(hDataset);
        GDALTranslateOptionsFree(psOptions);
        GDALTranslateOptionsForBinaryFree(psOptionsForBinary);
        CPLFree(pszSubDest);

        return eSubdatasets;
    }

    // 开始正儿八经的处理了
    hOutDS = GDALTranslate(psOptionsForBinary->pszDest, hDataset, psOptions, &bUsageError);

	if (bUsageError == TRUE)
		return eOther;

    GDALClose(hOutDS);
    GDALClose(hDataset);
    GDALTranslateOptionsFree(psOptions);
    GDALTranslateOptionsForBinaryFree(psOptionsForBinary);

	return eOK;
}

ipfGdalProgressTools::errType ipfGdalProgressTools::ipfGDALWarp(const QString & str)
{
	int nSrcCount = 0;
	GDALDatasetH *pahSrcDS = NULL;

	QStringList list = str.split(' ', QString::SkipEmptyParts);
	int argc = list.size();
	if (argc == 0)
		return eParameterNull;

	char **argv = (char **)malloc(sizeof(char*)*argc);
	for (int var = 0; var < argc; ++var)
	{
		std::string str = list.at(var).toStdString();
		const char* p = str.c_str();
		size_t cSize = strlen(p);
		char *c = (char*)malloc(sizeof(char*)*cSize);
		strncpy(c, p, cSize);
		c[cSize] = '\0';
		argv[var] = c;
	}

	EarlySetConfigOptions(argc, argv);

	argc = GDALGeneralCmdLineProcessor(argc, &argv, 0);
	if (argc < 1)
		return eParameterErr;

	if (CPLGetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", NULL) == NULL)
	{
#if defined(__MACH__) && defined(__APPLE__)
		// On Mach, the default limit is 256 files per process
		// TODO We should eventually dynamically query the limit for all OS
		CPLSetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", "100");
#else
		CPLSetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", "2048");
#endif
	}

	GDALWarpAppOptionsForBinary* psOptionsForBinary = GDALWarpAppOptionsForBinaryNew();
	/* coverity[tainted_data] */
	GDALWarpAppOptions *psOptions = GDALWarpAppOptionsNew(argv + 1, psOptionsForBinary);
	CSLDestroy(argv);

	if (psOptions == NULL)
		return eParameterNull;

	if (psOptionsForBinary->pszDstFilename == NULL)
		return eDestParameterNull;

	if (CSLCount(psOptionsForBinary->papszSrcFiles) == 1 &&
		strcmp(psOptionsForBinary->papszSrcFiles[0], psOptionsForBinary->pszDstFilename) == 0 &&
		psOptionsForBinary->bOverwrite)
	{
		return eSouDestDiff;
	}

	/* -------------------------------------------------------------------- */
	/*      Open Source files.                                              */
	/* -------------------------------------------------------------------- */
	for (int i = 0; psOptionsForBinary->papszSrcFiles[i] != NULL; i++)
	{
		nSrcCount++;
		pahSrcDS = (GDALDatasetH *)CPLRealloc(pahSrcDS, sizeof(GDALDatasetH) * nSrcCount);
		pahSrcDS[nSrcCount - 1] = GDALOpenEx(psOptionsForBinary->papszSrcFiles[i], GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR, NULL,
			(const char* const*)psOptionsForBinary->papszOpenOptions, NULL);

		if (pahSrcDS[nSrcCount - 1] == NULL)
			return eSourceOpenErr;
	}

	/* -------------------------------------------------------------------- */
	/*      Does the output dataset already exist?                          */
	/* -------------------------------------------------------------------- */

	/* FIXME ? source filename=target filename and -overwrite is definitely */
	/* an error. But I can't imagine of a valid case (without -overwrite), */
	/* where it would make sense. In doubt, let's keep that dubious possibility... */

	int bOutStreaming = FALSE;
	if (strcmp(psOptionsForBinary->pszDstFilename, "/vsistdout/") == 0)
	{
		psOptionsForBinary->bQuiet = TRUE;
		bOutStreaming = TRUE;
	}
#ifdef S_ISFIFO
	else
	{
		VSIStatBufL sStat;
		if (VSIStatExL(psOptionsForBinary->pszDstFilename, &sStat, VSI_STAT_EXISTS_FLAG | VSI_STAT_NATURE_FLAG) == 0 &&
			S_ISFIFO(sStat.st_mode))
		{
			bOutStreaming = TRUE;
		}
	}
#endif

	GDALDatasetH hDstDS = NULL;
	if (bOutStreaming)
	{
		GDALWarpAppOptionsSetWarpOption(psOptions, "STREAMABLE_OUTPUT", "YES");
	}
	else
	{
		CPLPushErrorHandler(CPLQuietErrorHandler);
		hDstDS = GDALOpenEx(psOptionsForBinary->pszDstFilename, GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR | GDAL_OF_UPDATE,
			NULL, psOptionsForBinary->papszDestOpenOptions, NULL);
		CPLPopErrorHandler();
	}

	if (hDstDS != NULL && psOptionsForBinary->bOverwrite)
	{
		GDALClose(hDstDS);
		hDstDS = NULL;
	}

	if (hDstDS != NULL && psOptionsForBinary->bCreateOutput)
		return eOutputDatasetExists;

	/* Avoid overwriting an existing destination file that cannot be opened in */
	/* update mode with a new GTiff file */
	if (!bOutStreaming && hDstDS == NULL && !psOptionsForBinary->bOverwrite)
	{
		CPLPushErrorHandler(CPLQuietErrorHandler);
		hDstDS = GDALOpen(psOptionsForBinary->pszDstFilename, GA_ReadOnly);
		CPLPopErrorHandler();

		if (hDstDS)
		{
			GDALClose(hDstDS);
			return eOutputDatasetExists;
		}
	}

	if (!(psOptionsForBinary->bQuiet))
	{
		GDALWarpAppOptionsSetProgress(psOptions, ALGTermProgress, proDialog);
	}

	// 设置空闲内存的1/8
	//qulonglong memorySize = getFreePhysicalMemory();
	//memorySize *= 0.8;
	//GDALWarpAppOptionsSetWarpOption(psOptions, "dfWarpMemoryLimit", QString::number(memorySize).toStdString().c_str());

	// 设置CPU的核数用于计算
	//int nuCPUs = getNoOfProcessors();
	//GDALWarpAppOptionsSetWarpOption(psOptions, "NUM_THREADS", QString::number((int)(nuCPUs/2)).toStdString().c_str());
	//GDALWarpAppOptionsSetWarpOption(psOptions, "NUM_THREADS", "ALL_CPUS");
	//GDALWarpAppOptionsSetWarpOption(psOptions, "USE_OPENCL", "TRUE");

	if (hDstDS == NULL && !psOptionsForBinary->bQuiet && !psOptionsForBinary->bFormatExplicitlySet)
		CheckExtensionConsistency(psOptionsForBinary->pszDstFilename, psOptionsForBinary->pszFormat);
	
	int bUsageError = FALSE;
	GDALDatasetH hOutDS = GDALWarp(psOptionsForBinary->pszDstFilename, hDstDS,
		nSrcCount, pahSrcDS, psOptions, &bUsageError);
	if (bUsageError)
		return eOther;
	int nRetCode = (hOutDS) ? 0 : 1;

	GDALWarpAppOptionsFree(psOptions);
	GDALWarpAppOptionsForBinaryFree(psOptionsForBinary);

	// Close first hOutDS since it might reference sources (case of VRT)
	GDALClose(hOutDS ? hOutDS : hDstDS);
	for (int i = 0; i < nSrcCount; i++)
	{
		GDALClose(pahSrcDS[i]);
	}
	CPLFree(pahSrcDS);

	GDALDumpOpenDatasets(stderr);

	return eOK;
}

ipfGdalProgressTools::errType ipfGdalProgressTools::ipfGDALbuildvrt(const QString & str)
{
	QStringList list = str.split(' ', QString::SkipEmptyParts);
	int argc = list.size();
	if (argc == 0)
		return eParameterNull;

	char **argv = (char **)malloc(sizeof(char*)*argc);
	for (int var = 0; var < argc; ++var)
	{
		std::string str = list.at(var).toStdString();
		const char* p = str.c_str();
		size_t cSize = strlen(p);
		char *c = (char*)malloc(sizeof(char*)*cSize);
		strncpy(c, p, cSize);
		c[cSize] = '\0';
		argv[var] = c;
	}

	EarlySetConfigOptions(argc, argv);

	argc = GDALGeneralCmdLineProcessor(argc, &argv, 0);
	if (argc < 1)
		return eParameterErr;

	GDALBuildVRTOptionsForBinary* psOptionsForBinary = GDALBuildVRTOptionsForBinaryNew();
	/* coverity[tainted_data] */
	GDALBuildVRTOptions *psOptions = GDALBuildVRTOptionsNew(argv + 1, psOptionsForBinary);
	CSLDestroy(argv);

	if (psOptions == NULL)
		return eParameterNull;

	if (psOptionsForBinary->pszDstFilename == NULL)
		return eDestParameterNull;

	if (!(psOptionsForBinary->bQuiet))
	{
		GDALBuildVRTOptionsSetProgress(psOptions, ALGTermProgress, proDialog);
	}

	/* Avoid overwriting a non VRT dataset if the user did not put the */
	/* filenames in the right order */
	VSIStatBuf sBuf;
	if (!psOptionsForBinary->bOverwrite)
	{
		int bExists = (VSIStat(psOptionsForBinary->pszDstFilename, &sBuf) == 0);
		if (bExists)
		{
			GDALDriverH hDriver = GDALIdentifyDriver(psOptionsForBinary->pszDstFilename, NULL);
			if (hDriver && !(EQUAL(GDALGetDriverShortName(hDriver), "VRT") ||
				(EQUAL(GDALGetDriverShortName(hDriver), "API_PROXY") &&
					EQUAL(CPLGetExtension(psOptionsForBinary->pszDstFilename), "VRT"))))
			{
				return eParameterErr;
			}
		}
	}

	int bUsageError = FALSE;
	GDALDatasetH hOutDS = GDALBuildVRT(psOptionsForBinary->pszDstFilename,
		psOptionsForBinary->nSrcFiles,
		NULL,
		(const char* const*)psOptionsForBinary->papszSrcFiles,
		psOptions, &bUsageError);
	if (bUsageError)
		return eOther;
	int nRetCode = (hOutDS) ? 0 : 1;

	GDALBuildVRTOptionsFree(psOptions);
	GDALBuildVRTOptionsForBinaryFree(psOptionsForBinary);

	CPLErrorReset();
	// The flush to disk is only done at that stage, so check if any error has
	// happened
	GDALClose(hOutDS);
	if (CPLGetLastErrorType() != CE_None)
		nRetCode = 1;

	return eOK;
}

ipfGdalProgressTools::errType ipfGdalProgressTools::ipfGDALlocationinfo(const QString &str, int &iRow, int &iCol)
{
	double dfGeoX = 0.0;
	double dfGeoY = 0.0;

	/* -------------------------------------------------------------------- */
	/*      解析参数.                                                       */
	/* -------------------------------------------------------------------- */
	QStringList list = str.split(' ', QString::SkipEmptyParts);
	int argc = list.size();
	if (argc != 4)
		return eParameterErr;

	std::string str1 = list.at(0).toStdString();
	const char *pszSrcFilename = str1.c_str();

	const char *pszSourceSRS;
	std::string str2 = list.at(1).toStdString();
	if (list.at(1) != "-geoloc")
		pszSourceSRS = SanitizeSRS(str2.c_str());
	else
		pszSourceSRS = str2.c_str();

	std::string str3 = list.at(2).toStdString();
	const char *pszLocX = str3.c_str();
	std::string str4 = list.at(3).toStdString();
	const char *pszLocY = str4.c_str();

	if (pszSrcFilename == NULL || pszLocX == NULL || pszLocY == NULL)
		return eParameterNull;
	dfGeoX = CPLAtof(pszLocX);
	dfGeoY = CPLAtof(pszLocY);

	/* -------------------------------------------------------------------- */
	/*      打开数据源.                                                     */
	/* -------------------------------------------------------------------- */
	GDALDatasetH hSrcDS = GDALOpenEx(pszSrcFilename, GDAL_OF_RASTER, NULL, NULL, NULL);
	if (hSrcDS == NULL)
		return eSourceOpenErr;

	/* -------------------------------------------------------------------- */
	/*      如果需要的话，设置坐标转换                                       */
	/* -------------------------------------------------------------------- */
	OGRSpatialReferenceH hSrcSRS = NULL, hTrgSRS = NULL;
	OGRCoordinateTransformationH hCT = NULL;
	if (pszSourceSRS != NULL && !EQUAL(pszSourceSRS, "-geoloc"))
	{
		hSrcSRS = OSRNewSpatialReference(pszSourceSRS);
		hTrgSRS = OSRNewSpatialReference(GDALGetProjectionRef(hSrcDS));

		hCT = OCTNewCoordinateTransformation(hSrcSRS, hTrgSRS);
		if (hCT == NULL)
		{
			GDALClose(hSrcDS);
			return eTransformErr;
		}
	}

	/* -------------------------------------------------------------------- */
	/*      Turn the location into a pixel and line location.               */
	/* -------------------------------------------------------------------- */
	int iPixel, iLine;
	if (hCT)
	{
		if (!OCTTransform(hCT, 1, &dfGeoX, &dfGeoY, NULL))
			return eTransformErr;
	}

	if (pszSourceSRS != NULL)
	{
		double adfGeoTransform[6], adfInvGeoTransform[6];

		if (GDALGetGeoTransform(hSrcDS, adfGeoTransform) != CE_None)
			return eTransformErr;

		if (!GDALInvGeoTransform(adfGeoTransform, adfInvGeoTransform))
			return eTransformErr;

		iPixel = (int)floor(
			adfInvGeoTransform[0]
			+ adfInvGeoTransform[1] * dfGeoX
			+ adfInvGeoTransform[2] * dfGeoY);
		iLine = (int)floor(
			adfInvGeoTransform[3]
			+ adfInvGeoTransform[4] * dfGeoX
			+ adfInvGeoTransform[5] * dfGeoY);
	}
	else
	{
		iPixel = (int)floor(dfGeoX);
		iLine = (int)floor(dfGeoY);
	}

	/* -------------------------------------------------------------------- */
	/*      Prepare report.                                                 */
	/* -------------------------------------------------------------------- */
	if (iPixel < 0 || iLine < 0
		|| iPixel >= GDALGetRasterXSize(hSrcDS)
		|| iLine >= GDALGetRasterYSize(hSrcDS))
	{
		/* -------------------------------------------------------------------- */
		/*      Cleanup                                                         */
		/* -------------------------------------------------------------------- */
		if (hCT) {
			OSRDestroySpatialReference(hSrcSRS);
			OSRDestroySpatialReference(hTrgSRS);
			OCTDestroyCoordinateTransformation(hCT);
		}

		if (hSrcDS)
			GDALClose(hSrcDS);

		return eRowColErr;
	}
	else
	{
		iRow = iLine;
		iCol = iPixel;
	}

	/* -------------------------------------------------------------------- */
	/*      Cleanup                                                         */
	/* -------------------------------------------------------------------- */
	if (hCT) {
		OSRDestroySpatialReference(hSrcSRS);
		OSRDestroySpatialReference(hTrgSRS);
		OCTDestroyCoordinateTransformation(hCT);
	}

	if (hSrcDS)
		GDALClose(hSrcDS);

	return eOK;
}

ipfGdalProgressTools::errType ipfGdalProgressTools::ipfGDALOgrToOgr(const QString & str)
{
	QStringList list = str.split('@', QString::SkipEmptyParts);
	int argc = list.size();
	if (argc == 0)
		return eParameterNull;

	char **argv = (char **)malloc(sizeof(char*)*argc);
	for (int var = 0; var < argc; ++var)
	{
		std::string str = list.at(var).toStdString();
		const char* p = str.c_str();
		size_t cSize = strlen(p);
		char *c = (char*)malloc(sizeof(char*)*cSize);
		strncpy(c, p, cSize);
		c[cSize] = '\0';
		argv[var] = c;
	}

	EarlySetConfigOptions(argc, argv);
	argc = OGRGeneralCmdLineProcessor(argc, &argv, 0);
	if (argc < 1)
		return eParameterErr;


	GDALVectorTranslateOptionsForBinary* psOptionsForBinary = GDALVectorTranslateOptionsForBinaryNew();
	GDALVectorTranslateOptions* psOptions = GDALVectorTranslateOptionsNew(argv + 1, psOptionsForBinary);

	if (psOptions == NULL)
		return eParameterNull;

	if (psOptionsForBinary->pszDataSource == NULL ||
		psOptionsForBinary->pszDestDataSource == NULL)
	{
		errType err;
		if (psOptionsForBinary->pszDestDataSource == NULL)
			err = eDestParameterNull;
		else
			err = eSourceParameterNull;
		GDALVectorTranslateOptionsFree(psOptions);
		GDALVectorTranslateOptionsForBinaryFree(psOptionsForBinary);
		return err;
	}

	if (strcmp(psOptionsForBinary->pszDestDataSource, "/vsistdout/") == 0)
		psOptionsForBinary->bQuiet = TRUE;

	if (!psOptionsForBinary->bQuiet && !psOptionsForBinary->bFormatExplicitlySet &&
		psOptionsForBinary->eAccessMode == ACCESS_CREATION)
	{
		CheckDestDataSourceNameConsistency(psOptionsForBinary->pszDestDataSource,
			psOptionsForBinary->pszFormat);
	}

	/* -------------------------------------------------------------------- */
	/*      Open data source.                                               */
	/* -------------------------------------------------------------------- */

	/* Avoid opening twice the same datasource if it is both the input and output */
	/* Known to cause problems with at least FGdb, SQlite and GPKG drivers. See #4270 */
	GDALDatasetH hDS = NULL;
	GDALDatasetH hODS = NULL;
	int bCloseODS = TRUE;
	int bUsageError = FALSE;
	GDALDatasetH hDstDS;
	int nRetCode = 1;

	if (psOptionsForBinary->eAccessMode != ACCESS_CREATION &&
		strcmp(psOptionsForBinary->pszDestDataSource, psOptionsForBinary->pszDataSource) == 0)
	{
		hODS = GDALOpenEx(psOptionsForBinary->pszDataSource,
			GDAL_OF_UPDATE | GDAL_OF_VECTOR, NULL, psOptionsForBinary->papszOpenOptions, NULL);
		GDALDriverH hDriver = NULL;
		if (hODS != NULL)
			hDriver = GDALGetDatasetDriver(hODS);

		/* Restrict to those 3 drivers. For example it is known to break with */
		/* the PG driver due to the way it manages transactions... */
		if (hDriver && !(EQUAL(GDALGetDescription(hDriver), "FileGDB") ||
			EQUAL(GDALGetDescription(hDriver), "SQLite") ||
			EQUAL(GDALGetDescription(hDriver), "GPKG")))
		{
			hDS = GDALOpenEx(psOptionsForBinary->pszDataSource,
				GDAL_OF_VECTOR, NULL, psOptionsForBinary->papszOpenOptions, NULL);
		}
		else
		{
			hDS = hODS;
			bCloseODS = FALSE;
		}
	}
	else
	{
		hDS = GDALOpenEx(psOptionsForBinary->pszDataSource,
			GDAL_OF_VECTOR, NULL, psOptionsForBinary->papszOpenOptions, NULL);
	}

	if (hDS == NULL)
	{
		GDALVectorTranslateOptionsFree(psOptions);
		GDALVectorTranslateOptionsForBinaryFree(psOptionsForBinary);
		return eSourceOpenErr;
	}

	if (hODS != NULL)
	{
		GDALDriverManager *poDM = GetGDALDriverManager();

		GDALDriver* poDriver = poDM->GetDriverByName(psOptionsForBinary->pszFormat);
		if (poDriver == NULL)
		{
			GDALVectorTranslateOptionsFree(psOptions);
			GDALVectorTranslateOptionsForBinaryFree(psOptionsForBinary);
			return eSourceOpenErr;
		}
	}

	if (!(psOptionsForBinary->bQuiet))
	{
		GDALVectorTranslateOptionsSetProgress(psOptions, ALGTermProgress, proDialog);
	}

	hDstDS = GDALVectorTranslate(psOptionsForBinary->pszDestDataSource, hODS,
		1, &hDS, psOptions, &bUsageError);
	if (bUsageError)
		return eOther;
	else
		nRetCode = (hDstDS) ? 0 : 1;

	GDALVectorTranslateOptionsFree(psOptions);
	GDALVectorTranslateOptionsForBinaryFree(psOptionsForBinary);

	if (hDS)
		GDALClose(hDS);
	if (bCloseODS)
		GDALClose(hDstDS);

	return eOK;
}

ipfGdalProgressTools::errType ipfGdalProgressTools::ipfGDALrasterize(const QString & str)
{
	QStringList list = str.split(' ', QString::SkipEmptyParts);
	int argc = list.size();
	if (argc == 0)
		return eParameterNull;

	char **argv = (char **)malloc(sizeof(char*)*argc);
	for (int var = 0; var < argc; ++var)
	{
		std::string str = list.at(var).toStdString();
		const char* p = str.c_str();
		size_t cSize = strlen(p);
		char *c = (char*)malloc(sizeof(char*)*cSize);
		strncpy(c, p, cSize);
		c[cSize] = '\0';
		argv[var] = c;
	}

	// 提前设置配置信息
	EarlySetConfigOptions(argc, argv);

	//GDAL通用命令行处理
	argc = GDALGeneralCmdLineProcessor(argc, &argv, 0);
	if (argc < 1)
		return eParameterErr;

	GDALRasterizeOptionsForBinary* psOptionsForBinary = GDALRasterizeOptionsForBinaryNew();
	GDALRasterizeOptions *psOptions = GDALRasterizeOptionsNew(argv, psOptionsForBinary);
	CSLDestroy(argv);

	if (psOptions == NULL)
		return eParameterNull;

	if (psOptionsForBinary->pszSource == NULL)
		return eSourceParameterNull;

	if (psOptionsForBinary->pszDest == NULL)
		return eSourceParameterNull;

	if (!(psOptionsForBinary->bQuiet))
	{
		GDALRasterizeOptionsSetProgress(psOptions, ALGTermProgress, proDialog);
	}

	/* -------------------------------------------------------------------- */
	/*      Open input file.                                                */
	/* -------------------------------------------------------------------- */
	GDALDatasetH hInDS = GDALOpenEx(
		psOptionsForBinary->pszSource, GDAL_OF_VECTOR | GDAL_OF_VERBOSE_ERROR,
		NULL, NULL, NULL);

	if (hInDS == NULL)
		return eSourceOpenErr;

	/* -------------------------------------------------------------------- */
	/*      Open output file if it exists.                                  */
	/* -------------------------------------------------------------------- */
	GDALDatasetH hDstDS = NULL;
	if (!(psOptionsForBinary->bCreateOutput))
	{
		CPLPushErrorHandler(CPLQuietErrorHandler);
		hDstDS = GDALOpenEx(
			psOptionsForBinary->pszDest,
			GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR | GDAL_OF_UPDATE,
			NULL, NULL, NULL);
		CPLPopErrorHandler();
	}

	if (psOptionsForBinary->bCreateOutput || hDstDS == NULL)
	{
		GDALDriverManager *poDM = GetGDALDriverManager();
		GDALDriver *poDriver = poDM->GetDriverByName(psOptionsForBinary->pszFormat);
		char** papszDriverMD = (poDriver) ? poDriver->GetMetadata() : NULL;
		if (poDriver == NULL ||
			!CPLTestBool(CSLFetchNameValueDef(papszDriverMD, GDAL_DCAP_RASTER,
				"FALSE")) ||
			!CPLTestBool(CSLFetchNameValueDef(papszDriverMD, GDAL_DCAP_CREATE,
				"FALSE")))
		{
			fprintf(stderr,
				"Output driver `%s' not recognised or does not support "
				"direct output file creation.\n",
				psOptionsForBinary->pszFormat);
			fprintf(stderr,
				"The following format drivers are configured and "
				"support direct output:\n");

			for (int iDriver = 0; iDriver < poDM->GetDriverCount(); iDriver++)
			{
				GDALDriver* poIter = poDM->GetDriver(iDriver);
				papszDriverMD = poIter->GetMetadata();
				if (CPLTestBool( CSLFetchNameValueDef(papszDriverMD, GDAL_DCAP_RASTER, "FALSE")) &&
					CPLTestBool( CSLFetchNameValueDef(papszDriverMD, GDAL_DCAP_CREATE, "FALSE")))
				{
					fprintf(stderr, "  -> `%s'\n", poIter->GetDescription());
				}
			}
			exit(1);
		}
	}

	if (hDstDS == NULL && !psOptionsForBinary->bQuiet &&
		!psOptionsForBinary->bFormatExplicitlySet)
		CheckExtensionConsistency(psOptionsForBinary->pszDest,
			psOptionsForBinary->pszFormat);

	int bUsageError = FALSE;
	GDALDatasetH hRetDS = GDALRasterize(psOptionsForBinary->pszDest, hDstDS, hInDS, psOptions, &bUsageError);
	if (bUsageError == TRUE)
		return eOther;

	GDALClose(hInDS);
	GDALClose(hRetDS);
	GDALRasterizeOptionsFree(psOptions);
	GDALRasterizeOptionsForBinaryFree(psOptionsForBinary);

	return eOK;
}

QString ipfGdalProgressTools::pixelDecimal(const QString &source, const QString &target, const int decimal)
{
	GDALDataset* poDataset_source = nullptr;
	GDALDataset *poDataset_target = nullptr;
	GDALDriver *poDriver = nullptr;

    // 尝试打开数据源
	poDataset_source = (GDALDataset*)GDALOpenEx(source.toStdString().c_str(), GDAL_OF_RASTER, NULL, NULL, NULL);

	if (!poDataset_source)
		return enumErrTypeToString(eSourceOpenErr);

	if (poDataset_source->GetRasterCount() != 1)
		return QStringLiteral("该功能主要针对单波段数据进行处理，该数据波段数量不符。");

	GDALDataType type = poDataset_source->GetRasterBand(1)->GetRasterDataType();
	if (!(type == GDT_Int16 || type == GDT_Float32 || type == GDT_Float64))
	{
		GDALClose((GDALDatasetH)poDataset_source);
		return QStringLiteral("不受支持的数据类型: ") + type;
	}

	double adfGeoTransform[6];
	poDataset_source->GetGeoTransform(adfGeoTransform);
	int nXSize = poDataset_source->GetRasterXSize();
	int nYSize = poDataset_source->GetRasterYSize();
	int nBands = poDataset_source->GetRasterCount();

	QFileInfo info(target);
	poDriver = GetGDALDriverManager()->GetDriverByName(enumFormatToString(info.suffix()).toStdString().c_str());
	char **papszMetadata = poDriver->GetMetadata();
	poDataset_target = poDriver->Create(target.toStdString().c_str(), nXSize, nYSize, 0, type, papszMetadata);
	if (poDataset_target == NULL)
		return enumErrTypeToString(eNotCreateDest);
	poDataset_target->SetGeoTransform(adfGeoTransform);
	poDataset_target->SetProjection(poDataset_source->GetProjectionRef());

	// 项vrt中注册算法
	char** options = NULL;
	options = CSLAddNameValue(options, "subclass", "VRTDerivedRasterBand");
	options = CSLAddNameValue(options, "PixelFunctionType", "pixelDecimalFunction");
	poDataset_target->AddBand(type, options);
	CSLDestroy(options);
	GDALRasterBand* new_band = poDataset_target->GetRasterBand(poDataset_target->GetRasterCount());
	VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band), poDataset_source->GetRasterBand(poDataset_source->GetRasterCount()),
		0, 0, nXSize, nYSize,
		0, 0, nXSize, nYSize,
		"near", VRT_NODATA_UNSET);

	QString sDec = "1";
	for (int i = 0; i < decimal; ++i)
		sDec.append("0");
	IPF_DECIMAL = sDec.toInt();

	GDALClose((GDALDatasetH)poDataset_target);
	GDALClose((GDALDatasetH)poDataset_source);
	return enumErrTypeToString(eOK);
}

QString ipfGdalProgressTools::extractRasterRange(const QString & source, const QString & target, const double background)
{
	ipfOGR ogr(source);
	if (!ogr.isOpen())
		return enumErrTypeToString(eSourceOpenErr);

	int nBands = ogr.getBandSize();

	if (nBands == 0)
		return QStringLiteral(": 栅格数据缺少有效波段，无法继续。");

	GDALDataType type = ogr.getDataType_y();
	IPF_BACKGROUND = background;

	QList<int> xySize = ogr.getYXSize();
	int nXSize = xySize.at(1);
	int nYSize = xySize.at(0);

	// 创建新栅格
	GDALDataset *poDataset_target = ogr.createNewRaster(target, 0);
	
	// 设置NODATA
	IPF_NODATA = ogr.getNodataValue(1);

	// 项vrt中注册算法
	char** options = NULL;
	options = CSLAddNameValue(options, "subclass", "VRTDerivedRasterBand");
	options = CSLAddNameValue(options, "PixelFunctionType", "pixelModifyUnBackGroundFunction");
	poDataset_target->AddBand(type, options);
	CSLDestroy(options);
	GDALRasterBand* new_band = poDataset_target->GetRasterBand(1);
	VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band),
		ogr.getRasterBand(1),
		0, 0, nXSize, nYSize,
		0, 0, nXSize, nYSize,
		"near", IPF_NODATA);

	// 设置NODATA
	poDataset_target->GetRasterBand(1)->SetNoDataValue(IPF_NODATA);

	GDALClose((GDALDatasetH)poDataset_target);
	return enumErrTypeToString(eOK);
}

QString ipfGdalProgressTools::filterInvalidValue(const QString & source, const QString & target
	, const QString invalidString, const bool isNegative, const bool isNodata)
{
	ipfOGR ogr(source);
	if (!ogr.isOpen())
		return enumErrTypeToString(eSourceOpenErr);

	// 初始化 pixelInvalidValue 所需参数
	IPF_ISNEGATIVE = isNegative;
	IPF_ISNODATA = isNodata;
	
	// 分割无效值
	QStringList valueList;
	IPF_INVALIDVALUE.clear();
	if (!invalidString.isEmpty())
		valueList = invalidString.split('@', QString::SkipEmptyParts);
	foreach (QString str, valueList)
	{
		IPF_INVALIDVALUE.append(str.toDouble());
	}

	GDALDataType type = ogr.getDataType_y();
	QList<int> xySize = ogr.getYXSize();
	int nXSize = xySize.at(1);
	int nYSize = xySize.at(0);

	// 创建新栅格
	GDALDataset *poDataset_target = ogr.createNewRaster(target, 0);

	// 向vrt注册自定义算法
	char** options = NULL;
	options = CSLAddNameValue(options, "subclass", "VRTDerivedRasterBand");

	// 向vrt添加波段
	options = CSLAddNameValue(options, "band", "1");
	options = CSLAddNameValue(options, "PixelFunctionType", "pixelInvalidValue");
	poDataset_target->AddBand(type, options); // 由于只用0和1区分，故只用8bit节省空间。
	CSLDestroy(options);

	// 创建新波段
	GDALRasterBand* new_band = poDataset_target->GetRasterBand(1);

	int nBands = ogr.getBandSize();
	for (int i = 1; i <= nBands; ++i)
	{
		// 分别保存每个波段的NODATA值
		IPF_BANSNODATA.append( ogr.getNodataValue(i) );

		VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band),
			ogr.getRasterBand(i),
			0, 0, nXSize, nYSize,
			0, 0, nXSize, nYSize,
			NULL, 0);
	}

	// 设置NODATA为
	poDataset_target->GetRasterBand(1)->SetNoDataValue(255);

	GDALClose((GDALDatasetH)poDataset_target);
	return enumErrTypeToString(eOK);
}

QString ipfGdalProgressTools::slopCalculation_S2(const QString & source, const QString & target)
{
	ipfOGR ogr(source);
	if (!ogr.isOpen())
		return enumErrTypeToString(eSourceOpenErr);

	int nBands = ogr.getBandSize();

	if (nBands != 4)
		return QStringLiteral("波段数量不符。");

	QList<int> xySize = ogr.getYXSize();
	int nXSize = xySize.at(1);
	int nYSize = xySize.at(0);

	// 创建新栅格
	GDALDataset *poDataset_target = ogr.createNewRaster(target, 0);

	// 设置NODATA
	IPF_NODATA = ogr.getNodataValue(1);

	// 向vrt注册自定义算法
	char** options = NULL;
	options = CSLAddNameValue(options, "subclass", "VRTDerivedRasterBand");
	
	// 向vrt添加波段
	options = CSLAddNameValue(options, "band", "1");
	options = CSLAddNameValue(options, "PixelFunctionType", "pixelSlopFunction_S2");
	poDataset_target->AddBand(GDT_Float64, options);

	CSLDestroy(options);

	// 创建新栅格
	GDALRasterBand* new_band = poDataset_target->GetRasterBand(1);
	
	VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band),
		ogr.getRasterBand(1),
		0, 0, nXSize, nYSize,
		0, 0, nXSize, nYSize,
		NULL, 0);
	VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band),
		ogr.getRasterBand(2),
		0, 0, nXSize, nYSize,
		0, 0, nXSize, nYSize,
		NULL, 0);
	VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band),
		ogr.getRasterBand(3),
		0, 0, nXSize, nYSize,
		0, 0, nXSize, nYSize,
		NULL, 0);
	VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band),
		ogr.getRasterBand(4),
		0, 0, nXSize, nYSize,
		0, 0, nXSize, nYSize,
		NULL, 0);

	// 设置NODATA
	poDataset_target->GetRasterBand(1)->SetNoDataValue(IPF_NODATA);

	GDALClose((GDALDatasetH)poDataset_target);
	return enumErrTypeToString(eOK);
}

QString ipfGdalProgressTools::dsmdemDiffeProcess(const QString & dsm, const QString & dem
	, const QString &outRaster, const QString & type
	, const double threshold, const bool isFillNodata)
{
	ipfOGR ogr_dsm(dsm);
	ipfOGR ogr_dem(dem);
	if (!ogr_dsm.isOpen() || !ogr_dem.isOpen())
		return enumErrTypeToString(eSourceOpenErr);

	if (ogr_dsm.getBandSize() != 1 || ogr_dem.getBandSize() != 1)
		return QStringLiteral("DSM或DEM波段数量不正确。");

	IPF_DSMDEM_TYPE = type;
	IPF_THRESHOLD = threshold;
	IPF_FILLNODATA = isFillNodata;

	ipfOGR *ogr = nullptr;
	GDALRasterBand* band_1 = nullptr;
	GDALRasterBand* band_2 = nullptr;
	if (type == "DSM")
	{
		ogr = &ogr_dsm;
		band_1 = ogr_dsm.getRasterBand(1);
		band_2 = ogr_dem.getRasterBand(1);
	}
	else if (type == "DEM")
	{
		ogr = &ogr_dem;
		band_1 = ogr_dem.getRasterBand(1);
		band_2 = ogr_dsm.getRasterBand(1);
	}

	// 获取行列数
	QList<int> xySize_dsm = ogr_dsm.getYXSize();
	int nXSize_dsm = xySize_dsm.at(1);
	int nYSize_dsm = xySize_dsm.at(0);
	QList<int> xySize_dem = ogr_dem.getYXSize();
	int nXSize_dem = xySize_dem.at(1);
	int nYSize_dem = xySize_dem.at(0);

	// 计算相交矩形
	QgsRectangle rect_dsm = ogr_dsm.getXY();
	QgsRectangle rect_dem = ogr_dem.getXY();
	QgsRectangle box = rect_dsm.intersect(&rect_dem);

	// 计算在两个栅格中的像素行列位置
	int iRow_dsm = 0;
	int iCol_dsm = 0;
	int iRow_dem = 0;
	int iCol_dem = 0;

	if (!ogr_dsm.Projection2ImageRowCol(box.xMinimum(), box.yMaximum(), iCol_dsm, iRow_dsm))
		return dsm + QStringLiteral(": 计算DSM行列位置失败，请检查投影信息等是否定义正确。");
	if (!ogr_dem.Projection2ImageRowCol(box.xMinimum(), box.yMaximum(), iCol_dem, iRow_dem))
		return dem + QStringLiteral(": 计算DEM行列位置失败，请检查投影信息等是否定义正确。");

	// 创建新栅格
	GDALDataset *poDataset_target = ogr->createNewRaster(outRaster, 0);

	// 设置NODATA
	IPF_DSM_NODATA = ogr_dsm.getNodataValue(1);
	IPF_DEM_NODATA = ogr_dem.getNodataValue(1);

	// 向vrt注册自定义算法
	char** options = NULL;
	options = CSLAddNameValue(options, "subclass", "VRTDerivedRasterBand");

	// 向vrt添加波段
	options = CSLAddNameValue(options, "band", "1");
	options = CSLAddNameValue(options, "PixelFunctionType", "pixelDSMDEMDiffProcessFunction");
	poDataset_target->AddBand(ogr->getDataType_y(), options);

	CSLDestroy(options);

	// 创建新栅格
	GDALRasterBand* new_band = poDataset_target->GetRasterBand(1);
	VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band),
		band_1,
		0, 0, nXSize_dsm, nYSize_dsm,
		0, 0, nXSize_dsm, nYSize_dsm,
		NULL, -9999);
	VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band),
		band_2,
		iCol_dem, iRow_dem, nXSize_dem, nYSize_dem,
		iCol_dsm, iRow_dsm, nXSize_dem, nYSize_dem,
		NULL, -9999);

	// 设置NODATA
	poDataset_target->GetRasterBand(1)->SetNoDataValue(ogr->getNodataValue(1));

	GDALClose((GDALDatasetH)poDataset_target);
	return enumErrTypeToString(eOK);
}

QString ipfGdalProgressTools::pixelModifyValue(const QString &source, const QString &target
	, const double valueOld_1, const double valueNew_1
	, const double valueOld_2, const double valueNew_2)
{
	GDALDataset* poDataset_source = nullptr;
	GDALDataset *poDataset_target = nullptr;
	GDALDriver *poDriver = nullptr;

	IPF_VALUE_OLD_1 = valueOld_1;
	IPF_VALUE_NEW_1 = valueNew_1;
	IPF_VALUE_OLD_2 = valueOld_2;
	IPF_VALUE_NEW_2 = valueNew_2;

	// 尝试打开数据源
	poDataset_source = (GDALDataset*)GDALOpenEx(source.toStdString().c_str(), GDAL_OF_RASTER, NULL, NULL, NULL);

	if (!poDataset_source)
		return enumErrTypeToString(eSourceOpenErr);

	GDALDataType type = poDataset_source->GetRasterBand(1)->GetRasterDataType();

	double adfGeoTransform[6];
	poDataset_source->GetGeoTransform(adfGeoTransform);
	int nXSize = poDataset_source->GetRasterXSize();
	int nYSize = poDataset_source->GetRasterYSize();
	int nBands = poDataset_source->GetRasterCount();

	QFileInfo info(target);
	poDriver = GetGDALDriverManager()->GetDriverByName(enumFormatToString(info.suffix()).toStdString().c_str());
	char **papszMetadata = poDriver->GetMetadata();
	poDataset_target = poDriver->Create(target.toStdString().c_str(), nXSize, nYSize, 0, type, papszMetadata);
	if (poDataset_target == NULL)
		return enumErrTypeToString(eNotCreateDest);
	poDataset_target->SetGeoTransform(adfGeoTransform);
	poDataset_target->SetProjection(poDataset_source->GetProjectionRef());

	// 给vrt中每个波段注册算法
	for (int i = 0; i < nBands; ++i)
	{
		char** options = NULL;
		options = CSLAddNameValue(options, "band", QString::number(i+1).toStdString().c_str()); 
		options = CSLAddNameValue(options, "subclass", "VRTDerivedRasterBand");
		options = CSLAddNameValue(options, "PixelFunctionType", "pixelModifyValueFunction");
		poDataset_target->AddBand(type, options);
		CSLDestroy(options);
	}

	// 添加波段
	for (int i=0; i < nBands; ++i)
	{
		GDALRasterBand* new_band = poDataset_target->GetRasterBand(i + 1);
		VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band), poDataset_source->GetRasterBand(i + 1),
			0, 0, nXSize, nYSize,
			0, 0, nXSize, nYSize,
			"near", VRT_NODATA_UNSET);
	}

	GDALClose((GDALDatasetH)poDataset_target);
	GDALClose((GDALDatasetH)poDataset_source);
	return enumErrTypeToString(eOK);
}

QString ipfGdalProgressTools::pixelFillValue(const QString & source, const QString & target, const double nodata, const double value)
{
	IPF_RANGE_NODATA = nodata;
	IPF_RANGE_VALUE = value;

	ipfOGR ogr(source);
	if (!ogr.isOpen())
		return enumErrTypeToString(eSourceOpenErr);

	int nBands = ogr.getBandSize();
	if (nBands != 1)
		return QStringLiteral("该功能主要针对高程模型，目前暂时只支持单波段数据");

	QList<int> xySize = ogr.getYXSize();
	int nXSize = xySize.at(1);
	int nYSize = xySize.at(0);
	
	// 创建新栅格
	GDALDataset *poDataset_target = ogr.createNewRaster(target, 0);

	// 给波段注册算法
	char** options = NULL;
	options = CSLAddNameValue(options, "band", "1");
	options = CSLAddNameValue(options, "subclass", "VRTDerivedRasterBand");
	options = CSLAddNameValue(options, "PixelFunctionType", "pixelFillValueFunction");
	poDataset_target->AddBand(ogr.getDataType_y(), options);
	CSLDestroy(options);

	// 添加波段
	GDALRasterBand* new_band = poDataset_target->GetRasterBand(1);
	VRTAddSimpleSource(static_cast<VRTSourcedRasterBandH>(new_band), ogr.getRasterBand(1),
		0, 0, nXSize, nYSize,
		0, 0, nXSize, nYSize,
		"near", IPF_RANGE_NODATA);

	// 设置NODATA为
	poDataset_target->GetRasterBand(1)->SetNoDataValue(IPF_RANGE_NODATA);
	GDALClose((GDALDatasetH)poDataset_target);
	return enumErrTypeToString(eOK);
}

int ipfGdalProgressTools::QStringToChar(const QString& str, char ***argv)
{
    int doneSize = 0;
    QStringList list = str.split(' ', QString::SkipEmptyParts);
    doneSize = list.size();
    if ( doneSize==0 )
        return doneSize;

    char **mArgv= *argv;
    for (int var = 0; var < doneSize; ++var)
    {
        std::string str = list.at(var).toStdString();
        const char* p = str.c_str();
        size_t cSize = strlen(p);
        char *c = new char[cSize];
        strncpy(c, p, cSize);
        c[cSize] = '\0';
        mArgv[var] = c;
    }
    return doneSize;
}

ipfGdalProgressTools::errType ipfGdalProgressTools::GDALGeneric3x3Processing(
	GDALRasterBandH hSrcBand, GDALRasterBandH hDstBand,
	GDALGeneric3x3ProcessingAlg pfnAlg, void* pData,
	GDALProgressFunc pfnProgress, void * pProgressData)
{
	CPLErr eErr;
	float *pafThreeLineWin;  /* 输入图像三行数据存储空间 */
	float *pafOutputBuf;     /* 输出图像一行数据存储空间 */
	int i, j;

	int bSrcHasNoData, bDstHasNoData;
	float fSrcNoDataValue = 0.0, fDstNoDataValue = 0.0;

	int nXSize = GDALGetRasterBandXSize(hSrcBand);
	int nYSize = GDALGetRasterBandYSize(hSrcBand);

	if (pfnProgress == NULL)
		pfnProgress = GDALDummyProgress;

	//初始化进度条计数器
	if (!pfnProgress(0.0, NULL, pProgressData))
		return eUserTerminated;

	//分配内存空间
	pafOutputBuf = (float *)CPLMalloc(sizeof(float)*nXSize);
	pafThreeLineWin = (float *)CPLMalloc(3 * sizeof(float)*(nXSize + 1));

	fSrcNoDataValue = (float)GDALGetRasterNoDataValue(hSrcBand, &bSrcHasNoData);
	fDstNoDataValue = (float)GDALGetRasterNoDataValue(hDstBand, &bDstHasNoData);
	if (!bDstHasNoData)
		fDstNoDataValue = 0.0;

	// 移动一个3x3的窗口pafWindow遍历每个象元
	// (中心象元编号为#4)
	// 
	//      0 1 2
	//      3 4 5
	//      6 7 8

	// 预先加载前两行数据
	for (i = 0; i < 2 && i < nYSize; i++)
	{
		GDALRasterIO(hSrcBand, GF_Read, 0, i, nXSize, 1,
			pafThreeLineWin + i * nXSize, nXSize, 1,
			GDT_Float32, 0, 0);
	}

	//不包括边界
	for (j = 0; j < nXSize; j++)
		pafOutputBuf[j] = fDstNoDataValue;

	GDALRasterIO(hDstBand, GF_Write, 0, 0, nXSize, 1,
		pafOutputBuf, nXSize, 1, GDT_Float32, 0, 0);

	if (nYSize > 1)
	{
		GDALRasterIO(hDstBand, GF_Write, 0, nYSize - 1, nXSize, 1,
			pafOutputBuf, nXSize, 1, GDT_Float32, 0, 0);
	}

	int nLine1Off = 0 * nXSize;
	int nLine2Off = 1 * nXSize;
	int nLine3Off = 2 * nXSize;

	for (i = 1; i < nYSize - 1; i++)
	{
		// 读取第三行数据
		eErr = GDALRasterIO(hSrcBand, GF_Read, 0, i + 1, nXSize, 1,
			pafThreeLineWin + nLine3Off, nXSize, 1, GDT_Float32, 0, 0);

		if (eErr != CE_None)
			goto end;

		//不包括边界数据
		pafOutputBuf[0] = fDstNoDataValue;
		if (nXSize > 1)
			pafOutputBuf[nXSize - 1] = fDstNoDataValue;

		// 这句使用OpenMP来加速
#pragma omp parallel for
		for (j = 1; j < nXSize - 1; j++)
		{
			float afWin[9];
			afWin[0] = pafThreeLineWin[nLine1Off + j - 1];
			afWin[1] = pafThreeLineWin[nLine1Off + j];
			afWin[2] = pafThreeLineWin[nLine1Off + j + 1];
			afWin[3] = pafThreeLineWin[nLine2Off + j - 1];
			afWin[4] = pafThreeLineWin[nLine2Off + j];
			afWin[5] = pafThreeLineWin[nLine2Off + j + 1];
			afWin[6] = pafThreeLineWin[nLine3Off + j - 1];
			afWin[7] = pafThreeLineWin[nLine3Off + j];
			afWin[8] = pafThreeLineWin[nLine3Off + j + 1];

			if (bSrcHasNoData && (
				afWin[0] == fSrcNoDataValue ||
				afWin[1] == fSrcNoDataValue ||
				afWin[2] == fSrcNoDataValue ||
				afWin[3] == fSrcNoDataValue ||
				afWin[4] == fSrcNoDataValue ||
				afWin[5] == fSrcNoDataValue ||
				afWin[6] == fSrcNoDataValue ||
				afWin[7] == fSrcNoDataValue ||
				afWin[8] == fSrcNoDataValue))
			{
				// 如果9个数据中有一个是NoData则将当前点设置为NoData
				pafOutputBuf[j] = fDstNoDataValue;
			}
			else
			{
				// 一个合格的3*3窗口
				pafOutputBuf[j] = pfnAlg(afWin, fDstNoDataValue, pData);
			}
		}

		//写入一行数据
		eErr = GDALRasterIO(hDstBand, GF_Write, 0, i, nXSize, 1,
			pafOutputBuf, nXSize, 1, GDT_Float32, 0, 0);

		if (eErr != CE_None)
			goto end;

		if (!pfnProgress(1.0 * (i + 1) / nYSize, NULL, pProgressData))
		{
			eErr = CE_Failure;
			goto end;
		}

		int nTemp = nLine1Off;
		nLine1Off = nLine2Off;
		nLine2Off = nLine3Off;
		nLine3Off = nTemp;
	}

	pfnProgress(1.0, NULL, pProgressData);
	eErr = CE_None;

end:
	CPLFree(pafOutputBuf);
	CPLFree(pafThreeLineWin);

	if (eErr == CE_None)
		return eOK;
	else
		return eOther;
}

QString ipfGdalProgressTools::buildOverviews(const QString & source)
{
	// 尝试打开数据源
	GDALDatasetH hDataset;
	hDataset = GDALOpenEx(source.toStdString().c_str(), GDAL_OF_RASTER, NULL, NULL, NULL);
	if (!hDataset)
		return enumErrTypeToString(eSourceOpenErr);

	int iWidth = GDALGetRasterXSize(hDataset);
	int iHeigh = GDALGetRasterYSize(hDataset);

	int iPixelNum = iWidth * iHeigh;    //图像中的总像元个数  
	int iTopNum = 4096;                 //顶层金字塔大小，64*64  
	int iCurNum = iPixelNum / 4;

	int anLevels[1024] = { 0 };
	int nLevelCount = 0;                //金字塔级数  

	do    //计算金字塔级数，从第二级到顶层  
	{
		anLevels[nLevelCount] = static_cast<int>(pow(2.0, nLevelCount + 2));
		nLevelCount++;
		iCurNum /= 4;
	} while (iCurNum > iTopNum);

	if (nLevelCount > 0 &&
		GDALBuildOverviews(hDataset, "nearest", nLevelCount, anLevels,
			0, NULL, ALGTermProgress, proDialog) != CE_None)
	{
		GDALClose(hDataset);
		return source + QStringLiteral(": 创建金字塔失败。");
	}

	GDALClose(hDataset);
	return enumErrTypeToString(eOK);
}

QString ipfGdalProgressTools::rasterToVector(const QString &rasterFile, const QString &vectorFile, const int index)
{
	// 打开栅格数据源
	GDALDatasetH hDataset;
	hDataset = GDALOpenEx(rasterFile.toStdString().c_str(), GDAL_OF_RASTER, NULL, NULL, NULL);
	if (!hDataset)
		return enumErrTypeToString(eSourceOpenErr);

	// 打开矢量数据源
	GDALDataset *poDS;
	poDS = (GDALDataset*)GDALOpenEx(vectorFile.toStdString().c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, NULL, NULL, NULL);
	if (poDS == NULL)
		return vectorFile + QStringLiteral(": 读取矢量文件失败，无法继续。");

	OGRLayer *poLayer = poDS->GetLayer(0);
	if (!poLayer)
		return vectorFile + QStringLiteral(": 失败，无法获取矢量图层。");

	GDALRasterBandH hBand = GDALGetRasterBand(hDataset, 1);
	GDALRasterBand* mask = ((GDALRasterBand*)hBand)->GetMaskBand();
	CPLErr err = GDALPolygonize(GDALGetRasterBand(hDataset, 1), mask, (OGRLayerH)poLayer, index, NULL, ALGTermProgress, proDialog);
	GDALClose(hDataset);
	GDALClose(poDS);

	if (err != CE_None)
		return enumErrTypeToString(eOther);

	return enumErrTypeToString(eOK);
}

QString ipfGdalProgressTools::splitShp(const QString & vectorName, QStringList & shps)
{
	GDALDataset *poDS;
	poDS = (GDALDataset*)GDALOpenEx(vectorName.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (poDS == NULL)
		return vectorName + QStringLiteral(": 读取矢量文件失败，无法继续。");

	OGRLayer *poLayer = poDS->GetLayer(0);
	if (!poLayer)
		return vectorName + QStringLiteral(": 失败，无法获取矢量图层。");


	OGRFeature * poFeature;
	while ((poFeature = poLayer->GetNextFeature()) != NULL)
	{
		// 加载shp驱动
		const char *pszDriverName = "ESRI Shapefile";
		GDALDriver *poDriver;
		poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
		if (poDriver == NULL)
			return vectorName + QStringLiteral(": 加载驱动失败。");


		// 创建矢量文件
		GDALDataset *poDS;
		QString new_shp = ipfFlowManage::instance()->getTempFormatFile(vectorName, ".shp");
		poDS = poDriver->Create(new_shp.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
		if (poDS == NULL)
			return vectorName + QStringLiteral(": 创建临时矢量文件失败。");

		// 创建矢量图层
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer("out", NULL, wkbPolygon, NULL);
		if (poLayer == NULL)
			return vectorName + QStringLiteral(": 创建图层失败。");

		// 添加要素
		poLayer->CreateFeature(poFeature);
		OGRFeature::DestroyFeature(poFeature);
		GDALClose(poDS);

		shps << new_shp;
	}
	GDALClose(poDS);
	return enumErrTypeToString(eOK);
}

QString ipfGdalProgressTools::mergeVector(const QString& outPut, const QString& ogrLayer, const QString& fieldName)
{
	QFileInfo info(ogrLayer);
	QString strArgv = QString("org@%1@%2@-dialect@sqlite@-sql@\"select ST_union(Geometry), %3 from %4 GROUP BY %5\"")
		.arg(outPut).arg(ogrLayer).arg(fieldName).arg(info.baseName()).arg(fieldName);

	ipfGdalProgressTools::errType err = ipfGDALOgrToOgr(strArgv);
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::stdevp3x3Alg(const QString & source, const QString & target, const int iband)
{
	GDALRasterBandH hSrcBand;
	GDALRasterBandH hDstBand;
	void* pData = NULL;
	GDALGeneric3x3ProcessingAlg pfnAlg = stdevpAlg;

	// 读取源数据波段
	ipfOGR ogr(source);
	if (!ogr.isOpen())
		return eSourceOpenErr;

	if (ogr.getBandSize() > 0)
		hSrcBand = ogr.getRasterBand(iband);
	else
		return eSourceOpenErr;

	// 创建新栅格
	GDALDataset* poDataset_target = ogr.createNewRaster(target, 1, GDT_Float32);
	if (!poDataset_target)
		return eNotCreateDest;

	GDALRasterBand* datasetBand = poDataset_target->GetRasterBand(1);
	datasetBand->SetNoDataValue(-9999);
	hDstBand = datasetBand;

	ipfGdalProgressTools::errType err = GDALGeneric3x3Processing(hSrcBand, hDstBand, pfnAlg, pData, ALGTermProgress, proDialog);
	if (poDataset_target)
	{
		GDALClose((GDALDatasetH)poDataset_target);
		poDataset_target = nullptr;
	}
	QString str = enumErrTypeToString(err);
	return str;
}

QString ipfGdalProgressTools::enumErrTypeToString(const ipfGdalProgressTools::errType err)
{
	switch (err)
	{
	case ipfGdalProgressTools::eOK:
		return QString();
	case ipfGdalProgressTools::eSourceOpenErr:
		return QStringLiteral("影像源无法读取，未处理。");
	case ipfGdalProgressTools::eParameterErr:
		return QStringLiteral("传递参数错误，无法进行解析，未处理。");
	case ipfGdalProgressTools::eParameterNull:
		return QStringLiteral("参数列表为空，未处理。");
	case ipfGdalProgressTools::eSourceParameterNull:
		return QStringLiteral("数据源参数为空，未处理。");
	case ipfGdalProgressTools::eDestParameterNull:
		return QStringLiteral("目标数据参数为空，未处理。");
	case ipfGdalProgressTools::eSouDestDiff:
		return QStringLiteral("源数据与目标数据不能相同。");
	case ipfGdalProgressTools::eOutputDatasetExists:
		return QStringLiteral("输出目标数据已经存在。");
	case ipfGdalProgressTools::eFormatErr:
		return QStringLiteral("错误或不支持的数据格式，未处理。");
	case ipfGdalProgressTools::eSubdatasets:
		return QStringLiteral("存在子数据，应重新选择，未处理。");
	case ipfGdalProgressTools::eTransformErr:
		return QStringLiteral("进行投影变换时失败。");
	case ipfGdalProgressTools::eRowColErr:
		return QStringLiteral("像元行列位置异常。");
	case ipfGdalProgressTools::eOther:
		return QStringLiteral("遇到未知错误，未处理。");
	case ipfGdalProgressTools::eUnChanged:
		return QStringLiteral("未改变。");
	case ipfGdalProgressTools::eNotCreateDest:
		return QStringLiteral("无法创建目标文件。");
	case ipfGdalProgressTools::eUserTerminated:
		return QStringLiteral("运行已被用户终止。");
	default:
		return QStringLiteral("遇到未知错误，未处理。");
	}
}

QString ipfGdalProgressTools::enumTypeToString(const int value)
{
    switch (value) {
    case 1:
        return "Byte";
    case 2:
        return "UInt16";
    case 3:
        return "Int16";
    case 4:
        return "UInt32";
    case 5:
        return "Int32";
    case 6:
        return "Float32";
    case 7:
        return "Float64";
    case 8:
        return "CInt16";
    case 9:
        return "CInt32";
    case 10:
        return "CFloat32";
    case 11:
        return "CFloat64";
    default:
        return "Byte";
    }
}

QString ipfGdalProgressTools::enumFormatToString(const QString & format)
{
	if (format == QStringLiteral("tif"))
		return QStringLiteral("GTiff");
	if (format == QStringLiteral("img"))
		return QStringLiteral("HFA");
	if (format == QStringLiteral("pix"))
		return QStringLiteral("PCIDSK");
	if (format == QStringLiteral("vrt"))
		return QStringLiteral("VRT");

	return QStringLiteral("other");
}

void ipfGdalProgressTools::showProgressDialog()
{
	proDialog->show();
}

void ipfGdalProgressTools::hideProgressDialog()
{
	proDialog->hide();
}

void ipfGdalProgressTools::pulsValueTatal()
{
	proDialog->pulsValueTatal();
}

void ipfGdalProgressTools::setProgressTitle(const QString & label)
{
	proDialog->setTitle(label);
}

void ipfGdalProgressTools::setProgressSize(const int max)
{
	proDialog->setRangeTotal(0, max);
}

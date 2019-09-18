#include "ipfModelerProcessChildProcessRasterRectPosition.h"
#include "head.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"
#include "ipfFlowManage.h"

ipfModelerProcessChildProcessRasterRectPosition::ipfModelerProcessChildProcessRasterRectPosition
	(QObject *parent, const QString modelerName) : ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
}

ipfModelerProcessChildProcessRasterRectPosition::~ipfModelerProcessChildProcessRasterRectPosition()
{
}

bool ipfModelerProcessChildProcessRasterRectPosition::checkParameter()
{
	return true;
}

void ipfModelerProcessChildProcessRasterRectPosition::setParameter()
{
	QMessageBox::about(0, QStringLiteral("调整栅格角点坐标")
		, QStringLiteral("该模块将自动读取栅格像元大小，\n调整栅格角点起始位置至像元倍数的中心点。\n\n注意：该操作将直接修改原始数据！"));
}

QString ipfModelerProcessChildProcessRasterRectPosition::adjustRange(const QString & fileName)
{
	// 计算四至理论坐标
	ipfOGR ogr(fileName, true);
	if (!ogr.isOpen())
		return QStringLiteral("影像源无法读取，未处理。");

	double xmin = 0.0;
	double ymax = 0.0;

	// 1/2像元大小
	double sizeMid = ogr.getPixelSize() / 2;

	// 左上角坐标
	QgsRectangle rect = ogr.getXY();
	double ufX = rect.xMinimum();
	double ufY = rect.yMaximum();

	if (sizeMid == 1)
	{
		int xtmp = round(ufX);
		if ((xtmp % 2) == 0)
			xmin = xtmp + sizeMid;
		else
			xmin = xtmp;
		int ytmp = round(ufY);
		if ((ytmp % 2) == 0)
			ymax = ytmp - sizeMid;
		else
			ymax = ytmp;
	}
	else
	{
		int xtmp = round(ufX / sizeMid);
		xmin = xtmp * sizeMid;
		int ytmp = round(ufY / sizeMid);
		ymax = ytmp * sizeMid;
	}

	if (ogr.setGeoXy(xmin, ymax))
		return QString();
	else
		return QStringLiteral("影像源无法修改坐标信息，未处理。");
}

void ipfModelerProcessChildProcessRasterRectPosition::run()
{
	clearOutFiles();
	clearErrList();
	ipfGdalProgressTools gdal;

	foreach(QString var, filesIn())
	{
		QString err = adjustRange(var);
		if (err.isEmpty())
			appendOutFile(var);
		else
			addErrList(var + ": " + err);
	}
}

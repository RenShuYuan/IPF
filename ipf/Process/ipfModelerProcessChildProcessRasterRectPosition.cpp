#include "ipfModelerProcessChildProcessRasterRectPosition.h"
#include "head.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "ipfFlowManage.h"

ipfModelerProcessChildProcessRasterRectPosition::ipfModelerProcessChildProcessRasterRectPosition
	(QObject *parent, const QString modelerName) : ipfModelerProcessBase(parent, modelerName)
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
		, QStringLiteral("该模块将自动读取栅格像元大小，\n调整栅格角点起始位置至像元倍数的中心点。"));
}

void ipfModelerProcessChildProcessRasterRectPosition::run()
{
	clearOutFiles();
	clearErrList();
	ipfGdalProgressTools gdal;

	foreach(QString var, filesIn())
	{
		QString target = ipfFlowManage::instance()->getTempFormatFile(var, ".vrt");
		QString err = gdal.rangeMultiple(var, target);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

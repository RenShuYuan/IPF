#include "ipfModelerProcessChildPixelDecimals.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerPixelDecimalsDialog.h"

ipfModelerProcessChildPixelDecimals::ipfModelerProcessChildPixelDecimals(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	decimals = 0;
	dialog = new ipfModelerPixelDecimalsDialog();
}


ipfModelerProcessChildPixelDecimals::~ipfModelerProcessChildPixelDecimals()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildPixelDecimals::checkParameter()
{
	bool isbl = true;

	if (!decimals)
	{
		isbl = false;
		addErrList(QStringLiteral("没有输入像素值保留位数。"));
	}

	return isbl;
}

void ipfModelerProcessChildPixelDecimals::setParameter()
{
	if (dialog->exec())
	{
		decimals = dialog->getParameter();
	}
}

QMap<QString, QString> ipfModelerProcessChildPixelDecimals::getParameter()
{
	QMap<QString, QString> map;
	map["decimals"] = QString::number(decimals);

	return map;
}

void ipfModelerProcessChildPixelDecimals::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	decimals = map["decimals"].toInt();
}

void ipfModelerProcessChildPixelDecimals::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		QString target = ipfFlowManage::instance()->getTempFormatFile(var, ".vrt");
		QString err = gdal.pixelDecimal(var, target, decimals);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

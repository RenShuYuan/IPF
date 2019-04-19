#include "ipfModelerProcessChildPixelMoidfyValue.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerPixelMoidfyValueDialog.h"

ipfModelerProcessChildPixelMoidfyValue::ipfModelerProcessChildPixelMoidfyValue(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	oldValue = 0.0;
	newValue = 0.0;
	dialog = new ipfModelerPixelMoidfyValueDialog();
}

ipfModelerProcessChildPixelMoidfyValue::~ipfModelerProcessChildPixelMoidfyValue()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildPixelMoidfyValue::checkParameter()
{
	return true;
}

void ipfModelerProcessChildPixelMoidfyValue::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		oldValue = map["oldValue"].toDouble();
		newValue = map["newValue"].toDouble();
	}
}

QMap<QString, QString> ipfModelerProcessChildPixelMoidfyValue::getParameter()
{
	QMap<QString, QString> map;
	map["oldValue"] = QString::number(oldValue, 'f', 3);
	map["newValue"] = QString::number(newValue, 'f', 3);

	return map;
}

void ipfModelerProcessChildPixelMoidfyValue::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	oldValue = map["oldValue"].toDouble();
	newValue = map["newValue"].toDouble();
}

void ipfModelerProcessChildPixelMoidfyValue::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		QString target = ipfFlowManage::instance()->getTempFormatFile(var, ".vrt");
		QString err = gdal.pixelModifyValue(var, target, oldValue, newValue);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

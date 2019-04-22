#include "ipfModelerProcessChildPixelMoidfyValue.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerPixelMoidfyValueDialog.h"

ipfModelerProcessChildPixelMoidfyValue::ipfModelerProcessChildPixelMoidfyValue(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	oldValue_1 = 0.0;
	newValue_1 = 0.0;
	oldValue_2 = 0.0;
	newValue_2 = 0.0;
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
		oldValue_1 = map["oldValue_1"].toDouble();
		newValue_1 = map["newValue_1"].toDouble();
		oldValue_2 = map["oldValue_2"].toDouble();
		newValue_2 = map["newValue_2"].toDouble();
	}
}

QMap<QString, QString> ipfModelerProcessChildPixelMoidfyValue::getParameter()
{
	QMap<QString, QString> map;
	map["oldValue_1"] = QString::number(oldValue_1, 'f', 3);
	map["newValue_1"] = QString::number(newValue_1, 'f', 3);
	map["oldValue_2"] = QString::number(oldValue_2, 'f', 3);
	map["newValue_2"] = QString::number(newValue_2, 'f', 3);

	return map;
}

void ipfModelerProcessChildPixelMoidfyValue::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	oldValue_1 = map["oldValue_1"].toDouble();
	newValue_1 = map["newValue_1"].toDouble();
	oldValue_2 = map["oldValue_2"].toDouble();
	newValue_2 = map["newValue_2"].toDouble();
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
		QString err = gdal.pixelModifyValue(var, target, oldValue_1, newValue_1, oldValue_2, newValue_2);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

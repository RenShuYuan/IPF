#include "ipfModelerProcessChildPixelMoidfyValue.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerPixelMoidfyValueDialog.h"

ipfModelerProcessChildPixelMoidfyValue::ipfModelerProcessChildPixelMoidfyValue(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
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
		if (map["bands_noDiffe"] == "YES")
			bands_noDiffe = true;
		else
			bands_noDiffe = false;
	}
}

QMap<QString, QString> ipfModelerProcessChildPixelMoidfyValue::getParameter()
{
	QMap<QString, QString> map;
	map["oldValue"] = QString::number(oldValue);
	map["newValue"] = QString::number(newValue);
	if (bands_noDiffe)
		map["bands_noDiffe"] = "YES";
	else
		map["bands_noDiffe"] = "NO";

	return map;
}

void ipfModelerProcessChildPixelMoidfyValue::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	oldValue = map["oldValue"].toDouble();
	newValue = map["newValue"].toDouble();
	if (map["bands_noDiffe"] == "YES")
		bands_noDiffe = true;
	else
		bands_noDiffe = false;
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
		QString target = ipfApplication::instance()->getTempVrtFile(var);
		QString err = gdal.pixelModifyValue(var, target, oldValue, newValue, bands_noDiffe);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

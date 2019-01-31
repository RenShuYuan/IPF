#include "ipfModelerProcessChildTypeConvert.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerTypeConvertDialog.h"

ipfModelerProcessChildTypeConvert::ipfModelerProcessChildTypeConvert(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	typeConvert = new ipfModelerTypeConvertDialog();
	dataType = typeConvert->getParameter();
}

ipfModelerProcessChildTypeConvert::~ipfModelerProcessChildTypeConvert()
{
	if (typeConvert) { delete typeConvert; }
}

bool ipfModelerProcessChildTypeConvert::checkParameter()
{
	bool isbl = true;

	if (dataType.isEmpty())
	{
		isbl = false;
		addErrList(QStringLiteral("错误或不支持的位深。"));
	}

	return isbl;
}

void ipfModelerProcessChildTypeConvert::setParameter()
{
	if (typeConvert->exec())
	{
		dataType = typeConvert->getParameter();
	}
}

QMap<QString, QString> ipfModelerProcessChildTypeConvert::getParameter()
{
	QMap<QString, QString> map;
	map["dataType"] = dataType;

	return map;
}

void ipfModelerProcessChildTypeConvert::setDialogParameter(QMap<QString, QString> map)
{
	typeConvert->setParameter(map);
	dataType = map["dataType"];
}

void ipfModelerProcessChildTypeConvert::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);
		
		QString err = gdal.typeConvert(var, target, dataType);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

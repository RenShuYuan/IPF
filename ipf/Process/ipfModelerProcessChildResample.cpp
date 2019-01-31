#include "ipfModelerProcessChildResample.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerResampleDialog.h"

ipfModelerProcessChildResample::ipfModelerProcessChildResample(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	res = 0.0;
	resample = new ipfModelerResampleDialog();
}


ipfModelerProcessChildResample::~ipfModelerProcessChildResample()
{
	if (resample) { delete resample; }
}

bool ipfModelerProcessChildResample::checkParameter()
{
	bool isbl = true;

	if (res<=0)
	{
		isbl = false;
		addErrList(QStringLiteral("��Ԫ��С��Ӧ��С�ڻ����0��"));
	}

	return isbl;
}

void ipfModelerProcessChildResample::setParameter()
{
	if (resample->exec())
	{
		QMap<QString, QString> map = resample->getParameter();
		resampling_method = map["resampling_method"];
		res = map["res"].toDouble();
	}
}

QMap<QString, QString> ipfModelerProcessChildResample::getParameter()
{
	QMap<QString, QString> map;
	map["res"] = QString::number(res, 'f', 15);
	map["resampling_method"] = resampling_method;

	return map;
}

void ipfModelerProcessChildResample::setDialogParameter(QMap<QString, QString> map)
{
	resample->setParameter(map);
	resampling_method = map["resampling_method"];
	res = map["res"].toDouble();
}

void ipfModelerProcessChildResample::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);

		QString err = gdal.resample(var, target, res, resampling_method);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

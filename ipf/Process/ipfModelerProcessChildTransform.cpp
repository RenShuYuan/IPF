#include "ipfModelerProcessChildTransform.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerTransformDialog.h"

ipfModelerProcessChildTransform::ipfModelerProcessChildTransform(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerTransformDialog();
}


ipfModelerProcessChildTransform::~ipfModelerProcessChildTransform()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildTransform::checkParameter()
{
	bool isbl = true;

	if (s_srs.isEmpty() || t_srs.isEmpty())
	{
		isbl = false;
		addErrList(QStringLiteral("源参考坐标系或目标参考坐标系不正确。"));
	}

	return isbl;
}

void ipfModelerProcessChildTransform::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		resampling_method = map["resampling_method"];
		s_srs = map["s_srs"];
		t_srs = map["t_srs"];
	}
}

QMap<QString, QString> ipfModelerProcessChildTransform::getParameter()
{
	QMap<QString, QString> map;
	map["resampling_method"] = resampling_method;
	map["s_srs"] = s_srs;
	map["t_srs"] = t_srs;

	return map;
}

void ipfModelerProcessChildTransform::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	resampling_method = map["resampling_method"];
	s_srs = map["s_srs"];
	t_srs = map["t_srs"];
}

void ipfModelerProcessChildTransform::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);

		QString err = gdal.transform(var, target, s_srs, t_srs, resampling_method);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

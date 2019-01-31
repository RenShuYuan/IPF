#include "ipfModelerProcessChildQuickView.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerQuickViewDialog.h"

ipfModelerProcessChildQuickView::ipfModelerProcessChildQuickView(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	quickView = new ipfModelerQuickViewDialog();
	bs = quickView->getParameter();
}


ipfModelerProcessChildQuickView::~ipfModelerProcessChildQuickView()
{
	if (quickView) { delete quickView; }
}

bool ipfModelerProcessChildQuickView::checkParameter()
{
	bool isbl = true;

	if (bs < 1 && bs > 100)
	{
		isbl = false;
		addErrList(QStringLiteral("像元缩小倍数超过限差。"));
	}

	return isbl;
}

void ipfModelerProcessChildQuickView::setParameter()
{
	if (quickView->exec())
	{
		bs = quickView->getParameter();
	}
}

QMap<QString, QString> ipfModelerProcessChildQuickView::getParameter()
{
	QMap<QString, QString> map;
	map["bs"] = QString::number(bs);

	return map;
}

void ipfModelerProcessChildQuickView::setDialogParameter(QMap<QString, QString> map)
{
	quickView->setParameter(map);
	map["bs"] = QString::number(bs);
}

void ipfModelerProcessChildQuickView::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);

		QString err = gdal.quickView(var, target, bs);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

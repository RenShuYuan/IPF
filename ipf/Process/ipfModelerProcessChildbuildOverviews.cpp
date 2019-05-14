#include "ipfModelerProcessChildbuildOverviews.h"
#include "../gdal/ipfgdalprogresstools.h"
#include <QFile>

ipfModelerProcessChildbuildOverviews::ipfModelerProcessChildbuildOverviews(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
}


ipfModelerProcessChildbuildOverviews::~ipfModelerProcessChildbuildOverviews()
{
}

bool ipfModelerProcessChildbuildOverviews::checkParameter()
{
	return true;
}

void ipfModelerProcessChildbuildOverviews::setParameter()
{
	QMessageBox::about(0, QStringLiteral("创建金字塔")
		, QStringLiteral("该模块将自动创建金字塔，无需参数设置。"));
}

void ipfModelerProcessChildbuildOverviews::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		QString err = gdal.buildOverviews(var);
		if (!err.isEmpty())
			addErrList(var + ": " + err);
	}
}

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
	QMessageBox::about(0, QStringLiteral("����������")
		, QStringLiteral("��ģ�齫�Զ�����������������������á�"));
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

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
		//else
		//{
		//	// �޸Ľ�������׺
		//	QFile file(var.mid(0, var.size()-3)+"aux");
		//	if (file.exists())
		//	{
		//		file.rename(var.mid(0, var.size() - 3) + "rrd");
		//	}
		//}
	}
}

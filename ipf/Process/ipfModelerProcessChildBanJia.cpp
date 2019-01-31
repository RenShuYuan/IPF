#include "ipfModelerProcessChildBanJia.h"
#include "../gdal/ipfExcel.h"


ipfModelerProcessChildBanJia::ipfModelerProcessChildBanJia(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
}


ipfModelerProcessChildBanJia::~ipfModelerProcessChildBanJia()
{
}

bool ipfModelerProcessChildBanJia::checkParameter()
{
	return false;
}

void ipfModelerProcessChildBanJia::setParameter()
{
}

QMap<QString, QString> ipfModelerProcessChildBanJia::getParameter()
{
	return QMap<QString, QString>();
}

void ipfModelerProcessChildBanJia::setDialogParameter(QMap<QString, QString> map)
{
}

void ipfModelerProcessChildBanJia::run()
{
}

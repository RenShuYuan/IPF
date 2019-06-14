#include "ipfModelerProcessBase.h"
#include "head.h"
#include "ipfOgr.h"

#include <QFileInfo>

ipfModelerProcessBase::ipfModelerProcessBase(QObject *parent, const QString modelerName)
	: QObject(parent)
	, modelerName(modelerName)
{
	setId(QUuid::createUuid().toString());
}


ipfModelerProcessBase::~ipfModelerProcessBase()
{
}

void ipfModelerProcessBase::setParameter()
{
}

bool ipfModelerProcessBase::checkParameter()
{
	return false;
}

void ipfModelerProcessBase::setInFiles(const QStringList files)
{
	mFilesIn = files;
}

void ipfModelerProcessBase::setOutFiles(const QStringList files)
{
	mFilesOut = files;
}

void ipfModelerProcessBase::appendOutFile(const QString file)
{
	mFilesOut.append(file);
}

QString ipfModelerProcessBase::removeDelimiter(const QString file)
{
	QString reName;

	// Esri Grid (hdr.adf)
	if (QFileInfo(file).fileName() == "hdr.adf")
	{
		ipfOGR org(file);
		if (!org.isOpen() || org.getBandSize() < 1) return QString();
		reName = org.getRasterBand(1)->GetDescription();
	}
	else
	{
		QString fileName = QFileInfo(file).fileName();
		fileName = fileName.left(fileName.lastIndexOf('.'));
		QStringList list = fileName.split(NAME_DELIMITER);
		if (list.isEmpty())
			return QString();
		else
			reName = list.at(0);
	}
	return reName;
}

void ipfModelerProcessBase::run()
{
}
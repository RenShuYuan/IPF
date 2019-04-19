#include "ipfModelerProcessBase.h"
#include "head.h"

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
	QFileInfo info(file);
	QString name = info.fileName();
	name = name.left(name.lastIndexOf('.'));

	QStringList list = name.split(NAME_DELIMITER);
	if (list.isEmpty())
		return QString();
	else
		return list.at(0);
}

QString ipfModelerProcessBase::addDelimiter(const QString file, const QString mark)
{
	QFileInfo info(file);
	QString name = info.fileName() + NAME_DELIMITER + mark;
	return name;
}

void ipfModelerProcessBase::run()
{
}
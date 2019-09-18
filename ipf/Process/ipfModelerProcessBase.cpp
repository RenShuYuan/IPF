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

void ipfModelerProcessBase::run()
{
}
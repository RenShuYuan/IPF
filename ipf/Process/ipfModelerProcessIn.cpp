#include "ipfModelerProcessIn.h"
#include "../ui/ipfModelerInDialog.h"

#include <QFileInfo>

ipfModelerProcessIn::ipfModelerProcessIn(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	in = new ipfModelerInDialog();
}


ipfModelerProcessIn::~ipfModelerProcessIn()
{
	RELEASE(in);
}

void ipfModelerProcessIn::setParameter()
{
	if (in->exec())
	{
		setOutFiles(in->getParameter());
	}
}

bool ipfModelerProcessIn::checkParameter()
{
	bool isbl = true;

	if (filesOut().isEmpty())
	{
		isbl = false;
		addErrList(QStringLiteral("û���������ݡ�"));
	}

	foreach (QString file, filesOut())
	{
		QFileInfo info(file);
		if (!info.exists())
		{
			isbl = false;
			addErrList(info.fileName() + QStringLiteral(": ��Ч������·����"));
		}
	}

	return isbl;
}

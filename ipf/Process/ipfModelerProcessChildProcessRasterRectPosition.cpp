#include "ipfModelerProcessChildProcessRasterRectPosition.h"
#include "head.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "ipfFlowManage.h"

ipfModelerProcessChildProcessRasterRectPosition::ipfModelerProcessChildProcessRasterRectPosition
	(QObject *parent, const QString modelerName) : ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
}

ipfModelerProcessChildProcessRasterRectPosition::~ipfModelerProcessChildProcessRasterRectPosition()
{
}

bool ipfModelerProcessChildProcessRasterRectPosition::checkParameter()
{
	return true;
}

void ipfModelerProcessChildProcessRasterRectPosition::setParameter()
{
	QMessageBox::about(0, QStringLiteral("����դ��ǵ�����")
		, QStringLiteral("��ģ�齫�Զ���ȡդ����Ԫ��С��\n����դ��ǵ���ʼλ������Ԫ���������ĵ㡣"));
}

void ipfModelerProcessChildProcessRasterRectPosition::run()
{
	clearOutFiles();
	clearErrList();
	ipfGdalProgressTools gdal;

	foreach(QString var, filesIn())
	{
		QString target = ipfFlowManage::instance()->getTempFormatFile(var, ".vrt");
		QString err = gdal.rangeMultiple(var, target);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

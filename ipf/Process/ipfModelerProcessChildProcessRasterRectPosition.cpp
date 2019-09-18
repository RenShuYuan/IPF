#include "ipfModelerProcessChildProcessRasterRectPosition.h"
#include "head.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"
#include "ipfFlowManage.h"

ipfModelerProcessChildProcessRasterRectPosition::ipfModelerProcessChildProcessRasterRectPosition
	(QObject *parent, const QString modelerName) : ipfModelerProcessOut(parent, modelerName)
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
		, QStringLiteral("��ģ�齫�Զ���ȡդ����Ԫ��С��\n����դ��ǵ���ʼλ������Ԫ���������ĵ㡣\n\nע�⣺�ò�����ֱ���޸�ԭʼ���ݣ�"));
}

QString ipfModelerProcessChildProcessRasterRectPosition::adjustRange(const QString & fileName)
{
	// ����������������
	ipfOGR ogr(fileName, true);
	if (!ogr.isOpen())
		return QStringLiteral("Ӱ��Դ�޷���ȡ��δ����");

	double xmin = 0.0;
	double ymax = 0.0;

	// 1/2��Ԫ��С
	double sizeMid = ogr.getPixelSize() / 2;

	// ���Ͻ�����
	QgsRectangle rect = ogr.getXY();
	double ufX = rect.xMinimum();
	double ufY = rect.yMaximum();

	if (sizeMid == 1)
	{
		int xtmp = round(ufX);
		if ((xtmp % 2) == 0)
			xmin = xtmp + sizeMid;
		else
			xmin = xtmp;
		int ytmp = round(ufY);
		if ((ytmp % 2) == 0)
			ymax = ytmp - sizeMid;
		else
			ymax = ytmp;
	}
	else
	{
		int xtmp = round(ufX / sizeMid);
		xmin = xtmp * sizeMid;
		int ytmp = round(ufY / sizeMid);
		ymax = ytmp * sizeMid;
	}

	if (ogr.setGeoXy(xmin, ymax))
		return QString();
	else
		return QStringLiteral("Ӱ��Դ�޷��޸�������Ϣ��δ����");
}

void ipfModelerProcessChildProcessRasterRectPosition::run()
{
	clearOutFiles();
	clearErrList();
	ipfGdalProgressTools gdal;

	foreach(QString var, filesIn())
	{
		QString err = adjustRange(var);
		if (err.isEmpty())
			appendOutFile(var);
		else
			addErrList(var + ": " + err);
	}
}

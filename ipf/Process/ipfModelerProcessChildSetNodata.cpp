#include "ipfModelerProcessChildSetNodata.h"
#include "ipfFlowManage.h"
#include "../ipfOgr.h"
#include "../ui/ipfModelerSetNodataDialog.h"

ipfModelerProcessChildSetNodata::ipfModelerProcessChildSetNodata(QObject * parent, const QString modelerName)
	:ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	nodata = 0;
	isDel = false;

	dialog = new ipfModelerSetNodataDialog();
}

ipfModelerProcessChildSetNodata::~ipfModelerProcessChildSetNodata()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildSetNodata::checkParameter()
{
	if ((nodata == 34743735.938444) && (!isDel))
	{
		addErrList(QStringLiteral("û��������Чֵ��"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildSetNodata::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		nodata = map["nodata"].toDouble();
		if (map["isDel"] == "YES")
			isDel = true;
		else
			isDel = false;
	}
}

QMap<QString, QString> ipfModelerProcessChildSetNodata::getParameter()
{
	QMap<QString, QString> map;

	map["nodata"] = QString::number(nodata, 'f', 3);

	if (isDel)
		map["isDel"] = "YES";
	else
		map["isDel"] = "NO";

	return map;
}

void ipfModelerProcessChildSetNodata::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	nodata = map["nodata"].toDouble();
	if (map["isDel"] == "YES")
		isDel = true;
	else
		isDel = false;
}

void ipfModelerProcessChildSetNodata::run()
{
	clearOutFiles();
	clearErrList();

	foreach(QString var, filesIn())
	{
		ipfOGR ogr(var, true);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡդ������ʧ�ܣ���������"));
			continue;
		}
		if (ogr.getBandSize() == 0)
		{
			addErrList(var + QStringLiteral(": ��ȡդ�񲨶�ʧ�ܣ���������"));
			continue;
		}

		bool bl = true;
		CPLErr err;
		for (int i = 1; i <= ogr.getBandSize(); ++i)
		{
			GDALRasterBand *band = ogr.getRasterBand(i);
			if (isDel)
				err = band->DeleteNoDataValue();
			else
			{
				err = band->SetNoDataValue(nodata);
				GDALSetRasterNoDataValue(band, nodata);
			}

			if (err != CE_None)
			{
				bl = false;
				addErrList(var + QStringLiteral(": ��%1��������ʧ�ܡ�").arg(i));
			}
		}
		if (bl)
			appendOutFile(var);
	}
}

#include "ipfModelerProcessOut.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerOutDialog.h"
#include "../ipfOgr.h"

#include <QDir>

ipfModelerProcessOut::ipfModelerProcessOut(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	out = new ipfModelerOutDialog();
	map = out->getParameter();
}

ipfModelerProcessOut::~ipfModelerProcessOut()
{
	if (out) { delete out; }
}

void ipfModelerProcessOut::setParameter()
{
	if (out->exec())
	{
		map = out->getParameter();
		format = map["format"];
		outPath = map["outPath"];
		compress = map["compress"];
		isTfw = map["isTfw"];
		noData = map["noData"];
	}
}

QMap<QString, QString> ipfModelerProcessOut::getParameter()
{
	QMap<QString, QString> map;
	map["format"] = format;
	map["outPath"] = outPath;
	map["compress"] = compress;
	map["isTfw"] = isTfw;
	map["noData"] = noData;

	return map;
}

void ipfModelerProcessOut::setDialogParameter(QMap<QString, QString> map)
{
	out->setParameter(map);

	format = map["format"];
	outPath = map["outPath"];
	compress = map["compress"];
	isTfw = map["isTfw"];
	noData = map["noData"];
}

void ipfModelerProcessOut::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach (QString var, filesIn())
	{
		// 将NODATA值设置为与原图一致
		//if (noData == "none")
		//{
		//	ipfOGR ogr(var);
		//	if (ogr.isOpen())
		//		noData = QString::number(ogr.getNodataValue(1));
		//}

		QString target = outPath + "\\" + removeDelimiter(var) + '.' + format;
		QString err = gdal.formatConvert(var, target, gdal.enumFormatToString(format), compress, isTfw, noData);
		if (err.isEmpty())
			appendOutFile(target);
		else
			addErrList(var + ": " + err);
	}
}

bool ipfModelerProcessOut::checkParameter()
{
	bool isbl = true;
	clearErrList();

	if (ipfGdalProgressTools::enumFormatToString(format)
		== QStringLiteral("other"))
	{
		isbl = false;
		addErrList(QStringLiteral("错误或不支持的数据格式。"));
	}
	QDir dir(outPath);
	if (!dir.exists())
	{
		isbl = false;
		addErrList(QStringLiteral("无效的输出文件夹。"));
	}

	return isbl;
}

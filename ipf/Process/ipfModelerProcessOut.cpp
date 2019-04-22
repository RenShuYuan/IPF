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
	RELEASE(out);
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

bool ipfModelerProcessOut::printErrToFile(const QString &fileName, const QStringList &errList)
{
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		addErrList(fileName + QStringLiteral("创建错误文件失败，已终止。"));
		return false;
	}
	QTextStream out(&file);
	foreach(QString str, errList)
		out << str << endl;
	file.close();
	return true;
}

void ipfModelerProcessOut::run()
{
	clearOutFiles();
	clearErrList();

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();


	// 这句使用OpenMP来加速 	foreach (QString var, filesIn())
//#pragma omp parallel for
	for (int i = 0; i < filesIn().size(); ++i)
	{
		QString var = filesIn().at(i);
		QString target = outPath + "\\" + removeDelimiter(var) + '.' + format;
		QString err = gdal.formatConvert(var, target, gdal.enumFormatToString(format), compress, isTfw, noData);
//#pragma omp critical
		//{
			if (err.isEmpty())
				appendOutFile(target);
			else
				addErrList(var + ": " + err);
		//}
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

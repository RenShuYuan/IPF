#include "ipfModelerProcessChildRasterInfoPrint.h"
#include "../ipfOgr.h"
#include "../ipf/gdal/ipfgdalprogresstools.h"
#include "../../ui/ipfModelerRasterInfoPrintDialog.h"

#include <QDir>
#include <QFileInfo>
#include <QProgressDialog>

ipfModelerProcessChildRasterInfoPrint::ipfModelerProcessChildRasterInfoPrint(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerRasterInfoPrintDialog();
}


ipfModelerProcessChildRasterInfoPrint::~ipfModelerProcessChildRasterInfoPrint()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildRasterInfoPrint::checkParameter()
{
	QDir dir = QFileInfo(saveName).dir();
	if (!dir.exists())
	{
		addErrList(QStringLiteral("无效的输出文件夹。"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildRasterInfoPrint::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		saveName = map["saveName"];
	}
}

QMap<QString, QString> ipfModelerProcessChildRasterInfoPrint::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;

	return map;
}

void ipfModelerProcessChildRasterInfoPrint::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	saveName = map["saveName"];
}

void ipfModelerProcessChildRasterInfoPrint::run()
{
	clearOutFiles();
	clearErrList();
	QStringList outList;

	//进度条
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("栅格信息输出..."), QStringLiteral("取消"), 0, filesIn().size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("栅格信息输出"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	foreach(QString var, filesIn())
	{
		dialog.setValue(++prCount);
		QApplication::processEvents();
		if (dialog.wasCanceled())
			return;

		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": 栅格数据读取失败。"));
			continue;
		}

		// 栅格名称
		QFileInfo info(var);
		outList << info.baseName();

		// 分辨率 X Y
		double pSize = ogr.getPixelSize();
		if (pSize == -98765.4)
			outList << QStringLiteral("\t像元大小: 横向与纵向不一致！");
		else
			outList << QStringLiteral("\t像元大小: %1").arg(pSize);

		// 波段数
		outList << QStringLiteral("\t波段数量: %1").arg(ogr.getBandSize());

		// 位深
		outList << QStringLiteral("\t位深: ") + ipfGdalProgressTools::enumTypeToString(ogr.getDataType());

		// NODATA
		for (int i = 1; i <= ogr.getBandSize(); ++i)
			outList << QStringLiteral("\tNODATA: %1").arg(ogr.getNodataValue(i));

		// 压缩
		outList << QStringLiteral("\t压缩格式: %1").arg(ogr.getCompressionName());

		// 投影信息
		outList << QStringLiteral("\t坐标系统: %1").arg(ogr.getProjection());
	}

	// 输出错误
	printErrToFile(saveName, outList);
}

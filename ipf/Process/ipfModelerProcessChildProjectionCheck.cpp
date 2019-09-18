#include "ipfModelerProcessChildProjectionCheck.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfModelerProjectionCheckDialog.h"
#include "../ipfOgr.h"

#include "QgsRasterLayer.h"
#include "qgscoordinatereferencesystem.h"

#include <QFileInfo>
#include <QProgressDialog>

ipfModelerProcessChildProjectionCheck::ipfModelerProcessChildProjectionCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerProjectionCheckDialog();
}

ipfModelerProcessChildProjectionCheck::~ipfModelerProcessChildProjectionCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildProjectionCheck::checkParameter()
{
	bool isbl = true;

	if (s_srs.isEmpty())
	{
		isbl = false;
		addErrList(QStringLiteral("源参考坐标系或目标参考坐标系不正确。"));
	}

	QDir dir = QFileInfo(saveName).dir();
	if (!dir.exists())
	{
		addErrList(QStringLiteral("无效的输出路径。"));
		isbl = false;
	}

	return isbl;
}

void ipfModelerProcessChildProjectionCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		s_srs = map["s_srs"];
		saveName = map["saveName"];
	}
}

QMap<QString, QString> ipfModelerProcessChildProjectionCheck::getParameter()
{
	QMap<QString, QString> map;
	map["s_srs"] = s_srs;
	map["saveName"] = saveName;

	return map;
}

void ipfModelerProcessChildProjectionCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	s_srs = map["s_srs"];
	saveName = map["saveName"];
}

void ipfModelerProcessChildProjectionCheck::run()
{
	clearOutFiles();
	clearErrList();
	QStringList outList;

	//进度条
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("栅格数据投影检查..."), QStringLiteral("取消"), 0, filesIn().size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("栅格数据投影检查"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	foreach(QString var, filesIn())
	{
		dialog.setValue(++prCount);
		QApplication::processEvents();
		if (dialog.wasCanceled())
			return;

		// 用QGis打开栅格
		QFileInfo info(var);
		QString layerName = info.baseName();
		QgsRasterLayer* layer = new QgsRasterLayer(var, layerName, "gdal");
		if (!layer || !layer->isValid())
		{
			addErrList(var + QStringLiteral(": 栅格数据读取失败。"));
			continue;
		}
		QgsCoordinateReferenceSystem layerCrs = layer->crs(); 
		QString crsId = layerCrs.authid();
		RELEASE(layer);

		if (s_srs != crsId)
		{
			outList << var + QStringLiteral(": 投影信息不一致。");
			outList << QStringLiteral("\t影像: ") + crsId;
			outList << QStringLiteral("\t基准: ") + s_srs;
		}
		else
		{
			outList << var + QStringLiteral(": 投影正确。");
		}
	}

	// 输出错误
	printErrToFile(saveName, outList);
}

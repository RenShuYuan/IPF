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
	if (dialog) { delete dialog; }
}

bool ipfModelerProcessChildProjectionCheck::checkParameter()
{
	bool isbl = true;

	if (s_srs.isEmpty())
	{
		isbl = false;
		addErrList(QStringLiteral("源参考坐标系或目标参考坐标系不正确。"));
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

		//// 打开栅格
		//QFileInfo info(var);
		//QString fileName = info.baseName();
		//QgsRasterLayer* layer = new QgsRasterLayer(var, fileName, "gdal");
		//if (!layer->isValid())
		//{
		//	addErrList(fileName + QStringLiteral(": 栅格数据读取失败。"));
		//	continue;
		//}
		//	
		//QgsCoordinateReferenceSystem srcCrs = layer->crs();
		//QString id = srcCrs.authid();
		//ipfOGR ogr(var);
		//if (!ogr.isOpen())
		//{
		//	addErrList(var + QStringLiteral(": 栅格数据读取失败。"));
		//	continue;
		//}
		//double R = ogr.getPixelSize();
		//
		//QString rasterProjection;
		//if (R < 0.1)
		//	rasterProjection = "EPSG:4326";
		//else
		//	rasterProjection = "EPSG:32649";

		//if (id != rasterProjection)
		//	outList << fileName + QStringLiteral(": 投影信息不一致。");
		//else
		//	outList << fileName + QStringLiteral(": 投影正确。");

		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": 栅格数据读取失败。"));
			continue;
		}
		QString strProjection = ogr.getProjection();
		double R = ogr.getPixelSize();
		QString rasterProjection;
		if (R < 0.1)
			rasterProjection = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]]";
		else
			rasterProjection = "PROJCS[\"WGS_1984_UTM_Zone_49N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,-0,-0,-0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",111],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"Meter\",1],AUTHORITY[\"EPSG\",\"32649\"]]";
		if (strProjection != rasterProjection)
			outList << var + QStringLiteral(": 投影信息不一致。");
		else
			outList << var + QStringLiteral(": 投影正确。");
	}

	// 输出错误
	QFile file(saveName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		addErrList(saveName + QStringLiteral("创建文件失败，已终止。"));
		return;
	}
	QTextStream out(&file);
	foreach(QString str, outList)
		out << str << endl;
	file.close();
}

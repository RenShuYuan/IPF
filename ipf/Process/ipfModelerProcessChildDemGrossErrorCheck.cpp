#include "ipfModelerProcessChildDemGrossErrorCheck.h"
#include "../../ui/ipfModelerDemGrossErrorCheckDialog.h"
#include "../../ui/ipfProgress.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"
#include "QgsRasterLayer.h"
#include "qgscoordinatereferencesystem.h"

ipfModelerProcessChildDemGrossErrorCheck::ipfModelerProcessChildDemGrossErrorCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerDemGrossErrorCheckDialog();
}

ipfModelerProcessChildDemGrossErrorCheck::~ipfModelerProcessChildDemGrossErrorCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildDemGrossErrorCheck::checkParameter()
{
	bool isbl = true;

	QDir dir = QFileInfo(errFile).dir();
	if (!dir.exists())
	{
		addErrList(QStringLiteral("��Ч�����·����"));
		isbl = false;
	}

	if (!QFileInfo(rasterName).exists())
	{
		addErrList(QStringLiteral("��Ч������·����"));
		return false;
	}
	return isbl;
}

void ipfModelerProcessChildDemGrossErrorCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		rasterName = map["rasterName"];
		errFile = map["errFile"];
		threshold = map["threshold"].toDouble();
	}
}

QMap<QString, QString> ipfModelerProcessChildDemGrossErrorCheck::getParameter()
{
	QMap<QString, QString> map;
	map["rasterName"] = rasterName;
	map["errFile"] = errFile;
	map["threshold"] = QString::number(threshold);

	return map;
}

void ipfModelerProcessChildDemGrossErrorCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	rasterName = map["rasterName"];
	errFile = map["errFile"];
	threshold = map["threshold"].toDouble();
}

void ipfModelerProcessChildDemGrossErrorCheck::run()
{
	clearOutFiles();
	clearErrList();

	QStringList outList;
	ipfGdalProgressTools gdal;

	foreach(QString var, filesIn())
	{
		QList<double> xylist;
		QString epsg;
		QString err;

		// ���DEM�ο�����ϵ
		QgsRasterLayer* rasterLayer = new QgsRasterLayer(var, "rasterLayer", "gdal");
		if (!rasterLayer->isValid())
		{
			addErrList(var + QStringLiteral(": ���ָ߳�ģ�Ͷ�ȡʧ�ܣ��޷�������"));
			continue;
		}
		QgsCoordinateReferenceSystem crs = rasterLayer->crs();
		epsg = crs.authid();
		RELEASE(rasterLayer);

		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡ���ָ߳�ģ��ʧ�ܣ��޷�������"));
			continue;
		}
		if (ogr.getBandSize() != 1)
		{
			addErrList(var + QStringLiteral(": ֻ֧�ֲ�����Ϊ1�����ָ߳�ģ�ͣ��޷�������"));
			continue;
		}

		// ���DEM������Χ
		xylist = ogr.getXY();

		// ����DEM������Χ�����ڲο�DEM�ϵ�����λ��
		int iRowLu = 0;
		int iColLu = 0;
		int iRowRd = 0;
		int iColRd = 0;
		err = gdal.locationPixelInfo(rasterName, epsg, xylist.at(0), xylist.at(1), iRowLu, iColLu);
		if (!err.isEmpty())
		{
			addErrList(var + QStringLiteral(": ���ָ߳�ģ����ο�DEMλ��ƥ��ʧ�ܣ�����ο�DEM�Ƿ�����ȫ���ǣ��޷�������"));
			continue;
		}
		err = gdal.locationPixelInfo(rasterName, epsg, xylist.at(2), xylist.at(3), iRowRd, iColRd);
		if (!err.isEmpty())
		{
			addErrList(var + QStringLiteral(": ���ָ߳�ģ����ο�DEMλ��ƥ��ʧ�ܣ�����ο�DEM�Ƿ�����ȫ���ǣ��޷�������"));
			continue;
		}
		QList<int> srcList;
		srcList << iColLu << iRowLu << iColRd - iColLu << iRowRd - iRowLu;

		// ���вο�DEM
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);
		err = gdal.proToClip_Translate_src(rasterName, target, srcList);
		if (!err.isEmpty())
		{
			addErrList(var + QStringLiteral(": ��ȡ�ο�DEMʧ�ܣ��޷�������"));
			continue;
		}

		// ����DEM��С��ο�DEM�ļ�ֵ
		double adfMinMax[2];
		double reAdfMinMax[2];
		CPLErr Cerr = ogr.getRasterBand(1)->ComputeRasterMinMax(FALSE, adfMinMax);

		ipfOGR ogrRe(target);
		if (!ogrRe.isOpen())
		{
			addErrList(target + QStringLiteral(": ��ȡ�ο����ָ߳�ģ��ʧ�ܣ��޷�������"));
			continue;
		}
		if (ogrRe.getBandSize() != 1)
		{
			addErrList(var + QStringLiteral(": ֻ֧�ֲ�����Ϊ1�����ָ߳�ģ�ͣ��޷�������"));
			continue;
		}
		CPLErr reCerr = ogrRe.getRasterBand(1)->ComputeRasterMinMax(FALSE, reAdfMinMax);

		if (Cerr != CE_None || reCerr != CE_None)
		{
			addErrList(var + QStringLiteral(": ���������Сֵʧ�ܣ��޷�������"));
			continue;
		}
		// ����DEM��С��ο�DEM�ļ�ֵ -- ����

		// �Ƚϲ�ֵ
		if (abs(adfMinMax[0] - reAdfMinMax[0]) > threshold ||
			abs(adfMinMax[1] - reAdfMinMax[1]) > threshold)
		{
			outList << var + QStringLiteral(": �߳�ģ�������Сֵ������ֵ------>ע���ʵ��");
		}
		else
			outList << var + QStringLiteral(": �߳�ģ�������Сֵδ������ֵ��");
	}

	printErrToFile(errFile, outList);
}

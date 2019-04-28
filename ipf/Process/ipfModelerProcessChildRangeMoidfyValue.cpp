#include "ipfModelerProcessChildRangeMoidfyValue.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ui/ipfModelerRangeMoidfyValueDialog.h"
#include "../ipfOgr.h"

ipfModelerProcessChildRangeMoidfyValue::ipfModelerProcessChildRangeMoidfyValue(QObject *parent, const QString modelerName)
	: ipfModelerProcessBase(parent, modelerName)
{
	setId(QUuid::createUuid().toString());

	dialog = new ipfModelerRangeMoidfyValueDialog();
}

ipfModelerProcessChildRangeMoidfyValue::~ipfModelerProcessChildRangeMoidfyValue()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildRangeMoidfyValue::checkParameter()
{
	clearErrList();

	if (!QFileInfo(vectorName).exists())
	{
		addErrList(QStringLiteral("ʸ���ļ�·����Ч��"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildRangeMoidfyValue::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		vectorName = map["vectorName"];
		value = map["value"].toDouble();
	}
}

QMap<QString, QString> ipfModelerProcessChildRangeMoidfyValue::getParameter()
{
	QMap<QString, QString> map;

	map["vectorName"] = vectorName;
	map["value"] = QString::number(value);

	return map;
}

void ipfModelerProcessChildRangeMoidfyValue::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	vectorName = map["vectorName"];
	value = map["value"].toDouble();
}

void ipfModelerProcessChildRangeMoidfyValue::run()
{
	clearOutFiles();
	clearErrList();

	// �ָ�ʸ��
	QStringList shpLists;
	if (!ipfOGR::splitShp(vectorName, shpLists))
	{
		addErrList(QStringLiteral("�ָ�ʸ������ʧ�ܡ�"));
		return;
	}

	ipfGdalProgressTools gdal;
	foreach(QString var, filesIn())
	{
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": ��ȡդ������ʧ�ܣ���������"));
			return;
		}
		double nodata = ogr.getNodataValue(1);

		// ��ʸ������դ��
		QStringList rasterList;
		foreach(QString shp, shpLists)
		{
			// ������з�Χ
			QgsRectangle rect;
			if (!ogr.shpEnvelope(vectorName, rect))
			{
				addErrList(vectorName + QStringLiteral(": ����ʸ����Χʧ�ܣ���������"));
				return;
			}
			int iRowLu = 0;
			int iColLu = 0;
			int iRowRd = 0;
			int iColRd = 0;
			QList<int> srcList;
			if (!ogr.Projection2ImageRowCol(rect.xMinimum(), rect.yMaximum(), iColLu, iRowLu)
				|| !ogr.Projection2ImageRowCol(rect.xMaximum(), rect.yMinimum(), iColRd, iRowRd))
			{
				addErrList(var + QStringLiteral(": ƥ����Ԫ����ʧ�ܣ��޷�������"));
				return;
			}
			srcList << iColLu << iRowLu << iColRd - iColLu << iRowRd - iRowLu;

			// ʹ��ʸ���ļ�����դ��
			QString target = ipfFlowManage::instance()->getTempVrtFile(var);
			QString err = gdal.AOIClip(var, target, shp);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
				continue;
			}

			// ������դ����С��Χ
			QString new_target = ipfFlowManage::instance()->getTempVrtFile(target);
			err = gdal.proToClip_Translate_src(target, new_target, srcList);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
				continue;
			}
			//rasterList << new_target;

			// �޸�դ����Ԫֵ
			QString new_new_target = ipfFlowManage::instance()->getTempVrtFile(new_target);
			err = gdal.pixelFillValue(new_target, new_new_target, nodata, value);
			if (!err.isEmpty())
			{
				addErrList(var + ": " + err);
				continue;
			}

			// ��Ƕ�ش��
			QString new_new_new_target = ipfFlowManage::instance()->getTempVrtFile(new_new_target);
			err = gdal.mosaic_Buildvrt(QStringList() << var << new_new_target, new_new_new_target);
			if (err.isEmpty())
				appendOutFile(new_new_new_target);
			else
				addErrList(var + ": " + err);
		}

		//// �޸�դ����Ԫֵ
		//QStringList fillList;
		//fillList << var;
		//foreach (QString var, rasterList)
		//{
		//	QString target = ipfFlowManage::instance()->getTempVrtFile(var);
		//	QString err = gdal.pixelFillValue(var, target, nodata, value);
		//	if (!err.isEmpty())
		//	{
		//		addErrList(var + ": " + err);
		//		continue;
		//	}
		//	fillList << target;
		//}

		//if (!fillList.isEmpty())
		//{
		//	// ��Ƕ�ش��
		//	QString target = ipfFlowManage::instance()->getTempVrtFile(var);
		//	QString err = gdal.mosaic_Buildvrt(fillList, target);
		//	if (err.isEmpty())
		//		appendOutFile(target);
		//	else
		//		addErrList(var + ": " + err);
		//}
	}
}

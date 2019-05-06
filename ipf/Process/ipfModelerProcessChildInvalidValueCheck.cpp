#include "ipfModelerProcessChildInvalidValueCheck.h"
#include "../../ui/ipfModelerInvalidValueCheckDialog.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfProgress.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"

ipfModelerProcessChildInvalidValueCheck::ipfModelerProcessChildInvalidValueCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerInvalidValueCheckDialog();
}

ipfModelerProcessChildInvalidValueCheck::~ipfModelerProcessChildInvalidValueCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildInvalidValueCheck::checkParameter()
{
	if (!QDir(saveName).exists())
	{
		addErrList(QStringLiteral("��Ч������ļ��С�"));
		return false;
	}
	return true;
}

void ipfModelerProcessChildInvalidValueCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();

		invalidValue = map["invalidValue"];
		saveName = map["saveName"];

		if (map["isNegative"] == "YES")
			isNegative = true;
		else
			isNegative = false;
		if (map["isNodata"] == "YES")
			isNodata = true;
		else
			isNodata = false;
		if (map["isShape"] == "YES")
			isShape = true;
		else
			isShape = false;
		if (map["bands_noDiffe"] == "YES")
			bands_noDiffe = true;
		else
			bands_noDiffe = false;
	}
}

QMap<QString, QString> ipfModelerProcessChildInvalidValueCheck::getParameter()
{
	QMap<QString, QString> map;

	map["invalidValue"] = invalidValue;
	map["saveName"] = saveName;

	if (isNegative)
		map["isNegative"] = "YES";
	else
		map["isNegative"] = "NO";
	if (isNodata)
		map["isNodata"] = "YES";
	else
		map["isNodata"] = "NO";
	if (isShape)
		map["isShape"] = "YES";
	else
		map["isShape"] = "NO";
	if (bands_noDiffe)
		map["bands_noDiffe"] = "YES";
	else
		map["bands_noDiffe"] = "NO";

	return map;
}

void ipfModelerProcessChildInvalidValueCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	invalidValue = map["invalidValue"];
	saveName = map["saveName"];

	if (map["isNegative"] == "YES")
		isNegative = true;
	else
		isNegative = false;
	if (map["isNodata"] == "YES")
		isNodata = true;
	else
		isNodata = false;
	if (map["isShape"] == "YES")
		isShape = true;
	else
		isShape = false;
	if (map["bands_noDiffe"] == "YES")
		bands_noDiffe = true;
	else
		bands_noDiffe = false;
}

void ipfModelerProcessChildInvalidValueCheck::run()
{
	clearOutFiles();
	clearErrList();

	QStringList outList;

	ipfGdalProgressTools gdal;
	gdal.setProgressSize(filesIn().size());
	gdal.showProgressDialog();

	foreach(QString var, filesIn())
	{
		gdal.pulsValueTatal();

		QString rasterFileName = QFileInfo(var).fileName();
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);

		QString err = gdal.filterInvalidValue(var, target, invalidValue, isNegative, isNodata, bands_noDiffe);
		if (err.isEmpty())
		{
			// ���������Сֵ
			ipfOGR ogr(target);
			if (!ogr.isOpen())
			{
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -2��"));
				continue;
			}

			CPLErr cerr = ogr.ComputeMinMax(IPF_ZERO);
			int nXSize = ogr.getYXSize().at(1);
			int nYSize = ogr.getYXSize().at(0);
			QString wkt = ogr.getProjection();
			ogr.close();

			if (cerr == CE_Warning)
			{
				// ���Ϊimg�ļ�
				QString format = "tif";
				QString targetTo = saveName + "\\" + removeDelimiter(target) + '.' + format;
				QString err = gdal.formatConvert(target, targetTo, gdal.enumFormatToString(format), "NONE", "NO", "0");
				if (!err.isEmpty())
				{
					addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -1��"));
					continue;
				}
				outList << rasterFileName + QStringLiteral(": ��鵽դ�������д�����Чֵ���������դ���б����Ϊ1��");

				// �Ƿ����ʸ��
				if (isShape)
				{
					// �ָ�դ������դ��תʸ����Ч�� ----->
					int nBlockSize = 1024;
					QStringList clipRasers;
					for (int i = 0; i < nYSize; i += nBlockSize)
					{
						for (int j = 0; j < nXSize; j += nBlockSize)
						{
							// ����ֿ�ʵ�ʴ�С
							int nXBK = nBlockSize;
							int nYBK = nBlockSize;

							//�������������ұߵĿ鲻����ʣ�¶��ٶ�ȡ����
							if (i + nBlockSize > nYSize)
								nYBK = nYSize - i;
							if (j + nBlockSize > nXSize)
								nXBK = nXSize - j;

							QList<int> srcList;
							srcList << j << i << nXBK << nYBK;

							ipfGdalProgressTools gdal;
							QString targetChild = ipfFlowManage::instance()->getTempVrtFile(var);
							QString err = gdal.proToClip_Translate_src(targetTo, targetChild, srcList);
							if (!err.isEmpty())
							{
								addErrList(rasterFileName + QStringLiteral(": �������ʸ��ʧ�ܣ���������"));
								continue;
							}
							else
								clipRasers << targetChild;
						}
					}
					// �ָ�դ������դ��תʸ����Ч�� -----<

					// ����ʸ��ͼ�� ----->
					QString vectorFile = targetTo.mid(0, targetTo.size()-3) + "shp";
					if (!ipfOGR::createrShape(vectorFile, QgsWkbTypes::Polygon, QgsFields(), wkt))
					{
						addErrList(rasterFileName + QStringLiteral(": ��������ʸ���ļ�ʧ�ܣ���������"));
						continue;
					}
					// ����ʸ���ļ� ------<

					// դ��תʸ�� ----->
					ipfGdalProgressTools gdal_v;
					gdal_v.setProgressTitle(QStringLiteral("��ȡʸ����Χ"));
					gdal_v.setProgressSize(clipRasers.size());
					gdal_v.showProgressDialog();
					for (int i = 0; i < clipRasers.size(); ++i)
					{
						QString clipRaster = clipRasers.at(i);
						QString err = gdal_v.rasterToVector(clipRaster, vectorFile, 0);
						if (!err.isEmpty())
						{
							addErrList(rasterFileName + QStringLiteral(": դ��תʸ��ʧ�ܣ���������"));
							continue;
						}
					}
					gdal_v.hideProgressDialog();
					// դ��תʸ�� -----<
				}
			}
			else if (cerr == CE_None)
				outList << rasterFileName + QStringLiteral(": ��ȷ��");
			else
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -4��"));
		}
		else
			addErrList(var + ": " + err);
	}

	QString outName = saveName + QStringLiteral("/��Чֵ���.txt");
	printErrToFile(outName, outList);
}

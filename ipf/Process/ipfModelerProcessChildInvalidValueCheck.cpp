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
		QString rasterFileName = QFileInfo(var).fileName();
		QString target = ipfFlowManage::instance()->getTempVrtFile(var);

		QString err = gdal.filterInvalidValue(var, target, invalidValue, isNegative, isNodata);
		if (err.isEmpty())
		{
			// ���������Сֵ
			double adfMinMax[2];
			ipfOGR ogr(target);
			if (!ogr.isOpen())
			{
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -2��"));
				gdal.pulsValueTatal();
				continue;
			}
			if (ogr.getBandSize() != 1)
			{
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -3��"));
				gdal.pulsValueTatal();
				continue;
			}
			CPLErr cerr = ogr.getRasterBand(1)->ComputeRasterMinMax(FALSE, adfMinMax);
			int nXSize = ogr.getYXSize().at(1);
			int nYSize = ogr.getYXSize().at(0);
			QString wkt = ogr.getProjection();
			ogr.close();

			if (cerr != CE_None)
			{
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -4��"));
				gdal.pulsValueTatal();
				continue;
			}

			// ����դ���Ƿ������Чֵ
			if (adfMinMax[0] == 0 && adfMinMax[1] == 0)
			{
				outList << rasterFileName + QStringLiteral(": ��ȷ��");
				gdal.pulsValueTatal();
			}
			else
			{
				// ���Ϊimg�ļ�
				QString format = "img";
				QString targetTo = saveName + "\\" + removeDelimiter(target) + '.' + format;
				QString err = gdal.formatConvert(target, targetTo, gdal.enumFormatToString(format), "NONE", "NO", "0");
				if (!err.isEmpty())
				{
					addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -1��"));
					gdal.pulsValueTatal();
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
								gdal.pulsValueTatal();
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
						gdal.pulsValueTatal();
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
							gdal.pulsValueTatal();
							continue;
						}
					}
					gdal_v.hideProgressDialog();
					// դ��תʸ�� -----<
				}
			}
		}
		else
		{
			addErrList(var + ": " + err);
			gdal.pulsValueTatal();
		}
	}

	QString outName = saveName + QStringLiteral("/��Чֵ���.txt");
	QFile file(outName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		addErrList(outName + QStringLiteral("���������ļ�ʧ�ܣ�����ֹ��"));
		return;
	}
	QTextStream out(&file);
	foreach(QString str, outList)
		out << str << endl;
	file.close();
}

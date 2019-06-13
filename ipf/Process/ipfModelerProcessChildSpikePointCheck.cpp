#include "ipfModelerProcessChildSpikePointCheck.h"
#include "../../ui/ipfModelerSpikePointCheckDialog.h"
#include "ipfFlowManage.h"
#include "../gdal/ipfgdalprogresstools.h"
#include "../ipfOgr.h"

#include <QProgressDialog>

ipfModelerProcessChildSpikePointCheck::ipfModelerProcessChildSpikePointCheck(QObject * parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerSpikePointCheckDialog();
}

ipfModelerProcessChildSpikePointCheck::~ipfModelerProcessChildSpikePointCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildSpikePointCheck::checkParameter()
{
	//if (threshold < 0)
	//{
	//	addErrList(QStringLiteral("��������ӦС��0��"));
	//	return false;
	//}
	return true;
}

void ipfModelerProcessChildSpikePointCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		threshold = map["threshold"].toDouble();
		saveName = map["saveName"];
	}
}

QMap<QString, QString> ipfModelerProcessChildSpikePointCheck::getParameter()
{
	QMap<QString, QString> map;

	map["threshold"] = QString::number(threshold);
	map["saveName"] = saveName;
	return map;
}

void ipfModelerProcessChildSpikePointCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	threshold = map["threshold"].toDouble();
	saveName = map["saveName"];
}

void ipfModelerProcessChildSpikePointCheck::run()
{
	clearOutFiles();
	clearErrList();

	QStringList outList;
	ipfGdalProgressTools gdal;

	//������
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("���ݴ���..."), QStringLiteral("ȡ��"), 0, filesIn().size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("���ݴ���"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	foreach(QString var, filesIn())
	{
		dialog.setValue(++prCount);
		QApplication::processEvents();

		QString rasterFileName = QFileInfo(var).fileName();
		QString target = saveName + "\\" + rasterFileName;
		QString err = gdal.stdevp3x3Alg(var, target, threshold);

		if (err.isEmpty())
		{
			// ���������Сֵ
			ipfOGR ogr(target);
			if (!ogr.isOpen())
			{
				addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -2��"));
				continue;
			}

			QString wkt = ogr.getProjection();

			//if (cerr == CE_Warning)
			//{
			//	// ����ļ�
			//	QString format = "tif";
			//	QString targetTo = saveName + "\\" + removeDelimiter(target) + '.' + format;
			//	QString err = gdal.formatConvert(target, targetTo, gdal.enumFormatToString(format), "NONE", "NO", "0");
			//	if (!err.isEmpty())
			//	{
			//		addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -1��"));
			//		continue;
			//	}
			//	outList << rasterFileName + QStringLiteral(": ��鵽դ�������д������㣬�������դ���б����Ϊ1��");

				// �Ƿ����ʸ��
				if (true)
				{
					// �ָ�դ������դ��תʸ����Ч��
					QStringList clipRasers;
					if (!ogr.splitRaster(1024, clipRasers))
					{
						addErrList(rasterFileName + QStringLiteral(": �������ʸ��ʧ�ܣ���������"));
						continue;
					}

					// ����ʸ��ͼ�� ----->
					QString vectorFile = target.mid(0, target.size() - 3) + "shp";
					if (!ipfOGR::createrVectorlayer(vectorFile, QgsWkbTypes::Polygon, QgsFields(), wkt))
					{
						addErrList(rasterFileName + QStringLiteral(": ��������ʸ���ļ�ʧ�ܣ���������"));
						continue;
					}

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
							addErrList(rasterFileName + ": " + err);
							continue;
						}
					}
					// դ��תʸ�� -----<
				}
			//}
			//else if (cerr == CE_None)
			//	outList << rasterFileName + QStringLiteral(": ��ȷ��");
			//else
			//	addErrList(rasterFileName + QStringLiteral(": ��������ʧ�ܣ������к˲������ -4��"));
		}
		else
			addErrList(var + ": " + err);
	}
	//QString outName = QStringLiteral("d:/���jj.txt");
	//printErrToFile(outName, outList);
}

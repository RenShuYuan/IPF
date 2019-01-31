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
	if (dialog) { delete dialog; }
}

bool ipfModelerProcessChildInvalidValueCheck::checkParameter()
{
	bool isbl = true;

	if (saveName.isEmpty())
	{
		isbl = false;
		addErrList(QStringLiteral("无效的输出文件夹。"));
	}
	else
	{
		QDir dir(saveName);
		if (!dir.exists())
		{
			isbl = false;
			addErrList(QStringLiteral("无效的输出文件夹。"));
		}
	}

	return isbl;
}

void ipfModelerProcessChildInvalidValueCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();

		saveName = map["saveName"];
		invalidValue = map["invalidValue"];

		if (map["isNegative"] == "YES")
			isNegative = true;
		else
			isNegative = false;
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

	return map;
}

void ipfModelerProcessChildInvalidValueCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);
	saveName = map["saveName"];
	invalidValue = map["invalidValue"];

	if (map["isNegative"] == "YES")
		isNegative = true;
	else
		isNegative = false;
}

void ipfModelerProcessChildInvalidValueCheck::run()
{
	clearOutFiles();
	clearErrList();
	QStringList errList;

	// 分割特定无效值
	QStringList valueList;
	if (!invalidValue.isEmpty())
		valueList = invalidValue.split('@', QString::SkipEmptyParts);

	ipfProgress proDialog;
	proDialog.setRangeTotal(0, filesIn().size());
	proDialog.show();

	foreach(QString var, filesIn())
	{
		// 打开栅格
		ipfOGR ogr(var);
		if (!ogr.isOpen())
		{
			addErrList(var + QStringLiteral(": 读取栅格数据失败，无法继续。"));
			continue;
		}

		/*
		// 创建输出栅格
		QString file = "d:\\1.img";
		GDALDataset* poDataset_target = ogr.createNewRaster(file);
		if (!poDataset_target)
		{
			addErrList(var + QStringLiteral(": 创建输出栅格数据失败，无法继续。"));
			continue;
		}
		*/

		int nBands = ogr.getBandSize();
		int nXSize = ogr.getYXSize().at(1);
		int nYSize = ogr.getYXSize().at(0);
		double nodata = ogr.getNodataValue(1);

		// 分块参数
		int nBlockSize = 512;
		long blockSize = nBlockSize * nBlockSize * nBands;
		double *pSrcData = new double[blockSize];

		// 计算进度条值
		int proX = 0;
		int proY = 0;
		if (nXSize % nBlockSize == 0)
			proX = nXSize / nBlockSize;
		else
			proX = nXSize / nBlockSize + 1;
		if (nYSize % nBlockSize == 0)
			proY = nYSize / nBlockSize;
		else
			proY = nYSize / nBlockSize + 1;
		proDialog.setRangeChild(0, proX * proY);

		//循环分块并进行处理
		int proCount = 0;
		bool isbl = true;
		for (int i = 0; i < nYSize; i += nBlockSize)
		{
			for (int j = 0; j < nXSize; j += nBlockSize)
			{
				// 保存分块实际大小
				int nXBK = nBlockSize;
				int nYBK = nBlockSize;

				//如果最下面和最右边的块不够，剩下多少读取多少
				if (i + nBlockSize > nYSize)
					nYBK = nYSize - i;
				if (j + nBlockSize > nXSize)
					nXBK = nXSize - j;

				long size = nYBK * nXBK * nBands;

				// 读取原始图像块
				if (!ogr.readRasterIO(pSrcData, j, i, nXBK, nYBK, GDT_Float64))
				{
					addErrList(var + QStringLiteral(": 读取栅格分块数据失败，已跳过。"));
					isbl = false;
					break;
				}

				// 处理算法
				for (long mi = 0; mi < size; ++mi)
				{
					if (isNegative)
					{
						if (pSrcData[mi] < 0)
						{
							errList << var + QStringLiteral(": 该栅格存在负值。");
							isbl = false;
						}
					}

					foreach(QString str, valueList)
					{
						if (str == "nodata")
						{
							if (pSrcData[mi] == nodata)
							{
								errList << var + QStringLiteral(": 该栅格存在无效值--> ") + invalidValue;
								isbl = false;
							}
						}
						else
						{
							double value = pSrcData[mi];
							double value1 = str.toDouble();
							if (value == value1)
							{
								errList << var + QStringLiteral(": 该栅格存在无效值--> ") + str;
								isbl = false;
							}
						}
						if (!isbl) break;
					}
					if (!isbl) break;
				}
				if (!isbl) break;

				proDialog.setValue(++proCount);
				QApplication::processEvents();
				if (proDialog.wasCanceled())
					return;
			}
			if (!isbl)
			{
				proDialog.pulsValueTatal();
				break;
			}
		}

		//释放申请的内存
		delete[] pSrcData; pSrcData = 0;
	}

	QString outName = saveName + QStringLiteral("/栅格无效值检查.txt");
	QFile file(outName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		addErrList(outName + QStringLiteral("创建错误文件失败，已终止。"));
		return;
	}
	QTextStream out(&file);
	foreach(QString str, errList)
		out << str << endl;
	file.close();
}

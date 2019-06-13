#include "ipfFractalmanagement.h"
#include "qgsapplication.h"

ipfFractalManagement::ipfFractalManagement(QObject *parent) : QObject(parent)
{
    // 设置源坐标系 WGS 84
	QgsCoordinateReferenceSystem mSourceCrs;
    mSourceCrs.createFromId(4490);
    ipfPrj.setSourceCrs(mSourceCrs);
}

ipfFractalManagement::ipfFractalManagement(const int blc, QObject *parent) : QObject(parent)
{
	// 设置源坐标系 WGS 84
	QgsCoordinateReferenceSystem mSourceCrs;
	mSourceCrs.createFromId(4490);
	ipfPrj.setSourceCrs(mSourceCrs);

	setBlc(blc);
}

bool ipfFractalManagement::effectiveness(const QString & th)
{
	QString strName = th;

	//检查图号位数是否正确
	if ((strName.size() != 10) && (strName.size() != 11))
		return false;

	if (strName.size() == 11)
	{
		QChar hemisphere = strName.at(0);
		if ((hemisphere == 'S') || (hemisphere == 'N'))
			strName = strName.remove(0, 1);
		else
			return false;
	}

	//检查图号的百万行号是否是字母
	QChar row = strName.at(0);
	if (!row.isLetter()
		|| row.toLower() == 'w'
		|| row.toLower() == 'x'
		|| row.toLower() == 'y'
		|| row.toLower() == 'z')
		return false;

	//检查图号的百万行号是否正确
	int col = strName.mid(1, 2).toInt();
	if (!(col >= 1 && col <= 60))
		return false;

	//检查比例尺正确性
	QChar scale = strName.at(3);
	if (!checkBlcAbb(scale))
		return false;

	//检查图幅的行列号是否正确
	int ranks = getRanks(scale);
	int rowNo = strName.mid(4, 3).toInt();
	int colNo = strName.mid(7, 3).toInt();
	if (rowNo > 0 && rowNo <= ranks && colNo > 0 && colNo <= ranks)
		return true;

	return false;
}

QList<QgsPointXY> ipfFractalManagement::dNToLal(const QString &strName)
{
	QString dNStr = strName;

	if (!effectiveness(dNStr))
	{
		errList.append(dNStr + QStringLiteral(": 图号不正确。"));
		return QList<QgsPointXY>();
	}

	// 提取南北半球标识符
	QChar hemisphere = 'N';
	if (dNStr.size() == 11)
	{
		hemisphere = dNStr.at(0);
		dNStr = dNStr.remove(0, 1);
	}

    int a=0, b=0, c=0, d=0;

    //得到百万图号的纬度
	QByteArray ba = dNStr.toLatin1();
	char ch = ba[0];
    int itmp=0;
    if(islower(ch))
        itmp=97;
    else
        itmp=65;

    for(int i=1;i<22;++i)
    {
        if(ch==itmp){
            a=i;
            break;
        }
        ++itmp;
    }

    //得到百万图号的经度
    QString tempStr = dNStr.mid(1,2);
    b = tempStr.toInt();

    //图幅的行号
    tempStr = dNStr.mid(4,3);
    c = tempStr.toInt();

    //图幅的列号
    tempStr = dNStr.mid(7,3);
    d = tempStr.toInt();

    //根据比例尺得到经差与纬差
    double djc = mJwc.djc/60;
    double dwc = mJwc.dwc/60;

    //计算出西南角的经纬度,十进制表示
    double djd= ((b-31)*360+(d-1)*djc)/60;
	double dwd = 0;
	if (hemisphere == 'S')
	{
		a = 0 - a;
		dwd = (a * 240 + (240 / dwc - c)*dwc) / 60;
	}
	else
	{
		dwd = ((a - 1) * 240 + (240 / dwc - c)*dwc) / 60;
	}

	QList<QgsPointXY> list;
    list << QgsPointXY( djd, dwd )                //西南
         << QgsPointXY( djd, dwd+dwc/60 )         //西北
         << QgsPointXY( djd+djc/60, dwd+dwc/60 )  //东北
         << QgsPointXY( djd+djc/60, dwd );        //东南

    return list;
}

QList<QgsPointXY> ipfFractalManagement::dNToXy(const QString &dNStr)
{
	if (!effectiveness(dNStr))
	{
		errList.append(dNStr + QStringLiteral(": 图号不正确。"));
		return QList<QgsPointXY>();
	}

    QList<QgsPointXY> pointsOut;
    QList<QgsPointXY> points = dNToLal(dNStr);
	if (points.size() != 4)
	{
		errList.append(dNStr + QStringLiteral(": 角点地理坐标计算失败。"));
		return pointsOut;
	}

	ipfProjectionTransformation::errType err = ipfPrj.createTargetCrs(points.at(0));
	if (err != ipfProjectionTransformation::eOK)
	{
		errList.append(dNStr + QStringLiteral(": 创建投影坐标系失败。"));
        return pointsOut;
	}

	QgsPointXY p1 = points.at(0);
	QgsPointXY p2 = points.at(1);
	QgsPointXY p3 = points.at(2);
	QgsPointXY p4 = points.at(3);

	if (ipfPrj.prjTransform(p1) != ipfProjectionTransformation::eOK
		|| ipfPrj.prjTransform(p2) != ipfProjectionTransformation::eOK
		|| ipfPrj.prjTransform(p3) != ipfProjectionTransformation::eOK
		|| ipfPrj.prjTransform(p4) != ipfProjectionTransformation::eOK)
	{
		errList.append(dNStr + QStringLiteral(": 地理坐标转投影坐标失败。"));
		return pointsOut;
	}

	pointsOut << p1 << p2 << p3 << p4;
    return pointsOut;
}

QString ipfFractalManagement::pointToTh(const QgsPointXY p)
{
    // 保存计算的图号各部分
    int a=0, b=0, c=0, d=0;
    QgsPointXY mP = p;

    //计算出1:100万图幅所在的字符对应的数字码
    a = mP.x()/6+31;
    b = mP.y()/4+1;

    //将十进制经纬度转换为度分秒,并全部转换为秒方便计算
    ipfProjectionTransformation::sjzToDfm( mP );
    ipfProjectionTransformation::dfmToMM(mP);

    //计算出1:100万图号后的行、列号
    c=(4*3600)/mJwc.dwc-(int)(((int)mP.y()%(4*3600))/mJwc.dwc);
    d=(int)(((int)mP.x()%(6*3600))/mJwc.djc)+1;

    //=============组合成完整标准图号=============//
    QString th;
    //百万行号
    char ch='A';
    for (int i = 1; i < 22; ++i)
    {
        if( b==i )
        {
            th = ch;
            break;
        }
        else
            ++ch;
    }

    //百万列号
    QString tmpStr = QString::number(a);
    if (tmpStr.size()==1)
        th += tmpStr.prepend("0");
    else
        th += tmpStr;

    //比例尺
    th += mJwc.blcAbb;

    //行号及列号
    tmpStr = QString::number(c);
    fill0_two(tmpStr);
    th += tmpStr;

    tmpStr = QString::number(d);
    fill0_two(tmpStr);
    th += tmpStr;

    return th;
}

QStringList ipfFractalManagement::rectToTh(const QgsPointXY lastPoint, const QgsPointXY nextPoint)
{
    QStringList thList;
    QgsPointXY lastPointD;
    QgsPointXY nextPointD;

    // 如果是投影坐标系，则转换为经纬度
    if (ipfPrj.sourceCrs().isGeographic())
	{
        if (checkLBisExtent(lastPoint) && checkLBisExtent(nextPoint))
        {
            lastPointD = lastPoint;
            nextPointD = nextPoint;
        }
        else
        {
            return thList;
        }
    }
    else
    {
        lastPointD = ipfPrj.transform(lastPoint);
        nextPointD = ipfPrj.transform(nextPoint);
    }

    qDebug() << "lastPointD=" << lastPointD.toString() << ", nextPointD=" << nextPointD.toString(); ///

    // 分开保存最大经度、纬度，与最小经度、纬度
    QgsPointXY maxPoint, minPoint;
    maxPoint.set( lastPointD.x()>nextPointD.x() ? lastPointD.x():nextPointD.x(),
                  lastPointD.y()>nextPointD.y() ? lastPointD.y():nextPointD.y());
    minPoint.set( lastPointD.x()<nextPointD.x() ? lastPointD.x():nextPointD.x(),
                  lastPointD.y()<nextPointD.y() ? lastPointD.y():nextPointD.y());

    qDebug() << "4.开始生成单幅图框";///
    QString maxTh = pointToTh(maxPoint);
    QString minTh = pointToTh(minPoint);
    qDebug() << "4.结束生成单幅图框" << maxTh << " : " << minTh ;///
    // 如果两幅图一致则返回
    if (maxTh == minTh)
    {
        return thList << maxTh;
    }

    QString minThRow = minTh;
    while (true)
    {
        qDebug() << "4.开始遍历行";///
        // 遍历行
        if ( (maxTh.at(0)!=minTh.at(0)) || (maxTh.mid(4,3)!=minTh.mid(4,3)) )
        {
            thList << minTh;
            qDebug() << minTh;///
            int row = (minTh.mid(4,3)).toInt();
            if (row > 1)
            {
                QString tmpStr = QString::number(--row);
                if (tmpStr.size()==1)
                    minTh.replace(4, 3, "00"+tmpStr);
                else if (tmpStr.size()==2)
                    minTh.replace(4, 3, "0"+tmpStr);
                else
                    minTh.replace(4, 3, tmpStr);
            }
            else
            {
                QByteArray ba = minTh.toLatin1();
                char ch = ba[0];
                minTh.replace(0, 1, ++ch);
                minTh.replace(4, 3, "192");		//   有问题，应该根据比例尺返回
            }
        }
        else
        {
            qDebug() << "4.开始遍历列";///
            thList << minTh;

            if ( (maxTh.mid(1,2)!=minTh.mid(1,2)) || (maxTh.mid(7,3)!=minTh.mid(7,3)) )
            {
                int column = (minTh.mid(7,3)).toInt();
                if (column < mJwc.ranks)
                {
                    QString tmpStr = QString::number(++column);
                    if (tmpStr.size()==1)
                        minTh.replace(7, 3, "00"+tmpStr);
                    else if (tmpStr.size()==2)
                        minTh.replace(7, 3, "0"+tmpStr);
                    else
                        minTh.replace(7, 3, tmpStr);
                }
                else
                {
                    int iColumn = (minTh.mid(1,2)).toInt();
                    QString tmpStr = QString::number(++iColumn);
                    if (tmpStr.size()==1)
                        minTh.replace(1, 2, "0"+tmpStr);
                    else if (tmpStr.size()==2)
                        minTh.replace(1, 2, tmpStr);
                    minTh.replace(7, 3, "001");
                }

                minTh.replace(0, 1, minThRow.mid(0, 1));
                minTh.replace(4, 3, minThRow.mid(4, 3));
            }
            else
                break;
        }
    }

    return thList;
}

void ipfFractalManagement::setBlc(const int blc)
{
    this->mBlc = blc;						//设置比例尺
	setBlcAbb();							//设置比例尺对应字母
    setJwc();								//设置经差与纬差
    mJwc.ranks = getRanks(mJwc.blcAbb);		//设置行列最大值
}

int ipfFractalManagement::getScale(const int index)
{
    switch (index) {
    case 0:
        return 5000;
    case 1:
        return 10000;
    case 2:
        return 25000;
    case 3:
        return 50000;
    case 4:
        return 100000;
    case 5:
        return 250000;
    case 6:
        return 500000;
    default:
        return 10000;
    }
}

QString ipfFractalManagement::getAdjacentFrac(const QString &strName, const int possition)
{
	if (!effectiveness(strName)) return QString();

	QString newTh = strName;
	int ranks = getRanks(newTh.at(4));

	if (newTh.size() != 11)
		newTh = QString("N") + strName;

	if (possition == 1)
	{
		newTh = getAdjacentFrac(newTh, 2);
		newTh = getAdjacentFrac(newTh, 4);
	}
	else if (possition == 2)
	{
		int row = (newTh.mid(5, 3)).toInt();
		if (row > 1)
		{
			QString tmpStr = QString::number(--row);
			fill0_three(tmpStr);
			newTh.replace(5, 3, tmpStr);
		}
		else
		{
			QString tmpStr = QString::number(ranks);
			fill0_three(tmpStr);
			newTh.replace(5, 3, tmpStr);

			if (newTh.at(1).toLower() == 'a' && newTh.at(0).toLower() == 's')
			{
				newTh.replace(0, 1, "N");
			}
			else
			{
				char ch = newTh.at(1).toLatin1();
				newTh.replace(1, 1, ++ch);
			}
		}
	}
	else if (possition == 3)
	{
		newTh = getAdjacentFrac(newTh, 2);
		newTh = getAdjacentFrac(newTh, 5);
	}
	else if (possition == 4)
	{
		int column = (newTh.mid(8, 3)).toInt();
		if (column > 1)
		{
			QString tmpStr = QString::number(--column);
			fill0_three(tmpStr);
			newTh.replace(8, 3, tmpStr);
		}
		else
		{
			int iColumn = (newTh.mid(2, 2)).toInt();
			QString tmpStr = QString::number(--iColumn);
			fill0_two(tmpStr);
			newTh.replace(2, 2, tmpStr);
			QString bwColumn = QString::number(ranks);
			fill0_three(bwColumn);
			newTh.replace(8, 3, bwColumn);
		}
	}
	else if (possition == 5)
	{
		int column = (newTh.mid(8, 3)).toInt();
		if (column < ranks)
		{
			QString tmpStr = QString::number(++column);
			fill0_three(tmpStr);
			newTh.replace(8, 3, tmpStr);
		}
		else
		{
			int iColumn = (newTh.mid(2, 2)).toInt();
			QString tmpStr = QString::number(++iColumn);
			fill0_two(tmpStr);
			newTh.replace(2, 2, tmpStr);
			newTh.replace(8, 3, "001");
		}
	}
	else if (possition == 6)
	{
		newTh = getAdjacentFrac(newTh, 4);
		newTh = getAdjacentFrac(newTh, 7);
	}
	else if (possition == 7)
	{
		int row = (newTh.mid(5, 3)).toInt();
		if (row < ranks)
		{
			QString tmpStr = QString::number(++row);
			fill0_three(tmpStr);
			newTh.replace(5, 3, tmpStr);
		}
		else
		{
			newTh.replace(5, 3, "001");

			if (newTh.at(1).toLower() == 'a' && newTh.at(0).toLower() == 'n')
			{
				newTh.replace(0, 1, "S");
			}
			else
			{
				char c = newTh.at(1).toLatin1();
				newTh.replace(1, 1, --c);
			}
		}
	}
	else if (possition == 8)
	{
		newTh = getAdjacentFrac(newTh, 5);
		newTh = getAdjacentFrac(newTh, 7);
	}

	return newTh;
}

QList<double> ipfFractalManagement::external(QList<QgsPointXY>& four, const double R, const int ext, const bool isCenter)
{
	// 随便赋个值，方便下面比较
	double xMin = four.at(0).x();
	double xMax = four.at(0).x();
	double yMin = four.at(0).y();
	double yMax = four.at(0).y();

	QList<double> list;
	for (int i = 0; i < four.size(); ++i)
	{
		QgsPointXY p = four.at(i);
		xMin = xMin > p.x() ? p.x() : xMin;
		xMax = xMax < p.x() ? p.x() : xMax;
		yMin = yMin > p.y() ? p.y() : yMin;
		yMax = yMax < p.y() ? p.y() : yMax;
	}


	//xMin = ((long)(xMin / R))*R - ext * R;
	//yMin = ((long)(yMin / R))*R - ext * R;
	//xMax = (((long)(xMax / R)) + 1)*R + ext * R;
	//yMax = (((long)(yMax / R)) + 1)*R + ext * R;
	xMin = ((long)((xMin - ext) / R)) * R;
	yMin = ((long)((yMin - ext) / R)) * R;
	xMax = ((long)((xMax + ext) / R)) * R;
	yMax = ((long)((yMax + ext) / R)) * R;

	if (isCenter)
	{
		double midSize = R / 2;
		xMin = xMin - midSize;
		yMin = yMin - midSize;
		xMax = xMax + midSize;
		yMax = yMax + midSize;
	}

	list << xMin << yMax << xMax << yMin;
	return list;
}

void ipfFractalManagement::setJwc()
{
    if (mBlc==500000)
    {
        mJwc.djc = 10800;
        mJwc.dwc = 7200;
    }
    else if (mBlc==250000)
    {
        mJwc.djc=5400;
        mJwc.dwc=3600;
    }
    else if (mBlc==100000)
    {
        mJwc.djc=1800;
        mJwc.dwc=1200;
    }
    else if (mBlc==50000)
    {
        mJwc.djc=900;
        mJwc.dwc=600;
    }
    else if (mBlc==25000)
    {
        mJwc.djc=450;
        mJwc.dwc=300;
    }
    else if (mBlc==10000)
    {
        mJwc.djc=225;
        mJwc.dwc=150;
    }
    else if (mBlc==5000)
    {
        mJwc.djc=112.5;
        mJwc.dwc=75.0;
    }
    else
    {
        mJwc.djc=0;
        mJwc.dwc=0;
    }
}

void ipfFractalManagement::setBlcAbb()
{
    if( mBlc==500000 )
        mJwc.blcAbb = 'B';
    else if( mBlc==250000 )
        mJwc.blcAbb = 'C';
    else if( mBlc==100000 )
        mJwc.blcAbb = 'D';
    else if( mBlc==50000 )
        mJwc.blcAbb = 'E';
    else if( mBlc==25000 )
        mJwc.blcAbb = 'F';
    else if( mBlc==10000 )
        mJwc.blcAbb = 'G';
    else if( mBlc==5000 )
        mJwc.blcAbb = 'H';
}

bool ipfFractalManagement::checkBlcAbb(const QChar c)
{
	QChar cc = c.toUpper();
	if (cc == 'B' || cc == 'C' || cc == 'D'
		|| cc == 'E' || cc == 'F' || cc == 'G' || cc == 'H')
	{
		return true;
	}
	return false;
}

int ipfFractalManagement::getRanks(const QChar c)
{
	QChar cc = c.toUpper();
	if (cc == 'H')
		return 192;
	else if (cc == 'G')
		return 96;
	else if (cc == 'F')
		return 48;
	else if (cc == 'E')
		return 24;
	else if (cc == 'D')
		return 12;
	else if (cc == 'C')
		return 4;
	else if (cc == 'B')
		return 2;
	else
		return 0;
}

void ipfFractalManagement::fill0_two(QString &str)
{
	if (str.size() == 1)
	{
		str = str.prepend("00");
	}
}

void ipfFractalManagement::fill0_three(QString & str)
{
	if (str.size() == 1)
	{
		str = str.prepend("00");
	}
	else if (str.size() == 2)
	{
		str = str.prepend("0");
	}
}

bool ipfFractalManagement::setGCS()
{   
    QgsCoordinateReferenceSystem crs;
    crs = ipfPrj.getGCS(ipfPrj.sourceCrs());

    if ( crs.isValid() )
    {
        ipfPrj.setDestinationCrs(crs);
        return true;
    }
    else
        return false;
}

bool ipfFractalManagement::checkLBisExtent(const QgsPointXY &point)
{
    if (point.x()>=-180 && point.x()<=180 &&
        point.y()>=-90 && point.y()<=90)
    {
        return true;
    }
    else
    {
        return false;
    }
}
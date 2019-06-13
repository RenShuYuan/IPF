#include <QDir>
#include <QFileInfo>

#include "ipfProjectionTransformation.h"
#include "qgsmessagebar.h"
#include "sqlite3.h"

ipfProjectionTransformation::ipfProjectionTransformation()
    : QgsCoordinateTransform()
{

}

ipfProjectionTransformation::ipfProjectionTransformation(
        const QgsCoordinateReferenceSystem& theSource,
        const QgsCoordinateReferenceSystem& theDest) :
        QgsCoordinateTransform(theSource, theDest)
{
	
}

bool ipfProjectionTransformation::isValidGCS(const QString &authid)
{
    if ((authid == "EPSG:4326") ||   // WGS 1984
		(authid == "EPSG:4490") ||   // CGCS2000
        (authid == "EPSG:4214") ||   // beijing1954
        (authid == "EPSG:4610"))    // xian1980
    {
       return true;
    }
    return false;
}

bool ipfProjectionTransformation::isValidPCS(const int postgisSrid)
{
    if (!(isSourceBeijing1954Prj(postgisSrid) ||
		  isSourceWGS1984Prj(postgisSrid) ||
          isSourceCGCS2000Prj(postgisSrid) ||
          isSourceXian1980Prj(postgisSrid)))
    {
        return false;
    }
    else
    {
        return true;
    }
}

ipfProjectionTransformation::errType ipfProjectionTransformation::prjTransform(QgsPointXY &p)
{
    if (!sourceCrs().isValid() || !destinationCrs().isValid())
        return ipfProjectionTransformation::eSouDestInvalid;

    // 坐标转换
    p = transform(p);
	return ipfProjectionTransformation::eOK;
}

bool ipfProjectionTransformation::isSourceWGS1984Prj(const int postgisSrid)
{
	if (postgisSrid > 32610 && postgisSrid < 32709)
	{
		return true;
	}
	return false;
}

bool ipfProjectionTransformation::isSourceCGCS2000Prj(const int postgisSrid)
{
    if (postgisSrid > 4491 && postgisSrid < 4554)
    {
        return true;
    }
    return false;
}

bool ipfProjectionTransformation::isSourceXian1980Prj(const int postgisSrid)
{
    if (postgisSrid > 2327 && postgisSrid < 2390)
    {
        return true;
    }
    return false;
}

bool ipfProjectionTransformation::isSourceBeijing1954Prj(const int postgisSrid)
{
    if (postgisSrid > 2401 && postgisSrid < 2442)
    {
        return true;
    }
    return false;
}

void ipfProjectionTransformation::sjzToDfm(QgsPointXY &p)
{
    double L = p.x();
    double B = p.y();
    sjzToDfm(L);
    sjzToDfm(B);
    p.set(L, B);
}

void ipfProjectionTransformation::sjzToDfm(double &num)
{
    double tmj, tmf, tmm;
    tmj = (int)num;
    tmf = (int)((num-tmj)*60);
    tmm = ((num-tmj)*60-tmf)*60;
    num = tmj + tmf/100 + tmm/10000;
}

void ipfProjectionTransformation::dfmToMM(QgsPointXY &p)
{
    double L = p.x();
    double B = p.y();

    double Lj, Lf, Lm;
    double Bj, Bf, Bm;

    Lj = (int)L;
    Lf = (int)((L-Lj)*100);
    Lm = ((L-Lj)*100-Lf)*100;

    Bj = (int)B;
    Bf = (int)((B-Bj)*100);
    Bm = ((B-Bj)*100-Bf)*100;

    p.setX(Lj*3600+Lf*60+Lm);
    p.setY(Bj*3600+Bf*60+Bm);
}

int ipfProjectionTransformation::getBandwidth3(double djd)
{
    int itmp=(djd-1.5)/3+1;
    return itmp;
}

int ipfProjectionTransformation::getCentralmeridian3(double djd)
{
    int itmp=(djd-1.5)/3+1;
    return itmp*3;
}

int ipfProjectionTransformation::getWgs84Bandwidth(double djd)
{
	return (int)((djd / 6) + 31);
}

ipfProjectionTransformation::errType ipfProjectionTransformation::createTargetCrs(const QgsPointXY point)
{
    // 验证源参照坐标系
    if (!sourceCrs().isValid())
        return ipfProjectionTransformation::eSourceCrsErr;

    // 检查输入的是否是常用的地理坐标系
    if (!isValidGCS(sourceCrs().authid()))
        return ipfProjectionTransformation::eNotSupportGCS;

    QgsCoordinateReferenceSystem mTargetCrs;

	long id = 0;
	if (this->sourceCrs().authid() == "EPSG:4326")
		id = getPCSauthid_Wgs84Gcs(point);
	else if (this->sourceCrs().authid() == "EPSG:4490")
		id = getPCSauthid_CGCS2000(point, 3);
	mTargetCrs.createFromId(id);

	if (!mTargetCrs.isValid())
		return ipfProjectionTransformation::eDestCrsErr;

    setDestinationCrs(mTargetCrs);

	return ipfProjectionTransformation::eOK;
}

QgsCoordinateReferenceSystem ipfProjectionTransformation::getGCS(const QgsCoordinateReferenceSystem &sourceCrs)
{
    QgsCoordinateReferenceSystem crs;

    // 判断源参照坐标系是否是支持的地理坐标系
    int postgisSrid = sourceCrs.postgisSrid();
    if ( isValidPCS(postgisSrid) )
    {
        int iEPSG = 0;
        // 判断对应的是哪个投影坐标系
        if (isSourceBeijing1954Prj(postgisSrid))
            iEPSG = 4214;
        else if (isSourceXian1980Prj(postgisSrid))
            iEPSG = 4610;
        else if (isSourceCGCS2000Prj(postgisSrid))
            iEPSG = 4490;
		else if (isSourceWGS1984Prj(postgisSrid))
			iEPSG = 4326;

        crs.createFromId(iEPSG);
    }
    else if ( isValidGCS(sourceCrs.authid()) )
    {
        return sourceCrs;
    }
    return crs;
}

long ipfProjectionTransformation::getPCSauthid_CGCS2000(const QgsPointXY point, const int bw)
{
    long id = 0;
    if (bw==3)
    {
        if (point.x() >= 75 && point.x() <= 135)
        {
            id = 4534;
            int cmJz = 75;
            int cmIn = getCentralmeridian3(point.x());
            while (cmJz != cmIn)
            {
                cmJz += 3;
                ++id;
            }
        }
        else if (point.x() >= 25 && point.x() <= 45)
        {
            id = 4513;
            int cmJz = 25;
            int cmIn = getBandwidth3(point.x());
            while (cmJz != cmIn)
            {
                ++cmJz;
                ++id;
            }
        }
    }
    else if (bw==6)
    {

    }
    return id;
}

long ipfProjectionTransformation::getPCSauthid_Wgs84Gcs(const QgsPointXY point)
{
	long id = 0;
	int dh = getWgs84Bandwidth(point.x());

	if (point.y() >= 0)
		id = 32600 + dh;
	else
		id = 32700 + dh;

	return id;
}

QString ipfProjectionTransformation::createProj4Cgcs2000Gcs(const double cm)
{
    QString pszCGCS;

    //3度分带，不加带号
    if (cm>74 && cm<136)
    {
        pszCGCS = QString("+proj=tmerc +lat_0=0 +lon_0=%1 +k=1 "
                          "+x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs").arg(cm);
    }
    //3度分带，加带号
    if (cm>24 && cm<46)
    {
        pszCGCS = QString("+proj=tmerc +lat_0=0 +lon_0=%1 +k=1 "
                          "+x_0=%2 +y_0=0 +ellps=GRS80 +units=m +no_defs").arg(cm*3).arg(cm*1000000+500000);
    }
    //6度分带，加带号
    if (cm>12 && cm<24)
    {
        pszCGCS = QString("+proj=tmerc +lat_0=0 +lon_0=%1 +k=1 "
                          "+x_0=%2 +y_0=0 +ellps=GRS80 +units=m +no_defs").arg(cm*6-3).arg(cm*1000000+500000);
    }

    return pszCGCS;
}

QString ipfProjectionTransformation::createProj4Xian1980Gcs(const double cm)
{
    QString pszCGCS;

    //3度分带，不加带号
    if (cm>74 && cm<136)
    {
        pszCGCS = QString("+proj=tmerc +lat_0=0 +lon_0=%1 +k=1 +x_0=500000 +y_0=0 "
                          "+a=6378140 +b=6356755.288157528 +units=m +no_defs").arg(cm);
    }
    //3度分带，加带号
    if (cm>24 && cm<46)
    {
        pszCGCS = QString("+proj=tmerc +lat_0=0 +lon_0=%1 +k=1 +x_0=%2 +y_0=0 "
                          "+a=6378140 +b=6356755.288157528 +units=m +no_defs").arg(cm*3).arg(cm*1000000+500000);
    }
    //6度分带，加带号
    if (cm>12 && cm<24)
    {
        pszCGCS = QString("+proj=tmerc +lat_0=0 +lon_0=%1 +k=1 +x_0=%2 +y_0=0 "
                          "+a=6378140 +b=6356755.288157528 +units=m +no_defs").arg(cm*6-3).arg(cm*1000000+500000);
    }

    return pszCGCS;
}

QString ipfProjectionTransformation::createProj4Beijing1954Gcs(const double cm)
{
    QString pszCGCS;

    //3度分带，不加带号
    if (cm>74 && cm<136)
    {
        pszCGCS = QString("+proj=tmerc +lat_0=0 +lon_0=%1 +k=1 "
                          "+x_0=500000 +y_0=0 +ellps=krass +towgs84=15.8,-154.4,-82.3,0,0,0,0 "
                          "+units=m +no_defs").arg(cm);
    }
    //3度分带，加带号
    if (cm>24 && cm<46)
    {
        pszCGCS = QString("+proj=tmerc +lat_0=0 +lon_0=%1 +k=1 "
                          "+x_0=%2 +y_0=0 +ellps=krass +towgs84=15.8,-154.4,-82.3,0,0,0,0 "
                          "+units=m +no_defs").arg(cm*3).arg(cm*1000000+500000);
    }
    //6度分带，加带号
    if (cm>12 && cm<24)
    {
        pszCGCS = QString("+proj=tmerc +lat_0=0 +lon_0=%1 +k=1 "
                          "+x_0=%2 +y_0=0 +ellps=krass +towgs84=15.8,-154.4,-82.3,0,0,0,0 "
                          "+units=m +no_defs").arg(cm*6-3).arg(cm*1000000+500000);
    }

    return pszCGCS;
}

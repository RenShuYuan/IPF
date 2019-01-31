#ifndef IPFFRACTALMANAGEMENT_H
#define IPFFRACTALMANAGEMENT_H

#include "head.h"
#include <QObject>
#include "qgspointxy.h"
#include "qgsmaptool.h"
#include "qgsmapcanvas.h"
#include "qgsgeometry.h"
#include "ipfProjectionTransformation.h"
#include "qgscoordinatereferencesystem.h"

struct jwc{
    double djc;
    double dwc;
    int ranks;
    QChar blcAbb;
};

class ipfFractalManagement : public QObject
{
    Q_OBJECT
public:
    ipfFractalManagement(QObject *parent = nullptr);
	ipfFractalManagement(const int blc, QObject *parent = nullptr);

	// 检查图号的有效性
	bool effectiveness(const QString &strName);

    // 根据一点生成图号
    QString pointToTh( const QgsPointXY p );

    // 生成两点范围内所有图号
    QStringList rectToTh(const QgsPointXY lastPoint, const QgsPointXY nextPoint);

    // 根据图号计算出四角的经纬度, 西南0,1;西北2,3;东北4,5;东南6,7
    QList< QgsPointXY > dNToLal(const QString &dNStr);

    // 根据图号计算出四角的投影坐标, 西南0,1;西北2,3;东北4,5;东南6,7
	QList< QgsPointXY > dNToXy(const QString &dNStr);

    // 根据设置对话框的索引号返回比例尺
    int getScale(const int index);

    // 设置分幅比例尺, 并同时设置对应的图幅经差与纬差
    void setBlc(const int mBlc);

	// 获得相邻图幅的图号
	// 1 2 3
	// 4 M 5
	// 6 7 8
	QString getAdjacentFrac(const QString &strName, const int possition);

	// 根据四至坐标计算外扩范围
	static QList<double> external(QList<QgsPointXY> &four, const double R, const int ext, const bool isCenter = true);

private:
    // 设置标准分幅经差与纬差,单位为秒
    void setJwc();

    // 设置比例尺对应字母
    void setBlcAbb();

	// 检查比例尺对应字母
	bool checkBlcAbb(const QChar c);

    // 根据比例尺字母返回行列最大值
    int getRanks(const QChar c);

    // 将行列号不够二、三位的填充"0"
    void fill0_two( QString& str );
	void fill0_three(QString& str);

    // 根据定义的投影坐标系设置对应
    // 的地理坐标系统，目前只支持
    // CGCS2000、XIAN 1980、BEIJING 1954、WGS 84
    bool setGCS();

    // 检查经纬度坐标是否在正常范围内
    bool checkLBisExtent(const QgsPointXY& point);

private:
    QSettings mSettings;
    ipfProjectionTransformation ipfPrj;

    int mBlc;           // 分幅比例尺
    struct jwc mJwc;    // 图幅经差与纬差

	QStringList errList;
};

#endif // IPFFRACTALMANAGEMENT_H

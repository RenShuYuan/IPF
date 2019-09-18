#ifndef IPFSPATIALGEOMETRYALGORITHM_H
#define IPFSPATIALGEOMETRYALGORITHM_H

#include "head.h"
#include "qgspointxy.h"
#include "QgsVectorLayer.h"
#include "QgsSpatialIndex.h"
#include "../../clipper/clipper.hpp"

using namespace ClipperLib;

class ipfSpatialGeometryAlgorithm
{
public:
	ipfSpatialGeometryAlgorithm();
	ipfSpatialGeometryAlgorithm(const int prec);
	~ipfSpatialGeometryAlgorithm();

	// 设置精度
	void setPrec(const int prec);

	// 获取pVertex到pOther1、pVertex到pOther2的夹角角度
	static double triangleVertexAngle(const QgsPoint & pVertex, const QgsPoint & pOther1, const QgsPoint & pOther2);

	// 获得QgsVectorLayer中所有feature集合
	static bool getFeatures(const QgsVectorLayer* layer_src, QVector< QgsFeature* > & featureVec);

	// 清除面积小于area的小面及环
	static bool clearPolygonANDring(const QgsVectorLayer* layer_src, QgsVectorLayer* layer_target, const double minimumArea, const double minimumRingsArea);

	// 融合 使用时需要注意先用setPrec设置精度
	QString dissovle(QgsVectorLayer* layer_src, QgsVectorLayer* layer_target, const QStringList& field);

private:
	// 以下函数主要是配合ClipperLib使用
	void multiPolygonToPath(const QgsGeometry &g, ClipperLib::Paths & paths);
	void pathToPolygon(const ClipperLib::Paths & paths, QgsGeometry & polygeon);
	void pathToOgrPolygon(const ClipperLib::Paths & paths, OGRPolygon & polygon);
	void pathToFeature(const ClipperLib::Paths & paths, QgsFeature & feature);

	ClipperLib::IntPoint doubleToInt(const QgsPointXY & p);
	QgsPointXY intToDouble(const ClipperLib::IntPoint & p);
	QgsRectangle intToDouble(const ClipperLib::IntRect & rect);

	void createSpatialIndex(const ClipperLib::Paths & paths, QgsSpatialIndex & index);

private:
	unsigned int mPrec;
	unsigned long mPrecX0;
};

#endif // IPFSPATIALGEOMETRYALGORITHM_H
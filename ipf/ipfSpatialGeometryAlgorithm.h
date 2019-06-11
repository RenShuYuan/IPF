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

	static double triangleVertexAngle(const QgsPoint & pVertex, const QgsPoint & pOther1, const QgsPoint & pOther2);

	// 融合
	QString dissovle(QgsVectorLayer* layer_src, QgsVectorLayer* layer_target, const QStringList& field);

	// 以下函数均需配合ClipperLib使用, 主要是与QGis之间的格式互换
	void geometryToPath(const QgsGeometry &g, ClipperLib::Paths & paths);
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
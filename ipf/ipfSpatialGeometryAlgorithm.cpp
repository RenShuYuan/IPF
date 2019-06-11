#include "ipfSpatialGeometryAlgorithm.h"
#include "gdal/ipfgdalprogresstools.h"
#include "../ui/ipfProgress.h"
#include <omp.h>

ipfSpatialGeometryAlgorithm::ipfSpatialGeometryAlgorithm()
{
}

ipfSpatialGeometryAlgorithm::ipfSpatialGeometryAlgorithm(const int prec)
{
	setPrec(prec);
}

ipfSpatialGeometryAlgorithm::~ipfSpatialGeometryAlgorithm()
{
}

void ipfSpatialGeometryAlgorithm::setPrec(const int prec)
{
	mPrec = prec;
	mPrecX0 = 1;
	for (int i = 0; i < prec; ++i)
		mPrecX0 *= 10;
}

double ipfSpatialGeometryAlgorithm::triangleVertexAngle(const QgsPoint & pVertex, const QgsPoint & pOther1, const QgsPoint & pOther2)
{
	double side[3];
	double angle = 180;

	// 计算边长
	side[0] = sqrt(pow(pVertex.x() - pOther1.x(), 2) + pow(pVertex.y() - pOther1.y(), 2) + pow(pVertex.z() - pOther1.z(), 2));
	side[1] = sqrt(pow(pVertex.x() - pOther2.x(), 2) + pow(pVertex.y() - pOther2.y(), 2) + pow(pVertex.z() - pOther2.z(), 2));
	side[2] = sqrt(pow(pOther2.x() - pOther1.x(), 2) + pow(pOther2.y() - pOther1.y(), 2) + pow(pOther2.z() - pOther1.z(), 2));

	// 计算顶点cos
	if (side[0] + side[1] <= side[2] || side[0] + side[2] <= side[1] || side[1] + side[2] <= side[0])
		return angle;

	// 反余弦计算顶点角度
	double cosVertex = (pow(side[0], 2) + pow(side[1], 2) - pow(side[2], 2)) / (2 * side[0] * side[1]);

	// 弧度转角度
	angle = acos(cosVertex);
	angle = angle * 180 / PI;

	return angle;
}

QString ipfSpatialGeometryAlgorithm::dissovle(QgsVectorLayer * layer_src, QgsVectorLayer * layer_target, const QStringList & field)
{
	QTime time; // time
	QString err;
	QgsSpatialIndex index;
	QgsFeatureList featureList;
	ClipperLib::Paths pathUnion;
	ClipperLib::Clipper clpr;
	ClipperLib::Paths solution;
	ClipperLib::Paths rings;
	QMultiMap<ClipperLib::cInt, ClipperLib::Path> polygons;

	//初始化进度条计数器
	ipfProgress proDialog;
	proDialog.setTitle(QStringLiteral("融合处理"));
	proDialog.setRangeTotal(0, 4);
	proDialog.show();

	// 选择图层中所有要素
	layer_src->selectAll();
	const QgsFeatureIds & ids_src = layer_src->selectedFeatureIds();
	
	// 要素分组
	QMap< QVector< QString >, QList< QgsFeatureId > > groups;
	for (auto i : ids_src)
	{
		QVector< QString > vec(field.size());
		QgsFeature & f = layer_src->getFeature(i);
		if (!f.isValid()) continue;

		for (int i = 0; i < field.size(); ++i)
		{
			vec[i] = f.attribute(field.at(i)).toString();
		}

		if (groups.contains(vec))
		{
			groups[vec].append(i);
		}
		else
		{
			groups[vec] = QList< QgsFeatureId >() << i;
		}
	}

	time.start(); //time
	// 获得图层中所有要素点集合
	int threadSize = omp_get_max_threads();
	std::vector< ClipperLib::Paths > pathTemp(threadSize);
	QList<QgsFeatureId> idList = ids_src.toList();
	proDialog.setRangeChild(0, idList.size());

#pragma omp parallel for
	for (int i = 0; i < idList.size(); ++i)
	{
		proDialog.pulsValue();
		QgsFeature & f = layer_src->getFeature(idList.at(i));
		if (f.isValid())
		{
			geometryToPath(f.geometry(), pathTemp[omp_get_thread_num()]);
		}
	}
	for (auto i : pathTemp)
		pathUnion.insert(pathUnion.end(), i.begin(), i.end());
	std::vector< ClipperLib::Paths >().swap(pathTemp); // 释放pathTemp
	qDebug() << QStringLiteral("获得所有要素点集合: %1s").arg(time.elapsed() / 1000.0); //time
	err += QStringLiteral("获得所有要素点集合: %1s \n").arg(time.elapsed() / 1000.0);

	time.start(); //time
	// 使用ClipperLib并联处理
	proDialog.setRangeChild(0, 0);
	clpr.AddPaths(pathUnion, ClipperLib::ptSubject, true);
	bool isbl = clpr.Execute(ClipperLib::ctUnion, solution, ClipperLib::pftNonZero);
	ClipperLib::Paths().swap(pathUnion); // 释放pathUnion
	if (!isbl || solution.empty())
		return QStringLiteral("dissovle: Union处理失败。");
	proDialog.userPulsValueTatal();
	qDebug() << QStringLiteral("ClipperLib::ctUnion处理: %1s").arg(time.elapsed() / 1000.0); //time
	err += QStringLiteral("ClipperLib::ctUnion处理: %1s \n").arg(time.elapsed() / 1000.0);

	time.start(); //time
	// 分离处理后的面与环
	proDialog.setRangeChild(0, solution.size());
	for(auto path : solution)
	{
		proDialog.pulsValue();
		double area = ClipperLib::Area(path) / mPrecX0;
		if (area >= 0)
			polygons.insert((ClipperLib::cInt)area, path);
		else
			rings << path;
	}
	ClipperLib::Paths().swap(solution); // 释放solution

	// 创建环的空间索引
	createSpatialIndex(rings, index);
	qDebug() << QStringLiteral("将面与环分开: %1s").arg(time.elapsed() / 1000.0); //time
	err += QStringLiteral("将面与环分开: %1s \n").arg(time.elapsed() / 1000.0);

	time.start(); //time
	// 遍历每个面，添加属于它的环
	int counter = 0;
	proDialog.setRangeChild(0, polygons.size());
	QList< ClipperLib::Path > & values = polygons.values();

#pragma omp parallel for
	for (int i = 0; i < values.size(); ++i)
	{
#pragma omp critical
		++counter;

		proDialog.pulsValue();

		ClipperLib::Paths result;
		const ClipperLib::Path & polygon = values.at(i);
		result << polygon;

		// 使用空间索引查找与面相交的环
		ClipperLib::Clipper clpr;
		clpr.AddPath(polygon, ClipperLib::ptSubject, true);
		ClipperLib::IntRect rect = clpr.GetBounds();
		QgsRectangle qrect = intToDouble(rect);
		QList< QgsFeatureId > idList = index.intersects(qrect);

		foreach(QgsFeatureId id, idList)
		{
			ClipperLib::Path ring = rings.at(id);
			if (counter == polygons.size())
			{
#pragma omp critical
				result << ring;
			}
			else
			{
				// 取环的第一个节点，判断该环是否在面内
				if (ClipperLib::PointInPolygon(ring[0], polygon) != 0)
				{
					QgsFeature MyFeature;
					MyFeature.setId(id);
					pathToFeature(ClipperLib::Paths() << ring, MyFeature);
#pragma omp critical
					{
						result << ring;
						index.deleteFeature(MyFeature);
					}
				}
			}
		}

		QgsFeature MyFeature;
		pathToFeature(result, MyFeature);
#pragma omp critical
		featureList.append(MyFeature);

		// 每处理一定数量的要素就写入图层中
		if ((counter % 50) == 0)
		{
#pragma omp critical
			{
				layer_target->dataProvider()->addFeatures(featureList);
				featureList.clear();
			}
		}
	}
	layer_target->dataProvider()->addFeatures(featureList);
	qDebug() << QStringLiteral("处理面与环: %1s").arg(time.elapsed() / 1000.0); //time
	err += QStringLiteral("处理面与环: %1s \n").arg(time.elapsed() / 1000.0);

	return err;
}

void ipfSpatialGeometryAlgorithm::geometryToPath(const QgsGeometry & g, ClipperLib::Paths & paths)
{
	QgsMultiPolygonXY multiPolygon = g.asMultiPolygon();

	foreach(QgsPolygonXY polygon, multiPolygon)
	{
		foreach(QgsPolylineXY polyline, polygon)
		{
			ClipperLib::Path p(polyline.size());
			for (int i = 0; i < polyline.size(); ++i)
			{
				p[i] = doubleToInt(polyline.at(i));
			}
			paths.push_back(p);
		}
	}
}

void ipfSpatialGeometryAlgorithm::pathToPolygon(const ClipperLib::Paths & paths, QgsGeometry & polygeon)
{
	QString wkt_polygon = "(";
	QStringList wkt_rings;

	for (size_t i = 0; i < paths.size(); ++i)
	{
		ClipperLib::Path part = paths[i];
		if (i == 0)
		{
			for (size_t i = 0; i < part.size(); ++i)
			{
				QgsPointXY point = intToDouble(part.at(i));
				wkt_polygon.append(QString("%1 %2,").arg(point.x(), 0, 'f', mPrec).arg(point.y(), 0, 'f', mPrec));
			}
			QgsPointXY point = intToDouble(part.at(0));
			wkt_polygon.append(QString("%1 %2,").arg(point.x(), 0, 'f', mPrec).arg(point.y(), 0, 'f', mPrec));

			wkt_polygon.remove(wkt_polygon.size() - 1, 1);
			wkt_polygon.append(")");
		}
		else
		{
			QString wkt_ring = "(";
			for (size_t i = 0; i < part.size(); ++i)
			{
				QgsPointXY point = intToDouble(part.at(i));
				wkt_ring.append(QString("%1 %2,").arg(point.x(), 0, 'f', mPrec).arg(point.y(), 0, 'f', mPrec));
			}
			QgsPointXY point = intToDouble(part.at(0));
			wkt_ring.append(QString("%1 %2,").arg(point.x(), 0, 'f', mPrec).arg(point.y(), 0, 'f', mPrec));

			wkt_ring.remove(wkt_ring.size() - 1, 1);
			wkt_ring.append(")");
			wkt_rings << wkt_ring;
		}
	}

	QString wkt = "MULTIPOLYGON((" + wkt_polygon;
	foreach(QString wkt_ring, wkt_rings)
		wkt.append("," + wkt_ring);
	wkt.append("))");
	polygeon = QgsGeometry::fromWkt(wkt);
}

void ipfSpatialGeometryAlgorithm::pathToOgrPolygon(const ClipperLib::Paths & paths, OGRPolygon & polygon)
{
	for (size_t i = 0; i < paths.size(); ++i)
	{
		OGRLinearRing ring;
		ClipperLib::Path part = paths[i];
		for (size_t i = 0; i < part.size(); ++i)
		{
			QgsPointXY point = intToDouble(part.at(i));
			ring.addPoint(point.x(), point.y());
		}
		polygon.addRing(&ring);
	}
}

void ipfSpatialGeometryAlgorithm::pathToFeature(const ClipperLib::Paths & paths, QgsFeature & feature)
{
	QgsGeometry mGeometry;
	pathToPolygon(paths, mGeometry);
	feature.setGeometry(mGeometry);
}

ClipperLib::IntPoint ipfSpatialGeometryAlgorithm::doubleToInt(const QgsPointXY & p)
{
	ClipperLib::IntPoint ip;
	ip.X = (ClipperLib::cInt)(p.x()*mPrecX0);
	ip.Y = (ClipperLib::cInt)(p.y()*mPrecX0);
	return ip;
}

QgsPointXY ipfSpatialGeometryAlgorithm::intToDouble(const ClipperLib::IntPoint & p)
{
	QgsPointXY qp;
	qp.setX(((double)p.X) / mPrecX0);
	qp.setY(((double)p.Y) / mPrecX0);
	return qp;
}

QgsRectangle ipfSpatialGeometryAlgorithm::intToDouble(const ClipperLib::IntRect & rect)
{
	QgsRectangle qRect(
		((double)rect.left) / mPrecX0,
		((double)rect.bottom) / mPrecX0,
		((double)rect.right) / mPrecX0,
		((double)rect.top) / mPrecX0);
	return qRect;
}

void ipfSpatialGeometryAlgorithm::createSpatialIndex(const ClipperLib::Paths & paths, QgsSpatialIndex & index)
{
	ClipperLib::Clipper clpr;
	for (size_t i = 0; i < paths.size(); ++i)
	{
		clpr.Clear();
		clpr.AddPath(paths.at(i), ClipperLib::ptSubject, true);
		index.insertFeature(i, intToDouble(clpr.GetBounds()));
	}
}
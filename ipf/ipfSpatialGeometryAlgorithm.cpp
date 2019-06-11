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

	// ����߳�
	side[0] = sqrt(pow(pVertex.x() - pOther1.x(), 2) + pow(pVertex.y() - pOther1.y(), 2) + pow(pVertex.z() - pOther1.z(), 2));
	side[1] = sqrt(pow(pVertex.x() - pOther2.x(), 2) + pow(pVertex.y() - pOther2.y(), 2) + pow(pVertex.z() - pOther2.z(), 2));
	side[2] = sqrt(pow(pOther2.x() - pOther1.x(), 2) + pow(pOther2.y() - pOther1.y(), 2) + pow(pOther2.z() - pOther1.z(), 2));

	// ���㶥��cos
	if (side[0] + side[1] <= side[2] || side[0] + side[2] <= side[1] || side[1] + side[2] <= side[0])
		return angle;

	// �����Ҽ��㶥��Ƕ�
	double cosVertex = (pow(side[0], 2) + pow(side[1], 2) - pow(side[2], 2)) / (2 * side[0] * side[1]);

	// ����ת�Ƕ�
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

	//��ʼ��������������
	ipfProgress proDialog;
	proDialog.setTitle(QStringLiteral("�ںϴ���"));
	proDialog.setRangeTotal(0, 4);
	proDialog.show();

	// ѡ��ͼ��������Ҫ��
	layer_src->selectAll();
	const QgsFeatureIds & ids_src = layer_src->selectedFeatureIds();
	
	// Ҫ�ط���
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
	// ���ͼ��������Ҫ�ص㼯��
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
	std::vector< ClipperLib::Paths >().swap(pathTemp); // �ͷ�pathTemp
	qDebug() << QStringLiteral("�������Ҫ�ص㼯��: %1s").arg(time.elapsed() / 1000.0); //time
	err += QStringLiteral("�������Ҫ�ص㼯��: %1s \n").arg(time.elapsed() / 1000.0);

	time.start(); //time
	// ʹ��ClipperLib��������
	proDialog.setRangeChild(0, 0);
	clpr.AddPaths(pathUnion, ClipperLib::ptSubject, true);
	bool isbl = clpr.Execute(ClipperLib::ctUnion, solution, ClipperLib::pftNonZero);
	ClipperLib::Paths().swap(pathUnion); // �ͷ�pathUnion
	if (!isbl || solution.empty())
		return QStringLiteral("dissovle: Union����ʧ�ܡ�");
	proDialog.userPulsValueTatal();
	qDebug() << QStringLiteral("ClipperLib::ctUnion����: %1s").arg(time.elapsed() / 1000.0); //time
	err += QStringLiteral("ClipperLib::ctUnion����: %1s \n").arg(time.elapsed() / 1000.0);

	time.start(); //time
	// ���봦�������뻷
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
	ClipperLib::Paths().swap(solution); // �ͷ�solution

	// �������Ŀռ�����
	createSpatialIndex(rings, index);
	qDebug() << QStringLiteral("�����뻷�ֿ�: %1s").arg(time.elapsed() / 1000.0); //time
	err += QStringLiteral("�����뻷�ֿ�: %1s \n").arg(time.elapsed() / 1000.0);

	time.start(); //time
	// ����ÿ���棬����������Ļ�
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

		// ʹ�ÿռ��������������ཻ�Ļ�
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
				// ȡ���ĵ�һ���ڵ㣬�жϸû��Ƿ�������
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

		// ÿ����һ��������Ҫ�ؾ�д��ͼ����
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
	qDebug() << QStringLiteral("�������뻷: %1s").arg(time.elapsed() / 1000.0); //time
	err += QStringLiteral("�������뻷: %1s \n").arg(time.elapsed() / 1000.0);

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
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

bool ipfSpatialGeometryAlgorithm::getFeatures(const QgsVectorLayer * layer_src, QVector<QgsFeature*>& featureVec)
{
	if (layer_src == nullptr || !layer_src->isValid())
		return false;

	int indexVec = -1;
	try
	{
		QgsFeature f;
		featureVec.resize(layer_src->featureCount());
		QgsFeatureIterator & featureIt = layer_src->getFeatures();
		while (featureIt.nextFeature(f))
			if (f.isValid())
				featureVec[++indexVec] = new QgsFeature(f);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool ipfSpatialGeometryAlgorithm::clearPolygonANDring(const QgsVectorLayer * layer_src, QgsVectorLayer * layer_target,const double minimumArea, const double minimumRingsArea)
{
	//��ʼ��������������
	ipfProgress proDialog;
	proDialog.setTitle(QStringLiteral("ʸ�����ݴ���"));
	proDialog.setRangeTotal(0, 1);
	proDialog.show();

	int counter = 0;
	QgsFeatureList featureList;
	QVector< QgsFeature* > featureVec;
	if (!getFeatures(layer_src, featureVec)) return false;
	proDialog.setRangeChild(0, featureVec.size());

#pragma omp parallel for
	for (int i = 0; i < featureVec.size(); ++i)
	{
		proDialog.pulsValue();
		QgsFeature* f = featureVec[i];
		if ((f->geometry()).area() >= minimumArea) // ɾ��������ն�
		{
			QgsFeature outFeature;
			outFeature.setGeometry((f->geometry()).removeInteriorRings(minimumRingsArea));
			outFeature.setAttributes(f->attributes());
#pragma omp critical
			featureList.append(outFeature);
		}

		// ÿ����һ��������Ҫ�ؾ�д��ͼ����
#pragma omp critical
		{
			if ((++counter % 50) == 0)
			{
				layer_target->dataProvider()->addFeatures(featureList);
				featureList.clear();
			}
		}
	}
	layer_target->dataProvider()->addFeatures(featureList);
	for (auto i : featureVec) RELEASE(i); // �ͷ�featureָ��
	return true;
}

QString ipfSpatialGeometryAlgorithm::dissovle(QgsVectorLayer * layer_src, QgsVectorLayer * layer_target, const QStringList & fieldList)
{
	QString err;
	QMap< QVector< QString >, QList< QgsFeature* > > groups;

	//��ʼ��������������
	ipfProgress proDialog;
	proDialog.setTitle(QStringLiteral("�ںϴ���"));
	proDialog.setRangeTotal(0, 0);
	proDialog.show();

	// ���ֶ�����ת��Ϊ������
	QgsFields fields = layer_src->fields();
	QVector< int > fieldIndex;
	for (auto i : fieldList)
	{
		int index = fields.indexFromName(i);
		if (index != -1)
			fieldIndex.push_back(index);
	}

	// ���ͼ��������Ҫ��
	QVector< QgsFeature* > featureVec;
	if (!getFeatures(layer_src, featureVec))
		return QStringLiteral("��ȡʸ��ͼ���е�Ҫ��ʧ�ܡ�");

	// Ҫ�ط���
	proDialog.setRangeChild(0, featureVec.size());
#pragma omp parallel for
	for (int i = 0; i < featureVec.size(); ++i)
	{
		proDialog.pulsValue();

		QgsFeature* f = featureVec[i];
		if ((f == nullptr) || (!f->isValid())) continue;

		QVector< QString > vec(fieldIndex.size());
		for (int i = 0; i < fieldIndex.size(); ++i)
			vec[i] = f->attribute(fieldIndex[i]).toString();

#pragma omp critical
		{
			if (groups.contains(vec))
				groups[vec].append(f);
			else
				groups[vec] = QList< QgsFeature* >() << f;
		}
	}

	int currIndex = 0;
	proDialog.setRangeTotal(0, 4);
	for (auto i = groups.begin(); i != groups.end(); ++i)
	{
		QgsSpatialIndex index;
		QgsFeatureList featureList;
		ClipperLib::Paths pathUnion;
		ClipperLib::Clipper clpr;
		ClipperLib::Paths solution;
		ClipperLib::Paths rings;
		QMultiMap<ClipperLib::cInt, ClipperLib::Path> polygons;

		proDialog.setTitle(QStringLiteral("�ںϴ��� %1/%2").arg(++currIndex).arg(groups.size()));

		auto vec = i.key();
		auto list = i.value();

		// ����feature����
		QVector< QVariant > attrsVec(layer_target->fields().size());
		for (int i = 0; i < fieldIndex.size(); ++i)
			attrsVec[fieldIndex[i]] = vec[i];
		QgsAttributes attrs(attrsVec);

		// ���ͼ��������Ҫ�ص㼯��
		int threadSize = omp_get_max_threads();
		std::vector< ClipperLib::Paths > pathTemp(threadSize);
		proDialog.setRangeChild(0, list.size());

#pragma omp parallel for
		for (int i = 0; i < list.size(); ++i)
		{
			proDialog.pulsValue();
			QgsFeature* f = list.at(i);
			if (f!=nullptr && f->isValid())
			{
				int num = omp_get_thread_num();
				multiPolygonToPath(f->geometry(), pathTemp[num]);
			}
		}
		for (auto i : pathTemp)
			pathUnion.insert(pathUnion.end(), i.begin(), i.end());
		std::vector< ClipperLib::Paths >().swap(pathTemp); // �ͷ�pathTemp
		for (auto i : featureVec) RELEASE(i); // �ͷ�featureָ��

		// ʹ��ClipperLib��������
		proDialog.setRangeChild(0, 0);
		clpr.AddPaths(pathUnion, ClipperLib::ptSubject, true);
		bool isbl = clpr.Execute(ClipperLib::ctUnion, solution, ClipperLib::pftNonZero);
		ClipperLib::Paths().swap(pathUnion); // �ͷ�pathUnion
		if (!isbl || solution.empty())
			return QStringLiteral("dissovle: Union����ʧ�ܡ�");
		proDialog.userPulsValueTatal();

		// ���봦�������뻷
		proDialog.setRangeChild(0, solution.size());
		for (auto path : solution)
		{
			proDialog.pulsValue();
			double area = ClipperLib::Area(path);
			if (area >= 0)
				polygons.insert((ClipperLib::cInt)area, path);
			else
				rings << path;
		}
		ClipperLib::Paths().swap(solution); // �ͷ�solution

		// �������Ŀռ�����
		createSpatialIndex(rings, index);

		// ����ÿ���棬����������Ļ�
		int counter = 0;
		proDialog.setRangeChild(0, polygons.size());
		auto & values = polygons.values();

#pragma omp parallel for
		for (int i = 0; i < values.size(); ++i)
		{
#pragma omp critical
			++counter;

			proDialog.pulsValue();

			ClipperLib::Paths result;
			auto & polygon = values.at(i);
			result << polygon;

			// ʹ�ÿռ��������������ཻ�Ļ�
			ClipperLib::Clipper clpr;
			clpr.AddPath(polygon, ClipperLib::ptSubject, true);
			auto & idList = index.intersects(intToDouble(clpr.GetBounds()));

			foreach(QgsFeatureId id, idList)
			{
				auto ring = rings.at(id);
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

			// ����feature
			QgsFeature MyFeature;
			MyFeature.setAttributes(attrs);
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
	}

	return err;
}

void ipfSpatialGeometryAlgorithm::multiPolygonToPath(const QgsGeometry & g, ClipperLib::Paths & paths)
{
	QgsMultiPolygonXY multiPolygon = g.asMultiPolygon();
	for (QgsPolygonXY polygon : multiPolygon)
	{
		size_t size = paths.size();
		paths.resize(size + polygon.size());
		for (int j = 0; j < polygon.size(); ++j)
		{
			QgsPolylineXY polyline = polygon.at(j);
			ClipperLib::Path p(polyline.size());
			for (int i = 0; i < polyline.size(); ++i)
				p[i] = doubleToInt(polyline.at(i));
			paths[size + j] = p;
		}
	}
}

void ipfSpatialGeometryAlgorithm::pathToPolygon(const ClipperLib::Paths & paths, QgsGeometry & polygeon)
{
	QString wkt_polygon = "(";
	QStringList wkt_rings;

	for (auto i = 0; i < paths.size(); ++i)
	{
		auto & part = paths[i];
		if (i == 0)
		{
			for (auto & p : part)
			{
				QgsPointXY && point = intToDouble(p);
				wkt_polygon.append(std::move(QString("%1 %2,").arg(point.x(), 0, 'f', mPrec).arg(point.y(), 0, 'f', mPrec)));
			}
			QgsPointXY && point = intToDouble(part.at(0));
			wkt_polygon.append(std::move(QString("%1 %2,").arg(point.x(), 0, 'f', mPrec).arg(point.y(), 0, 'f', mPrec)));

			wkt_polygon.remove(wkt_polygon.size() - 1, 1);
			wkt_polygon.append(")");
		}
		else
		{
			QString wkt_ring = "(";
			for (auto & p : part)
			{
				QgsPointXY && point = intToDouble(p);
				wkt_ring.append(std::move(QString("%1 %2,").arg(point.x(), 0, 'f', mPrec).arg(point.y(), 0, 'f', mPrec)));
			}
			QgsPointXY && point = intToDouble(part.at(0));
			wkt_ring.append(std::move(QString("%1 %2,").arg(point.x(), 0, 'f', mPrec).arg(point.y(), 0, 'f', mPrec)));

			wkt_ring.remove(wkt_ring.size() - 1, 1);
			wkt_ring.append(")");
			wkt_rings << std::move(wkt_ring);
		}
	}

	QString wkt = "MULTIPOLYGON((" + std::move(wkt_polygon);
	for (QString wkt_ring : wkt_rings)
		wkt.append("," + std::move(wkt_ring));
	wkt.append("))");
	polygeon = std::move(QgsGeometry::fromWkt(wkt));
}

void ipfSpatialGeometryAlgorithm::pathToOgrPolygon(const ClipperLib::Paths & paths, OGRPolygon & polygon)
{
	for (size_t i = 0; i < paths.size(); ++i)
	{
		OGRLinearRing ring;
		auto part = paths[i];
		for (size_t i = 0; i < part.size(); ++i)
		{
			auto point = intToDouble(part.at(i));
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
	ip.X = static_cast<ClipperLib::cInt>(p.x()*mPrecX0);
	ip.Y = static_cast<ClipperLib::cInt>(p.y()*mPrecX0);
	return ip;
}

QgsPointXY ipfSpatialGeometryAlgorithm::intToDouble(const ClipperLib::IntPoint & p)
{
	QgsPointXY qp;
	qp.setX((static_cast<double>(p.X) / mPrecX0));
	qp.setY((static_cast<double>(p.Y) / mPrecX0));
	return qp;
}

QgsRectangle ipfSpatialGeometryAlgorithm::intToDouble(const ClipperLib::IntRect & rect)
{
	QgsRectangle qRect(
		(static_cast<double>(rect.left)) / mPrecX0,
		(static_cast<double>(rect.bottom)) / mPrecX0,
		(static_cast<double>(rect.right)) / mPrecX0,
		(static_cast<double>(rect.top)) / mPrecX0);
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
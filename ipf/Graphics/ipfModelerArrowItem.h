#ifndef IPFMODELERARROWITEM_H
#define IPFMODELERARROWITEM_H

#include <QGraphicsPathItem>
#include <QPen>
#include <QPolygonF>
#include <QPointF>
#include <QColor>

class ipfModelerGraphicItem;

class ipfModelerArrowItem : public QGraphicsPathItem
{
public:
	ipfModelerArrowItem(ipfModelerGraphicItem *startItem, int startIndex, ipfModelerGraphicItem *endItem, int endIndex);
	~ipfModelerArrowItem();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void setStartItem(ipfModelerGraphicItem *startItem, int startIndex);
	void setEndItem(ipfModelerGraphicItem *endItem, int endIndex);
	void setPenStyle(Qt::PenStyle style);
	void updatePath();

	void removeSelf();

private:
	ipfModelerGraphicItem *startItem;
	ipfModelerGraphicItem *endItem;
	QList< QPointF > endPoints;
	QPolygonF arrowHead;
	int startIndex;
	int endIndex;
	QColor myColor;
};

#endif // IPFMODELERARROWITEM_H
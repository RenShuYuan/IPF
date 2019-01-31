#include "ipfModelerArrowItem.h"
#include "ipfModelerGraphicItem.h"
#include "ipf/Process/ipfModelerProcessChild.h"
#include "ipf/Process/ipfModelerProcessIn.h"
#include "ipf/Process/ipfModelerProcessOut.h"
#include "head.h"

#include <QPainter>
#include <QGraphicsScene>

ipfModelerArrowItem::ipfModelerArrowItem(ipfModelerGraphicItem *startItem, int startIndex, ipfModelerGraphicItem *endItem, int endIndex)
	: endIndex(endIndex)
	, startIndex(startIndex)
	, startItem(startItem)
	, endItem(endItem)
{
	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
	this->myColor = Qt::gray;
	this->setPen(QPen(this->myColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	this->setZValue(0);
}

ipfModelerArrowItem::~ipfModelerArrowItem()
{
}

void ipfModelerArrowItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	QPen myPen = this->pen();
	myPen.setColor(this->myColor);
	painter->setPen(myPen);
	painter->setBrush(this->myColor);
	painter->setRenderHint(QPainter::Antialiasing);

	foreach(QPointF point, this->endPoints)
	{
		painter->drawEllipse(point.x(), point.y(), 6, 6);
	}

	painter->setBrush(Qt::NoBrush);
	painter->drawPath(this->path());
}

void ipfModelerArrowItem::setStartItem(ipfModelerGraphicItem * startItem, int startIndex)
{
	this->startItem = startItem;
	this->startIndex = startIndex;
	updatePath();
}

void ipfModelerArrowItem::setEndItem(ipfModelerGraphicItem * endItem, int endIndex)
{
	this->endItem = endItem;
	this->endIndex = endIndex;
	updatePath();
}

void ipfModelerArrowItem::setPenStyle(Qt::PenStyle style)
{
	QPen pen = this->pen();
	pen.setStyle(style);
	this->setPen(pen);
	this->update();
}

void ipfModelerArrowItem::updatePath()
{
	QPainterPath path;
	QList<QPointF> controlPoints;
	QPointF startPt;
	QPointF endPt;
	endPoints.clear();

	endPt = endItem->getLinkPointForParameter(endIndex);
	if (startItem->label() == MODELER_IN)
	{
		controlPoints.append(startItem->pos());
		controlPoints.append(startItem->pos() + QPointF(ipfModelerGraphicItem::BOX_WIDTH / 3, 0));
	}
	else
	{
		QPointF pt = startItem->getLinkPointForOutput(-1);
		controlPoints.append(startItem->pos() + pt);
		controlPoints.append(startItem->pos() + pt + QPointF(ipfModelerGraphicItem::BOX_WIDTH / 3, 0));
		endPoints.append(startItem->pos() + pt + QPointF(-3, -3));
	}

	if (endItem->label() == MODELER_OUT)
	{
		controlPoints.append(endItem->pos() - QPointF(0, ipfModelerGraphicItem::BOX_WIDTH / 3));
		QPointF pt = endItem->getLinkPointForParameter(-1);
		pt = QPointF(0, pt.y());
		controlPoints.append(endItem->pos() + pt);
		endPoints.append(endItem->pos() + pt + QPointF(-3, -3));
	}
	else
	{
		controlPoints.append(endItem->pos() + endPt - QPointF(ipfModelerGraphicItem::BOX_WIDTH / 3, 0));
		QPointF pt = endItem->getLinkPointForParameter(-1);
		controlPoints.append(endItem->pos() + pt);
		endPoints.append(endItem->pos() + pt + QPointF(-3, -3));
	}

	path.moveTo(controlPoints[0]);
	path.cubicTo(controlPoints.at(1), controlPoints.at(2), controlPoints.at(3));
	setPath(path);
}

void ipfModelerArrowItem::removeSelf()
{
	scene()->removeItem(this);
}

#include "ipfGraphicsView.h"
#include "ipfGraphicsScene.h"
#include "Graphics/ipfModelerGraphicItem.h"

#include <QTreeView>
#include <QMouseEvent>
#include <QMimeData>
#include <QStandardItem>

ipfGraphicsView::ipfGraphicsView(QWidget *parent)
	: QGraphicsView(parent)
	, collidingItem(nullptr)
{
}

ipfGraphicsView::~ipfGraphicsView()
{
}

void ipfGraphicsView::wheelEvent(QWheelEvent * event)
{
	double numDegrees = event->delta() / 8.0;
	double numSteps = numDegrees / 15.0;
	double factor = std::pow(1.125, numSteps);
	scale(factor, factor);

	event->accept();
}

void ipfGraphicsView::dragEnterEvent(QDragEnterEvent * event)
{
	event->acceptProposedAction();
}

void ipfGraphicsView::dropEvent(QDropEvent * event)
{
	ipfModelerProcessBase *base = nullptr;
	QTreeView* tree = qobject_cast<QTreeView *>(event->source());
	if (tree)
	{
		QModelIndex index = tree->currentIndex();
		QVariant item = tree->model()->data(index);
		QString itemName = item.toString();

		ipfGraphicsScene *s = dynamic_cast<ipfGraphicsScene*>(scene());
		if (s)
		{
			if (collidingItem)
			{
				s->insrtModel(collidingItem, itemName);
				collidingItem->setColliding(false);
				collidingItem = nullptr;
			}
			else
			{
				QPointF p = mapToScene(event->pos());
				s->addModel(itemName, p);
			}
		}
	}

	event->accept();
}

void ipfGraphicsView::dragMoveEvent(QDragMoveEvent * event)
{
	if (QGraphicsItem *item = itemAt(event->pos()))
	{
		ipfModelerGraphicItem *i = dynamic_cast<ipfModelerGraphicItem*>(item);
		if (i)
		{
			if (collidingItem)
			{
				if (i != collidingItem)
				{
					collidingItem->setColliding(false);
					collidingItem = i;
				}
			}
			else
			{
				i->setColliding(true);
				collidingItem = i;
			}
		}
	}
	else
	{
		if (collidingItem)
		{
			collidingItem->setColliding(false);
			collidingItem = nullptr;
		}
	}

	event->accept();
}

void ipfGraphicsView::mousePressEvent(QMouseEvent * event)
{
	setDragMode(QGraphicsView::ScrollHandDrag);
	QGraphicsView::mousePressEvent(event);
}

void ipfGraphicsView::mouseReleaseEvent(QMouseEvent * event)
{
	setDragMode(QGraphicsView::NoDrag);
	QGraphicsView::mouseReleaseEvent(event);
}

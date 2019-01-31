#ifndef IPFGRAPHICSVIEW_H
#define IPFGRAPHICSVIEW_H

#include <QGraphicsView>
#include "head.h"

class ipfModelerGraphicItem;

class ipfGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	ipfGraphicsView(QWidget *parent = 0);
	~ipfGraphicsView();

protected:
	void wheelEvent(QWheelEvent *event);

	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);

	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

private:
	ipfModelerGraphicItem * collidingItem;
};

#endif // IPFGRAPHICSVIEW_H

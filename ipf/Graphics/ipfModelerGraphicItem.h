#ifndef IPFMODELERGRAPHICITEM_H
#define IPFMODELERGRAPHICITEM_H

#include <QGraphicsItem>
#include <QColor>
#include <QRectF>
#include <QFont>
#include <QPicture>

class ipfModelerArrowItem;
class ipfModelerProcessBase;

class FlatButtonGraphicItem : public QGraphicsItem
{
public:
	enum Type
	{
		tDelete,
		tEdit,
		tErr,
		tOk,
		tOther,
	};

	static const int sSize = 16;
	static const int bSize = 28;

	FlatButtonGraphicItem(QPicture picture, QPointF position, FlatButtonGraphicItem::Type type);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void setPicture(const QPicture& picture);
	void setSize(const int size);
	int getSize() { return size; };

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
	QPicture picture;
	QPointF position;
	bool isIn;

	int size;

	FlatButtonGraphicItem::Type type;
};

class FoldButtonGraphicItem : public FlatButtonGraphicItem
{
public:
	static const int WIDTH = 11;
	static const int HEIGHT = 11;

	FoldButtonGraphicItem(QPointF position, bool folded);
	bool isFolded();

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
	bool folded;
	QMap< bool, QPicture > picMap;
	QPicture picture;
	QPointF pt;
};

class ipfModelerGraphicItem : public QGraphicsItem
{
public:
	static const int BOX_HEIGHT = 30;
	static const int BOX_WIDTH = 200;

	ipfModelerGraphicItem(ipfModelerProcessBase *process, QMenu *menu);
	~ipfModelerGraphicItem();

	void colorInOut();
	void colorChild();

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	void isOkButton(const bool isbl);
	void isErrButton(const bool isbl);

	QString getAdjustedText(QString text);
	QPointF getLinkPointForParameter(int paramIndex);
	QPointF getLinkPointForOutput(int outputIndex);

	void addArrow(ipfModelerArrowItem *arrow);
	QList<ipfModelerArrowItem*> getArrows() { return arrows; };
	void deleteArrows(QList<ipfModelerArrowItem*> arrows);
	void deleteArrow(ipfModelerArrowItem* arrow);
	
	// 将自身图形删除，同时在ipfFlowManage中删除
	void removeSelf();
	void removeGraphicItem();

	void editParameter();
	void diskplayInfo();
	void setColliding(bool bl);

	ipfModelerProcessBase* modelerProcess() { return process; };
	QString label() { return itemLabel; };

protected:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
	QMenu *menu;

	QFont font;
	QString itemLabel;

	QColor color;
	QColor stroke;
	QColor selected;
	QColor colliding;

	ipfModelerProcessBase *process;
	FoldButtonGraphicItem *inButton;
	FoldButtonGraphicItem *outButton;
	QList<ipfModelerArrowItem*> arrows;

	FlatButtonGraphicItem *okButton;
	FlatButtonGraphicItem *errButton;
	bool isErrB;

	bool isColliding;

	QPicture picture;
};

#endif // IPFMODELERGRAPHICITEM_H
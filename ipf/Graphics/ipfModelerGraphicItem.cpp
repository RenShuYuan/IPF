#include "ipfModelerGraphicItem.h"
#include "ipfModelerArrowItem.h"
#include "ImageProcessFactory.h"
#include "../ipfapplication.h"
#include "../Process/ipfFlowManage.h"
#include "ipf/Process/ipfModelerProcessIn.h"
#include "ipf/Process/ipfModelerProcessOut.h"
#include "../ui/ipfTextInfoDialog.h"
#include "head.h"

#include <QRect>
#include <QPen>
#include <QFontMetricsF>
#include <QPainter>
#include <QSvgRenderer>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>

FlatButtonGraphicItem::FlatButtonGraphicItem(QPicture picture, QPointF position, FlatButtonGraphicItem::Type type)
	: picture(picture)
	, position(position)
	, type(type)
	, isIn(false)
{
	if (type == tDelete || type == tEdit)
		setSize(sSize);
	else if (type == tErr || type == tOk)
		setSize(bSize);

	setAcceptHoverEvents(true);
	setFlag(QGraphicsItem::ItemIsMovable, false);
}

QRectF FlatButtonGraphicItem::boundingRect() const
{
	QRectF rect = QRectF(position.x() - floor(size / 2), position.y() - floor(size / 2), size, size);
	return rect;
}

void FlatButtonGraphicItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QPointF pt = QPointF(-floor(size / 2), -floor(size / 2)) + position;
	QRectF rect = QRectF(pt.x(), pt.y(), size, size);
	if (isIn)
	{
		painter->setPen(QPen(Qt::transparent, 1));
		painter->setBrush(QBrush(QColor(55, 55, 55, 33), Qt::SolidPattern));
	}
	else
	{
		painter->setPen(QPen(Qt::transparent, 1));
		painter->setBrush(QBrush(Qt::transparent, Qt::SolidPattern));
	}
	painter->drawRect(rect);
	painter->drawPicture(pt.x(), pt.y(), picture);
}

void FlatButtonGraphicItem::setPicture(const QPicture &picture)
{
	this->picture = picture;
}

void FlatButtonGraphicItem::setSize(const int size)
{
	this->size = size;
}

void FlatButtonGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	ipfModelerGraphicItem *item = dynamic_cast<ipfModelerGraphicItem*>(parentItem());
	if (item)
	{
		if (type == tDelete)
			item->removeSelf();
		else if (type == tEdit)
			item->editParameter();
		else if (type == tErr)
			item->diskplayInfo();
	}
}

void FlatButtonGraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	isIn = true;
	update();
}

void FlatButtonGraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	isIn = false;
	update();
}

FoldButtonGraphicItem::FoldButtonGraphicItem(QPointF position, bool folded)
	: FlatButtonGraphicItem(QPicture(), position, FlatButtonGraphicItem::tOther)
{
	QPicture plus;
	QPicture minus;
	
	QSvgRenderer svgP(ipfApplication::getThemeIconPath("plus.svg"));
	QPainter painterP(&plus);
	svgP.render(&painterP);
	QSvgRenderer svgM(ipfApplication::getThemeIconPath("minus.svg"));
	QPainter painterM(&minus);
	svgM.render(&painterM);

	picMap[true] = plus;
	picMap[false] = minus;

	this->folded = folded;
	picture = picMap[this->folded];

	setPicture(picture);
	setSize(WIDTH);
}

bool FoldButtonGraphicItem::isFolded()
{
	return folded;
}

void FoldButtonGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	folded = !folded;
	picture = picMap[folded];
	setPicture(picture);
	update();
}

ipfModelerGraphicItem::ipfModelerGraphicItem(ipfModelerProcessBase *process, QMenu *menu)
	: process(process)
	, menu(menu)
	, inButton(nullptr)
	, outButton(nullptr)
	, isColliding(false)
	, isErrB(false)
{
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setZValue(1000);

	itemLabel = process->name();
	colliding = QColor(255, 160, 47);
	font = QFont(QStringLiteral("微软雅黑"), 8);
	font.setPixelSize(12);

	if (process->name() == MODELER_IN)
	{
		// 设置图标
		QSvgRenderer svg(ipfApplication::getThemeIconPath("input.svg"));
		QPainter painter(&picture);
		svg.render(&painter);
	}
	else if (process->typeName() == QString("OUT"))
	{
		// 设置图标
		QSvgRenderer svg(ipfApplication::getThemeIconPath("output.svg"));
		QPainter painter(&picture);
		svg.render(&painter);
	}
	else
	{
		// 设置图标
		QSvgRenderer svg(ipfApplication::getThemeIconPath("process.svg"));
		QPainter painter(&picture); picture.setBoundingRect(QRect(0,0,20,20));
		svg.render(&painter);

		//+ - 按钮
		QPointF pt = getLinkPointForParameter(-1);
		pt = QPointF(0, pt.y());
		inButton = new FoldButtonGraphicItem(pt, true);
		inButton->setParentItem(this);

		pt = getLinkPointForOutput(-1);
		pt = QPointF(0, pt.y());
		outButton = new FoldButtonGraphicItem(pt, true);
		outButton->setParentItem(this);
	}

	// 编辑按钮
	QSvgRenderer svg(ipfApplication::getThemeIconPath("edit.svg"));
	QPicture picture;
	QPainter painter(&picture);
	svg.render(&painter);
	QPointF pt(BOX_WIDTH / 2 - FlatButtonGraphicItem::sSize / 2
			, BOX_HEIGHT / 2 - FlatButtonGraphicItem::sSize / 2);
	FlatButtonGraphicItem *editButton = new FlatButtonGraphicItem(picture, pt, FlatButtonGraphicItem::tEdit);
	editButton->setParentItem(this);

	// 删除按钮
	QSvgRenderer svg1(ipfApplication::getThemeIconPath("delete.svg"));
	QPicture picture1;
	QPainter painter1(&picture1);
	svg1.render(&painter1);
	pt = QPointF(BOX_WIDTH / 2 - FlatButtonGraphicItem::sSize / 2,
				FlatButtonGraphicItem::sSize / 2 - BOX_HEIGHT / 2);
	FlatButtonGraphicItem *deleteButton = new FlatButtonGraphicItem(picture1, pt, FlatButtonGraphicItem::tDelete);
	deleteButton->setParentItem(this);

	// OK按钮
	QSvgRenderer svg2(ipfApplication::getThemeIconPath("ok.svg"));
	QPicture picture2;
	QPainter painter2(&picture2);
	svg2.render(&painter2, QRectF(0, 0, FlatButtonGraphicItem::bSize, FlatButtonGraphicItem::bSize));
	pt = QPointF(BOX_WIDTH / 2 + 5 + FlatButtonGraphicItem::bSize / 2, BOX_HEIGHT / 2 - FlatButtonGraphicItem::bSize / 2);
	okButton = new FlatButtonGraphicItem(picture2, pt, FlatButtonGraphicItem::tOk);
	okButton->setParentItem(this);
	okButton->setVisible(false);

	// err按钮
	QSvgRenderer svg3(ipfApplication::getThemeIconPath("err.svg"));
	QPicture picture3;
	QPainter painter3(&picture3);
	svg3.render(&painter3, QRectF(0, 0, FlatButtonGraphicItem::bSize, FlatButtonGraphicItem::bSize));
	pt = QPointF(0 - BOX_WIDTH / 2 - FlatButtonGraphicItem::bSize / 2 - 5, BOX_HEIGHT / 2 - FlatButtonGraphicItem::bSize / 2);
	errButton = new FlatButtonGraphicItem(picture3, pt, FlatButtonGraphicItem::tErr);
	errButton->setParentItem(this);
	errButton->setVisible(false);
}

ipfModelerGraphicItem::~ipfModelerGraphicItem()
{
}

void ipfModelerGraphicItem::colorInOut()
{
	color = QColor(238, 242, 131);
	stroke = QColor(234, 226, 118);
	selected = QColor(116, 113, 68);
}

void ipfModelerGraphicItem::colorChild()
{
	color = QColor(255, 255, 255);
	stroke = Qt::gray;
	selected = QColor(50, 50, 50);
}

QRectF ipfModelerGraphicItem::boundingRect() const
{
	QFontMetricsF fm = QFontMetricsF(font);
	int numParams = 1;
	int numOutputs = 1;

	qreal hUp = fm.height() * 1.2 * (numParams + 2);
	qreal hDown = fm.height() * 1.2 * (numOutputs + 2);

	qreal l = -(BOX_WIDTH + 2) / 2;
	qreal u = -(BOX_HEIGHT + 2) / 2 - hUp;
	qreal r = BOX_WIDTH + 2;
	qreal d = BOX_HEIGHT + hDown + hUp;

	if (isErrB)
	{
		l = l - FlatButtonGraphicItem::bSize -5;
		r = r + FlatButtonGraphicItem::bSize +5;
	}

	QRectF rect = QRectF(l, u, r, d);
	return rect;
}

void ipfModelerGraphicItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	// 矩形范围
	QRectF rect = QRectF(-(BOX_WIDTH + 2) / 2.0, -(BOX_HEIGHT + 2) / 2.0, BOX_WIDTH + 2, BOX_HEIGHT + 2);

	// 不同模块设置不同颜色
	if (process->typeName() == QString("OUT") || process->typeName() == QString("IN"))
	{
		colorInOut();
	}
	else
	{
		colorChild();
	}

	// 当图形被选中时，改变颜色
	if (isSelected())
	{
		stroke = selected;
		color = color.darker(110);
	}

	// 绘制矩形
	if (isColliding)
	{
		painter->setPen(QPen(colliding, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	}
	else
	{
		painter->setPen(QPen(stroke, 2, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
	}
	painter->setBrush(QBrush(color, Qt::SolidPattern));
	painter->drawRect(rect);

	// 绘制label
	painter->setFont(font);
	painter->setPen(QPen(Qt::black));
	itemLabel = getAdjustedText(itemLabel);

	QFontMetricsF fm = QFontMetricsF(font);
	itemLabel = getAdjustedText(itemLabel);
	qreal h = fm.ascent();
	QPointF pt = QPointF(-BOX_WIDTH / 2 + 25, BOX_HEIGHT / 2.0 - h + 1);
	painter->drawText(pt, itemLabel);
	painter->setPen(QPen(Qt::black));

	if (typeid(*(process->classType())) != typeid(ipfModelerProcessIn)
		|| typeid(*(process->classType())) != typeid(ipfModelerProcessOut))
	{
		h = -(fm.height() * 1.2);
		h = h - BOX_HEIGHT / 2.0 + 5;
		pt = QPointF(-BOX_WIDTH / 2 + 25, h);
		painter->drawText(pt, "In");

		// 显示多参数时，应实现
		//if (!self.element.parametersCollapsed())
		//    for param in [p for p in self.element.algorithm().parameterDefinitions() if not p.isDestination()]:
		//        if not param.flags() & QgsProcessingParameterDefinition.FlagHidden:
		//            text = self.getAdjustedText(param.description())
		//            h = -(fm.height() * 1.2) * (i + 1)
		//            h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
		//            pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 33, h)
		//            painter.drawText(pt, text)
		//            i += 1

		h = fm.height() * 1.1;
		h = h + BOX_HEIGHT / 2.0;
		pt = QPointF(-BOX_WIDTH / 2 + 25, h);
		painter->drawText(pt, "Out");

		// 显示多参数时，应实现
		//if not self.element.outputsCollapsed():
		//    for i, out in enumerate(self.element.algorithm().outputDefinitions()):
		//        text = self.getAdjustedText(out.description())
		//        h = fm.height() * 1.2 * (i + 2)
		//        h = h + ModelerGraphicItem.BOX_HEIGHT / 2.0
		//        pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 33, h)
		//        painter.drawText(pt, text)

		//if (!inButton->isFolded())
		//{
		//	QString text("22222222222222222222222222222222222222222");
		//	text = getAdjustedText(text);
		//	h = -(fm.height() * 1.2) * (1 + 1);
		//	h = h - BOX_HEIGHT / 2.0 + 5;
		//	pt = QPointF(-BOX_WIDTH / 2 + 33, h);
		//	painter->drawText(pt, text);
		//	prepareGeometryChange();
		//}

	}

	if (!picture.isNull())
		painter->drawPicture(-(BOX_WIDTH / 2.0) + 3, -8, picture);
}

QVariant ipfModelerGraphicItem::itemChange(GraphicsItemChange change, const QVariant & value)
{
	if (change == QGraphicsItem::ItemPositionHasChanged)
	{
		// 更新箭头
		foreach(ipfModelerArrowItem *arrow, arrows)
			arrow->updatePath();
	}
	return value;
}

void ipfModelerGraphicItem::isOkButton(const bool isbl)
{
	okButton->setVisible(isbl);
}

void ipfModelerGraphicItem::isErrButton(const bool isbl)
{
	errButton->setVisible(isbl);
	isErrB = isbl;
}

QString ipfModelerGraphicItem::getAdjustedText(QString text)
{
	QFontMetricsF fm = QFontMetricsF(font);
	qreal w = fm.width(text);
	if (w < BOX_WIDTH - 25 - FlatButtonGraphicItem::sSize)
		return text;

	text = text.mid(0, text.size() - 3) + QStringLiteral("…");
	w = fm.width(text);
	while (w > BOX_WIDTH - 25 - FlatButtonGraphicItem::sSize)
	{
		text = text.mid(0, text.size() - 4) + QStringLiteral("…");
		w = fm.width(text);
	}
	return text;
}

QPointF ipfModelerGraphicItem::getLinkPointForParameter(int paramIndex)
{
	paramIndex = -1;
	int offsetX = 17;
	//    if isinstance(self.element, QgsProcessingModelParameter):
	//        paramIndex = -1
	//        offsetX = 0
	QFont font("Verdana", 8);
	font.setPixelSize(12);
	QFontMetricsF fm(font);

	double h = -(fm.height() * 1.2) * (paramIndex + 2) - fm.height() / 2.0 + 8;
	h = h - BOX_HEIGHT / 2.0;

	return QPointF(-BOX_WIDTH / 2 + offsetX, h);
}

QPointF ipfModelerGraphicItem::getLinkPointForOutput(int outputIndex)
{
	if (outButton && outButton->isFolded())
	{
		outputIndex = -1;
	}

	QFont font("Verdana", 8);
	font.setPixelSize(12);
	QFontMetricsF fm(font);
	double w = fm.width(itemLabel);
	double h = fm.height() * 1.2 * (outputIndex + 1) + fm.height() / 2.0;
	double y = h + BOX_HEIGHT / 2.0 + 5;
	double x = -BOX_WIDTH / 2 + 33 + w + 5;

	if (outButton && outButton->isFolded())
	{
		x = 10;
	}

	return QPointF(x, y);
}

void ipfModelerGraphicItem::addArrow(ipfModelerArrowItem * arrow)
{
	arrows.append(arrow);
}

void ipfModelerGraphicItem::deleteArrows(QList<ipfModelerArrowItem*> arrows)
{
	foreach(ipfModelerArrowItem *arrow, arrows)
	{
		int index = this->arrows.indexOf(arrow);
		if (index != -1)
		{
			this->arrows.removeAt(index);
		}
	}
}

void ipfModelerGraphicItem::deleteArrow(ipfModelerArrowItem * arrow)
{
	int index = this->arrows.indexOf(arrow);
	if (index != -1)
	{
		this->arrows.removeAt(index);
	}
}

void ipfModelerGraphicItem::removeSelf()
{
	ipfFlowManage::instance()->deleteItem(this);
	scene()->removeItem(this);
}

void ipfModelerGraphicItem::removeGraphicItem()
{
	scene()->removeItem(this);
}

void ipfModelerGraphicItem::editParameter()
{
	process->setParameter();
}

void ipfModelerGraphicItem::diskplayInfo()
{
	ipfTextInfoDialog info;

	QStringList errList = process->getErrList();
	foreach(QString text, errList)
		info.appendText(text);

	info.exec();
}

void ipfModelerGraphicItem::setColliding(bool bl)
{
	isColliding = bl;
	prepareGeometryChange();
}

void ipfModelerGraphicItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
	menu->exec(event->screenPos());
}
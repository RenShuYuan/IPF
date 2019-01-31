#include "ipfPushButton.h"
#include "..\ipfapplication.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QPalette>
#include <QMouseEvent>
#include <QEvent>

ipfPushButton::ipfPushButton(QString & icon, QWidget * parent)
	: QPushButton(parent)
{
	setMaximumSize(60, 60);
	QPixmap pixmap(ipfApplication::getThemeIconPath(icon));
	setIcon(QIcon(pixmap));
	setIconSize(QSize(60, 60));

	QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0.5);
	setGraphicsEffect(opacityEffect);

}

ipfPushButton::~ipfPushButton()
{
}

void ipfPushButton::mousePressEvent(QMouseEvent * e)
{
	setIconSize(QSize(65, 65));
	QPushButton::mousePressEvent(e);
}

void ipfPushButton::mouseReleaseEvent(QMouseEvent * e)
{
	setIconSize(QSize(60, 60));
	QPushButton::mouseReleaseEvent(e);
}

void ipfPushButton::enterEvent(QEvent * event)
{
	QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
	shadow_effect->setOffset(2.5, 2.5);
	shadow_effect->setColor(Qt::gray);
	shadow_effect->setBlurRadius(8);
	setGraphicsEffect(shadow_effect);
	QPushButton::enterEvent(event);
}

void ipfPushButton::leaveEvent(QEvent * event)
{
	QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect;
	opacityEffect->setOpacity(0.5);
	setGraphicsEffect(opacityEffect);
	QPushButton::leaveEvent(event);
}

#ifndef IPFPUSHBUTTON_H
#define IPFPUSHBUTTON_H

#include <QPushButton>

class QGraphicsDropShadowEffect;
class QGraphicsOpacityEffect;

class ipfPushButton : public QPushButton
{
public:
	ipfPushButton(QString &icon, QWidget *parent = Q_NULLPTR);
	~ipfPushButton();

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
};

#endif // IPFPUSHBUTTON_H
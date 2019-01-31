#include "ipfTextInfoDialog.h"
#include <QMouseEvent>

ipfTextInfoDialog::ipfTextInfoDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	// 去除页面边框
	setWindowFlags(Qt::FramelessWindowHint);

	// 背景和元素都设置透明效果
	setWindowOpacity(0.7);

	ui.textEdit->setReadOnly(true);
}

ipfTextInfoDialog::~ipfTextInfoDialog()
{
}

void ipfTextInfoDialog::appendText(const QString & str)
{
	ui.textEdit->append(str);
}

void ipfTextInfoDialog::clear()
{
	ui.textEdit->clear();
}

void ipfTextInfoDialog::mousePressEvent(QMouseEvent * event)
{
	this->windowPos = this->pos();			// 获得部件当前位置
	this->mousePos = event->globalPos();	// 获得鼠标位置
	this->dPos = mousePos - windowPos;		// 移动后部件所在的位置
}

void ipfTextInfoDialog::mouseMoveEvent(QMouseEvent * event)
{
	this->move(event->globalPos() - this->dPos);
}

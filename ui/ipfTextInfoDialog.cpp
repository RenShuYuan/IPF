#include "ipfTextInfoDialog.h"
#include <QMouseEvent>

ipfTextInfoDialog::ipfTextInfoDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	// ȥ��ҳ��߿�
	setWindowFlags(Qt::FramelessWindowHint);

	// ������Ԫ�ض�����͸��Ч��
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
	this->windowPos = this->pos();			// ��ò�����ǰλ��
	this->mousePos = event->globalPos();	// ������λ��
	this->dPos = mousePos - windowPos;		// �ƶ��󲿼����ڵ�λ��
}

void ipfTextInfoDialog::mouseMoveEvent(QMouseEvent * event)
{
	this->move(event->globalPos() - this->dPos);
}

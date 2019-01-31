#include "ipfAboutDialog.h"
#include "../ipf/ipfapplication.h"

ipfAboutDialog::ipfAboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	// ȥ��ҳ��߿�
	setWindowFlags(Qt::FramelessWindowHint);

	// ������Ԫ�ض�����͸��Ч��
	setWindowOpacity(0.7);

	QPixmap pixmap(ipfApplication::getThemeIconPath(QStringLiteral("Earth_Globe.ico")));
	ui.label->setPixmap(pixmap);
}

ipfAboutDialog::~ipfAboutDialog()
{
}

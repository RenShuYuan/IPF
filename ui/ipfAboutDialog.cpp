#include "ipfAboutDialog.h"
#include "../ipf/ipfapplication.h"

ipfAboutDialog::ipfAboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	// 去除页面边框
	setWindowFlags(Qt::FramelessWindowHint);

	// 背景和元素都设置透明效果
	setWindowOpacity(0.7);

	QPixmap pixmap(ipfApplication::getThemeIconPath(QStringLiteral("Earth_Globe.ico")));
	ui.label->setPixmap(pixmap);
}

ipfAboutDialog::~ipfAboutDialog()
{
}

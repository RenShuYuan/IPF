#include "ipfModelerPixelDecimalsDialog.h"

ipfModelerPixelDecimalsDialog::ipfModelerPixelDecimalsDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	decimals = 0;
}

ipfModelerPixelDecimalsDialog::~ipfModelerPixelDecimalsDialog()
{
}

void ipfModelerPixelDecimalsDialog::setParameter(QMap<QString, QString> map)
{
	decimals = map["decimals"].toInt();
	ui.spinBox->setValue(decimals);
}

void ipfModelerPixelDecimalsDialog::on_pushButton_clicked()
{
	decimals = ui.spinBox->value();
	accept();
}
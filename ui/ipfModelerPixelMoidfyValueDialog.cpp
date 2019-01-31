#include "ipfModelerPixelMoidfyValueDialog.h"
#include "head.h"

ipfModelerPixelMoidfyValueDialog::ipfModelerPixelMoidfyValueDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerPixelMoidfyValueDialog::~ipfModelerPixelMoidfyValueDialog()
{
}

QMap<QString, QString> ipfModelerPixelMoidfyValueDialog::getParameter()
{
	QMap<QString, QString> map;
	map["oldValue"] = oldValue;
	map["newValue"] = newValue;

	return map;
}

void ipfModelerPixelMoidfyValueDialog::setParameter(QMap<QString, QString> map)
{
	oldValue = map["oldValue"];
	newValue = map["newValue"];
	ui.lineEdit->setText(oldValue);
	ui.lineEdit_2->setText(newValue);
}

void ipfModelerPixelMoidfyValueDialog::on_pushButton_clicked()
{
	oldValue = ui.lineEdit->text();
	newValue = ui.lineEdit_2->text();

	accept();
}
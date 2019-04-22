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
	map["oldValue_1"] = oldValue_1;
	map["newValue_1"] = newValue_1;
	map["oldValue_2"] = oldValue_2;
	map["newValue_2"] = newValue_2;

	return map;
}

void ipfModelerPixelMoidfyValueDialog::setParameter(QMap<QString, QString> map)
{
	oldValue_1 = map["oldValue_1"];
	newValue_1 = map["newValue_1"];
	oldValue_2 = map["oldValue_2"];
	newValue_2 = map["newValue_2"];
	ui.lineEdit->setText(oldValue_1);
	ui.lineEdit_2->setText(newValue_1);
	ui.lineEdit_3->setText(oldValue_2);
	ui.lineEdit_4->setText(newValue_2);
}

void ipfModelerPixelMoidfyValueDialog::on_pushButton_clicked()
{
	oldValue_1 = ui.lineEdit->text();
	newValue_1 = ui.lineEdit_2->text();
	oldValue_2 = ui.lineEdit_3->text();
	newValue_2 = ui.lineEdit_4->text();

	accept();
}
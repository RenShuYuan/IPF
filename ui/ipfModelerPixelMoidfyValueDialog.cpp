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

	if (bands_noDiffe)
		map["bands_noDiffe"] = "YES";
	else
		map["bands_noDiffe"] = "NO";

	return map;
}

void ipfModelerPixelMoidfyValueDialog::setParameter(QMap<QString, QString> map)
{
	oldValue = map["oldValue"];
	newValue = map["newValue"];
	if (map["bands_noDiffe"] == "YES")
		bands_noDiffe = true;
	else
		bands_noDiffe = false;

	ui.lineEdit->setText(oldValue);
	ui.lineEdit_2->setText(newValue);
	ui.checkBox->setChecked(bands_noDiffe);
}

void ipfModelerPixelMoidfyValueDialog::on_pushButton_clicked()
{
	oldValue = ui.lineEdit->text();
	newValue = ui.lineEdit_2->text();
	if (ui.checkBox->isChecked())
		bands_noDiffe = true;
	else
		bands_noDiffe = false;

	accept();
}
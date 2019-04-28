#include "ipfModelerSetNodataDialog.h"

ipfModelerSetNodataDialog::ipfModelerSetNodataDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerSetNodataDialog::~ipfModelerSetNodataDialog()
{
}

QMap<QString, QString> ipfModelerSetNodataDialog::getParameter()
{
	QMap<QString, QString> map;
	map["nodata"] = QString::number(nodata);

	if (isDel)
		map["isDel"] = "YES";
	else
		map["isDel"] = "NO";

	return map;
}

void ipfModelerSetNodataDialog::setParameter(QMap<QString, QString> map)
{
	nodata = map["nodata"].toDouble();
	if (map["isDel"] == "YES")
		isDel = true;
	else
		isDel = false;

	ui.doubleSpinBox->setValue(nodata);
	ui.checkBox->setChecked(isDel);
}

void ipfModelerSetNodataDialog::on_checkBox_clicked(bool checked)
{
	ui.doubleSpinBox->setEnabled(!checked);
}

void ipfModelerSetNodataDialog::on_pushButton_clicked()
{
	nodata = ui.doubleSpinBox->value();
	if (ui.checkBox->isChecked())
		isDel = true;
	else
		isDel = false;

	accept();
}
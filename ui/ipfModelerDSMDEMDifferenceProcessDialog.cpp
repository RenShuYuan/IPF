#include "ipfModelerDSMDEMDifferenceProcessDialog.h"

ipfModelerDSMDEMDifferenceProcessDialog::ipfModelerDSMDEMDifferenceProcessDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList list;
	list << QStringLiteral("DSM")
		<< QStringLiteral("DEM");
	ui.comboBox->addItems(list);
	ui.sb_threshold->setValue(1.0);
}

ipfModelerDSMDEMDifferenceProcessDialog::~ipfModelerDSMDEMDifferenceProcessDialog()
{
}

QMap<QString, QString> ipfModelerDSMDEMDifferenceProcessDialog::getParameter()
{
	QMap<QString, QString> map;
	map["type"] = type;
	map["threshold"] = QString::number(threshold);

	if (isFillNodata)
		map["isFillNodata"] = "YES";
	else
		map["isFillNodata"] = "NO";

	return map;
}

void ipfModelerDSMDEMDifferenceProcessDialog::setParameter(QMap<QString, QString> map)
{
	type = map["type"];
	threshold = map["threshold"].toDouble();

	if (map["isFillNodata"] == "YES")
		isFillNodata = true;
	else
		isFillNodata = false;

	ui.comboBox->setCurrentText(type);
	ui.sb_threshold->setValue(threshold);
	ui.checkBox->setChecked(isFillNodata);
}

void ipfModelerDSMDEMDifferenceProcessDialog::on_pushButton_clicked()
{
	type = ui.comboBox->currentText();
	threshold = ui.sb_threshold->value();

	if (ui.checkBox->isChecked())
		isFillNodata = true;
	else
		isFillNodata = false;

	accept();
}
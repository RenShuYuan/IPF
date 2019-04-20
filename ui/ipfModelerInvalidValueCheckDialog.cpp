#include "ipfModelerInvalidValueCheckDialog.h"
#include <QFileDialog>

ipfModelerInvalidValueCheckDialog::ipfModelerInvalidValueCheckDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerInvalidValueCheckDialog::~ipfModelerInvalidValueCheckDialog()
{
}

QMap<QString, QString> ipfModelerInvalidValueCheckDialog::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;
	map["invalidValue"] = invalidValue;

	if (isNegative)
		map["isNegative"] = "YES";
	else
		map["isNegative"] = "NO";
	if (isNodata)
		map["isNodata"] = "YES";
	else
		map["isNodata"] = "NO";
	if (isShape)
		map["isShape"] = "YES";
	else
		map["isShape"] = "NO";

	return map;
}

void ipfModelerInvalidValueCheckDialog::setParameter(QMap<QString, QString> map)
{
	saveName = map["saveName"];
	invalidValue = map["invalidValue"];

	if (map["isNegative"] == "YES")
		isNegative = true;
	else
		isNegative = false;
	if (map["isNodata"] == "YES")
		isNodata = true;
	else
		isNodata = false;
	if (map["isShape"] == "YES")
		isNodata = true;
	else
		isNodata = false;

	ui.lineEdit->setText(invalidValue);
	ui.lineEdit_2->setText(saveName);
	ui.checkBox->setChecked(isNegative);
	ui.checkBox_2->setChecked(isNodata);
	ui.checkBox_3->setChecked(isShape);
}

void ipfModelerInvalidValueCheckDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Ñ¡ÔñÎÄ¼þ¼Ð"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(dir));
	}
}

void ipfModelerInvalidValueCheckDialog::on_pushButton_clicked()
{
	invalidValue = ui.lineEdit->text();
	saveName = ui.lineEdit_2->text();
	if (ui.checkBox->isChecked())
		isNegative = true;
	else
		isNegative = false;
	if (ui.checkBox_2->isChecked())
		isNodata = true;
	else
		isNodata = false;
	if (ui.checkBox_3->isChecked())
		isShape = true;
	else
		isShape = false;

	accept();
}
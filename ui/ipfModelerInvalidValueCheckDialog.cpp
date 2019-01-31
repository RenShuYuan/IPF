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
	map["invalidValue"] = invalidValue;
	map["saveName"] = saveName;

	if (isNegative)
		map["isNegative"] = "YES";
	else
		map["isNegative"] = "NO";

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

	ui.lineEdit->setText(invalidValue);
	ui.lineEdit_2->setText(saveName);
	ui.checkBox->setChecked(isNegative);
}

void ipfModelerInvalidValueCheckDialog::on_pushButton_clicked()
{
	saveName = ui.lineEdit_2->text();
	invalidValue = ui.lineEdit->text();
	if (ui.checkBox->isChecked())
		isNegative = true;
	else
		isNegative = false;

	accept();
}

void ipfModelerInvalidValueCheckDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Ñ¡ÔñÎÄ¼þ¼Ð"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty())
	{
		return;
	}
	ui.lineEdit_2->setText(QDir::toNativeSeparators(dir));
}

#include "ipfModelerDiffeCheckDialog.h"
#include <QFileDialog>

ipfModelerDiffeCheckDialog::ipfModelerDiffeCheckDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerDiffeCheckDialog::~ipfModelerDiffeCheckDialog()
{
}

QMap<QString, QString> ipfModelerDiffeCheckDialog::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;
	map["valueMax"] = QString::number(valueMax);

	return map;
}

void ipfModelerDiffeCheckDialog::setParameter(QMap<QString, QString> map)
{
	saveName = map["saveName"];
	valueMax = map["valueMax"].toDouble();

	ui.lineEdit_2->setText(saveName);
	ui.doubleSpinBox->setValue(valueMax);
}

void ipfModelerDiffeCheckDialog::on_pushButton_clicked()
{
	valueMax = ui.doubleSpinBox->value();
	saveName = ui.lineEdit_2->text();

	accept();
}

void ipfModelerDiffeCheckDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Ñ¡ÔñÎÄ¼þ¼Ð"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(dir));
	}
}

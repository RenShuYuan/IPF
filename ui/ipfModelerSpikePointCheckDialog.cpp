#include "ipfModelerSpikePointCheckDialog.h"

ipfModelerSpikePointCheckDialog::ipfModelerSpikePointCheckDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerSpikePointCheckDialog::~ipfModelerSpikePointCheckDialog()
{
}

QMap<QString, QString> ipfModelerSpikePointCheckDialog::getParameter()
{
	QMap<QString, QString> map;
	map["threshold"] = QString::number(threshold);
	map["saveName"] = saveName;
	return map;
}

void ipfModelerSpikePointCheckDialog::setParameter(QMap<QString, QString> map)
{
	saveName = map["saveName"];
	ui.lineEdit->setText(saveName);

	threshold = map["threshold"].toDouble();
	ui.doubleSpinBox->setValue(threshold);
}

void ipfModelerSpikePointCheckDialog::on_pushButton_1_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Ñ¡ÔñÎÄ¼þ¼Ð"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(dir));
	}
}

void ipfModelerSpikePointCheckDialog::on_pushButton_clicked()
{
	threshold = ui.doubleSpinBox->value();
	saveName = ui.lineEdit->text();
	accept();
}
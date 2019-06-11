#include "ipfModelerExtractRasterRangeDialog.h"

ipfModelerExtractRasterRangeDialog::ipfModelerExtractRasterRangeDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	minimumRingsArea = 1000;
	background = 0.0;

	ui.sb_background->setValue(background);
	ui.sb_minimumRingsArea->setValue(minimumRingsArea);
}

ipfModelerExtractRasterRangeDialog::~ipfModelerExtractRasterRangeDialog()
{
}

QMap<QString, QString> ipfModelerExtractRasterRangeDialog::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["background"] = QString::number(background);
	map["minimumRingsArea"] = QString::number(minimumRingsArea);

	return map;
}

void ipfModelerExtractRasterRangeDialog::setParameter(QMap<QString, QString> map)
{
	fileName = map["fileName"];
	background = map["background"].toDouble();
	minimumRingsArea = map["minimumRingsArea"].toInt();

	ui.lineEdit->setText(fileName);
	ui.sb_background->setValue(background);
	ui.sb_minimumRingsArea->setValue(minimumRingsArea);
}

void ipfModelerExtractRasterRangeDialog::on_pushButton_clicked()
{
	fileName = ui.lineEdit->text();
	background = ui.sb_background->value();
	minimumRingsArea = ui.sb_minimumRingsArea->value();

	accept();
}

void ipfModelerExtractRasterRangeDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Ñ¡ÔñÎÄ¼þ¼Ð"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(dir));
	}
}

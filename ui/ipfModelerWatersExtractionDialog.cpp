#include "ipfModelerWatersExtractionDialog.h"

ipfModelerWatersExtractionDialog::ipfModelerWatersExtractionDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.horizontalSlider, &QSlider::valueChanged, this, &ipfModelerWatersExtractionDialog::setDoubleSpinBoxValue);
	connect(ui.sb_index, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));

	index = 0.3;
	minimumArea = 500;
	minimumRingsArea = 500;

	ui.sb_index->setValue(index);
	ui.sb_minimumArea->setValue(minimumArea);
	ui.sb_minimumRingsArea->setValue(minimumRingsArea);
}

ipfModelerWatersExtractionDialog::~ipfModelerWatersExtractionDialog()
{
}

QMap<QString, QString> ipfModelerWatersExtractionDialog::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["index"] = QString::number(index);
	map["minimumArea"] = QString::number(minimumArea);
	map["minimumRingsArea"] = QString::number(minimumRingsArea);

	return map;
}

void ipfModelerWatersExtractionDialog::setParameter(QMap<QString, QString> map)
{
	fileName = map["fileName"];
	index = map["index"].toDouble();
	minimumArea = map["minimumArea"].toInt();
	minimumRingsArea = map["minimumRingsArea"].toInt();

	ui.lineEdit->setText(fileName);
	ui.sb_index->setValue(index);
	ui.sb_minimumArea->setValue(minimumArea);
	ui.sb_minimumRingsArea->setValue(minimumRingsArea);
}

void ipfModelerWatersExtractionDialog::on_pushButton_clicked()
{
	fileName = ui.lineEdit->text();
	index = ui.sb_index->value();
	minimumArea = ui.sb_minimumArea->value();
	minimumRingsArea = ui.sb_minimumRingsArea->value();

	accept();
}

void ipfModelerWatersExtractionDialog::on_pushButton_1_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Ñ¡ÔñÎÄ¼þ¼Ð"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(dir));
	}
}

void ipfModelerWatersExtractionDialog::setSliderValue(const double value)
{
	int v = (int)(value * 100);
	ui.horizontalSlider->setValue(v);
}

void ipfModelerWatersExtractionDialog::setDoubleSpinBoxValue(const int value)
{
	double v = (double)value / 100;
	ui.sb_index->setValue(v);
}
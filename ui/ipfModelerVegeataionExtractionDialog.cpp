#include "ipfModelerVegeataionExtractionDialog.h"

#include <QFileDialog>

ipfModelerVegeataionExtractionDialog::ipfModelerVegeataionExtractionDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.horizontalSlider, &QSlider::valueChanged, this, &ipfModelerVegeataionExtractionDialog::setDoubleSpinBoxValue);
	connect(ui.sb_index, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));
	connect(ui.horizontalSlider_stlip, &QSlider::valueChanged, this, &ipfModelerVegeataionExtractionDialog::setDoubleSpinBox_stlip_Value);
	connect(ui.sb_stlip_index, SIGNAL(valueChanged(double)), this, SLOT(setSlider_stlip_Value(double)));

	index = 0.3;
	stlip_index = 0.0;
	minimumArea = 500;
	minimumRingsArea = 500;
	buffer = 20;

	ui.sb_index->setValue(index);
	ui.sb_stlip_index->setValue(stlip_index);
	ui.sb_minimumArea->setValue(minimumArea);
	ui.sb_minimumRingsArea->setValue(minimumRingsArea);
	ui.sb_buffer->setValue(buffer);
}

ipfModelerVegeataionExtractionDialog::~ipfModelerVegeataionExtractionDialog()
{

}

QMap<QString, QString> ipfModelerVegeataionExtractionDialog::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["index"] = QString::number(index);
	map["stlip_index"] = QString::number(stlip_index);
	map["minimumArea"] = QString::number(minimumArea);
	map["minimumRingsArea"] = QString::number(minimumRingsArea);
	map["buffer"] = QString::number(buffer);

	return map;
}

void ipfModelerVegeataionExtractionDialog::setParameter(QMap<QString, QString> map)
{
	fileName = map["fileName"];
	index = map["index"].toDouble();
	stlip_index = map["stlip_index"].toDouble();
	minimumArea = map["minimumArea"].toInt();
	minimumRingsArea = map["minimumRingsArea"].toInt();
	buffer = map["buffer"].toInt();

	ui.lineEdit->setText(fileName);
	ui.sb_index->setValue(index);
	ui.sb_stlip_index->setValue(stlip_index);
	ui.sb_minimumArea->setValue(minimumArea);
	ui.sb_minimumRingsArea->setValue(minimumRingsArea);
	ui.sb_buffer->setValue(buffer);
}

void ipfModelerVegeataionExtractionDialog::on_pushButton_clicked()
{
	fileName = ui.lineEdit->text();
	index = ui.sb_index->value();
	stlip_index = ui.sb_stlip_index->value();
	minimumArea = ui.sb_minimumArea->value();
	minimumRingsArea = ui.sb_minimumRingsArea->value();
	buffer = ui.sb_buffer->value();

	accept();
}

void ipfModelerVegeataionExtractionDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Ñ¡ÔñÎÄ¼þ¼Ð"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(dir));
	}
}

void ipfModelerVegeataionExtractionDialog::setSliderValue(const double value)
{
	int v = (int)(value * 100);
	ui.horizontalSlider->setValue(v);
}

void ipfModelerVegeataionExtractionDialog::setDoubleSpinBoxValue(const int value)
{
	double v = (double)value / 100;
	ui.sb_index->setValue(v);
}

void ipfModelerVegeataionExtractionDialog::setSlider_stlip_Value(const double value)
{
	int v = (int)(value * 100);
	ui.horizontalSlider_stlip->setValue(v);
}

void ipfModelerVegeataionExtractionDialog::setDoubleSpinBox_stlip_Value(const int value)
{
	double v = (double)value / 100;
	ui.sb_stlip_index->setValue(v);
}

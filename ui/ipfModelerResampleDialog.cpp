#include "ipfModelerResampleDialog.h"

ipfModelerResampleDialog::ipfModelerResampleDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	res = 0.0;

	QStringList list;
	list << QStringLiteral("near")
		<< QStringLiteral("bilinear")
		<< QStringLiteral("cubic")
		<< QStringLiteral("cubicspline")
		<< QStringLiteral("lanczos")
		<< QStringLiteral("average")
		<< QStringLiteral("mode")
		<< QStringLiteral("max")
		<< QStringLiteral("min")
		<< QStringLiteral("med")
		<< QStringLiteral("Q1")
		<< QStringLiteral("Q3");
	ui.comboBox->addItems(list);
	resampling_method = QStringLiteral("near");
}

ipfModelerResampleDialog::~ipfModelerResampleDialog()
{
}

QMap<QString, QString> ipfModelerResampleDialog::getParameter()
{
	QMap<QString, QString> map;
	map["resampling_method"] = resampling_method;
	map["res"] = QString::number(res, 'f', 15);

	return map;
}

void ipfModelerResampleDialog::setParameter(QMap<QString, QString> map)
{
	resampling_method = map["resampling_method"];
	res = map["res"].toDouble();

	ui.doubleSpinBox->setValue(res);
	ui.comboBox->setCurrentText(resampling_method);
}

void ipfModelerResampleDialog::on_pushButton_clicked()
{
	res = ui.doubleSpinBox->value();
	resampling_method = ui.comboBox->currentText();
	accept();
}
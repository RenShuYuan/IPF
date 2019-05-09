#include "ipfModelerRangeMoidfyValueDialog.h"
#include <QFileDialog>

ipfModelerRangeMoidfyValueDialog::ipfModelerRangeMoidfyValueDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerRangeMoidfyValueDialog::~ipfModelerRangeMoidfyValueDialog()
{
}

QMap<QString, QString> ipfModelerRangeMoidfyValueDialog::getParameter()
{
	QMap<QString, QString> map;
	map["vectorName"] = vectorName;
	map["value"] = QString::number(value);

	return map;
}

void ipfModelerRangeMoidfyValueDialog::setParameter(QMap<QString, QString> map)
{
	vectorName = map["vectorName"];
	value = map["value"].toDouble();

	ui.lineEdit->setText(vectorName);
	ui.sb_index->setValue(value);
}

void ipfModelerRangeMoidfyValueDialog::setValueEnable(const bool enable)
{
	ui.sb_index->setHidden(enable);
	ui.label_2->setHidden(enable);
}

void ipfModelerRangeMoidfyValueDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择矢量文件"), path, QStringLiteral("shp文件 (*.shp)"));
	if (!fileName.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));     // 将"/"转换为"\\"
	}
}

void ipfModelerRangeMoidfyValueDialog::on_pushButton_clicked()
{
	vectorName = ui.lineEdit->text();
	value = ui.sb_index->value();

	accept();
}
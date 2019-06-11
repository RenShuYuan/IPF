#include "ipfModelerWaterFlattenDialog.h"
#include <QDir>

ipfModelerWaterFlattenDialog::ipfModelerWaterFlattenDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerWaterFlattenDialog::~ipfModelerWaterFlattenDialog()
{
}

QMap<QString, QString> ipfModelerWaterFlattenDialog::getParameter()
{
	QMap<QString, QString> map;
	map["vectorName"] = vectorName;
	map["outPath"] = outPath;

	return map;
}

void ipfModelerWaterFlattenDialog::setParameter(QMap<QString, QString> map)
{
	vectorName = map["vectorName"];
	outPath = map["outPath"];

	ui.lineEdit->setText(QDir::toNativeSeparators(vectorName));     // 将"/"转换为"\\"
	ui.lineEdit_2->setText(QDir::toNativeSeparators(outPath));
}

void ipfModelerWaterFlattenDialog::on_pushButton_clicked()
{
	vectorName = ui.lineEdit->text();
	outPath = ui.lineEdit_2->text();

	accept();
}

void ipfModelerWaterFlattenDialog::on_pushButton_2_clicked()
{
	QSettings mSettings;
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择矢量文件"), path, QStringLiteral("shp文件 (*.shp)"));
	if (!fileName.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));     // 将"/"转换为"\\"
	}
}

void ipfModelerWaterFlattenDialog::on_pushButton_3_clicked()
{
	QSettings mSettings;
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("选择文件夹"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty())
	{
		return;
	}
	ui.lineEdit_2->setText(QDir::toNativeSeparators(dir));
}

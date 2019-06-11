#include "ipfModelerDemGrossErrorCheckDialog.h"
#include <QDir>

ipfModelerDemGrossErrorCheckDialog::ipfModelerDemGrossErrorCheckDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerDemGrossErrorCheckDialog::~ipfModelerDemGrossErrorCheckDialog()
{
}

QMap<QString, QString> ipfModelerDemGrossErrorCheckDialog::getParameter()
{
	QMap<QString, QString> map;
	map["rasterName"] = rasterName;
	map["errFile"] = errFile;
	map["threshold"] = QString::number(threshold);

	return map;
}

void ipfModelerDemGrossErrorCheckDialog::setParameter(QMap<QString, QString> map)
{
	rasterName = map["rasterName"];
	errFile = map["errFile"];
	threshold = map["threshold"].toDouble();
	ui.lineEdit->setText(rasterName);
	ui.lineEdit_2->setText(errFile);
	ui.doubleSpinBox->setValue(threshold);
}

void ipfModelerDemGrossErrorCheckDialog::on_pushButton_clicked()
{
	rasterName = ui.lineEdit->text();
	errFile = ui.lineEdit_2->text();
	threshold = ui.doubleSpinBox->value();
	accept();
}

void ipfModelerDemGrossErrorCheckDialog::on_pushButton_2_clicked()
{
	QString mRasterFileFilter = QStringLiteral("* (*.*);;TIFF (*.tif);;Erdas Imagine Images (*.img);;PCIDSK Database File (*.pix)");
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择数字高程模型"), path, mRasterFileFilter);
	if (!fileName.isEmpty())
	{
		// 将"/"转换为"\\"
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));
	}
}

void ipfModelerDemGrossErrorCheckDialog::on_pushButton_3_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString saveName = QFileDialog::getSaveFileName(this, QStringLiteral("输出错误文件")
		, path + QStringLiteral("/数字高程模型粗差检查问题.txt")
		, QStringLiteral("文本文件(*.txt)"));
	if (!saveName.isEmpty())
	{
		// 将"/"转换为"\\"
		ui.lineEdit_2->setText(QDir::toNativeSeparators(saveName));
	}
}

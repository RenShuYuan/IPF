#include "ipfModelerCreateMetadataDialog.h"

#include <QFileDialog>

ipfModelerCreateMetadataDialog::ipfModelerCreateMetadataDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList list;
	list << QStringLiteral("整景元数据")
		<< QStringLiteral("DSM元数据")
		<< QStringLiteral("DEM元数据")
		<< QStringLiteral("DOM元数据");
	ui.comboBox->addItems(list);
}

ipfModelerCreateMetadataDialog::~ipfModelerCreateMetadataDialog()
{
}

QMap<QString, QString> ipfModelerCreateMetadataDialog::getParameter()
{
	QMap<QString, QString> map;
	map["metaDataType"] = metaDataType;
	map["sample"] = sample;
	map["outPath"] = outPath;

	return map;
}

void ipfModelerCreateMetadataDialog::setParameter(QMap<QString, QString> map)
{
	metaDataType = map["metaDataType"];
	sample = map["sample"];
	outPath = map["outPath"];

	ui.comboBox->setCurrentText(metaDataType);
	ui.lineEdit->setText(sample);
	ui.lineEdit_2->setText(outPath);
}

void ipfModelerCreateMetadataDialog::on_pushButton_clicked()
{
	metaDataType = ui.comboBox->currentText();
	sample = ui.lineEdit->text();
	outPath = ui.lineEdit_2->text();

	accept();
}

void ipfModelerCreateMetadataDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择元数据样本文件"), path, QStringLiteral("excel (*.xls)"));
	if (!fileName.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));     // 将"/"转换为"\\"
	}
}

void ipfModelerCreateMetadataDialog::on_pushButton_3_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("选择文件夹"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(dir));
	}
}

void ipfModelerCreateMetadataDialog::on_comboBox_currentTextChanged(const QString & text)
{
	if (text == QStringLiteral("整景元数据"))
		ui.label_2->setText(QStringLiteral("    该模块接收数据为整景影像，数据命名及组织需要按项目要求整理好，否则个别内容会填写失败或整个元数据生成失败。\n    该模块会自动找出全色影像，并根据命名匹配多光谱影像。"));
	else if (text == QStringLiteral("DSM元数据")
			|| text == QStringLiteral("DEM元数据")
			|| text == QStringLiteral("DOM元数据"))
		ui.label_2->setText(QStringLiteral("    该模块接收数据为分幅裁切后DSM/DEM/DOM数据。"));
}

#include "ipfModelerFracClipDialog.h"
#include "qgsprojectionselectionwidget.h"

#include <QFileDialog>
#include <QGroupBox>

ipfModelerFracClipDialog::ipfModelerFracClipDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

}

ipfModelerFracClipDialog::~ipfModelerFracClipDialog()
{
}

void ipfModelerFracClipDialog::on_pushButton_clicked()
{
	fileName = ui.lineEdit->text();
	saveName = ui.lineEdit_2->text();
	ext = ui.spinBox->value();

	accept();
}

void ipfModelerFracClipDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择标准图号文本"), path, QStringLiteral("文本文件 (*.txt)"));
	if (!fileName.isEmpty())
	{
		// 将"/"转换为"\\"
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));
	}
}

void ipfModelerFracClipDialog::on_pushButton_3_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString saveName = QFileDialog::getSaveFileName(this, QStringLiteral("输出外扩坐标")
		, path + QStringLiteral("/out.txt")
		, QStringLiteral("文本文件(*.txt)"));
	if (!saveName.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(saveName));     // 将"/"转换为"\\"
	}
}

void ipfModelerFracClipDialog::on_groupBox_checked(bool checked)
{
	ui.groupBox->setCheckable(checked);
}

QMap<QString, QString> ipfModelerFracClipDialog::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["ext"] = QString::number(ext);

	if (ui.groupBox->isChecked())
	{
		map["isChecked"] = "true";
		map["saveName"] = saveName;
	}
	else
	{
		map["isChecked"] = "false";
		map["saveName"] = QString();
	}

	return map;
}

void ipfModelerFracClipDialog::setParameter(QMap<QString, QString> map)
{
	fileName = map["fileName"];
	ext = map["ext"].toInt();
	saveName = map["saveName"];
	QString isChecked = map["isChecked"];

	ui.lineEdit->setText(fileName);
	ui.lineEdit_2->setText(saveName);
	ui.spinBox->setValue(ext);
	if (isChecked == "true")
		ui.groupBox->setChecked(true);
	else
		ui.groupBox->setChecked(false);
}

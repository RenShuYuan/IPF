#include "ipfModelerZCheckDialog.h"

#include <QFileDialog>
#include <QDir>

ipfModelerZCheckDialog::ipfModelerZCheckDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerZCheckDialog::~ipfModelerZCheckDialog()
{
}

QMap<QString, QString> ipfModelerZCheckDialog::getParameter()
{
	QMap<QString, QString> map;
	map["flies"] = flies;
	map["saveName"] = saveName;

	return map;
}

void ipfModelerZCheckDialog::setParameter(QMap<QString, QString> map)
{
	flies = map["flies"];
	saveName = map["saveName"];

	ui.lineEdit->setText(flies);
	ui.lineEdit_2->setText(saveName);
}

void ipfModelerZCheckDialog::on_pushButton_clicked()
{
	flies = ui.lineEdit->text();
	saveName = ui.lineEdit_2->text();

	accept();
}

void ipfModelerZCheckDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择检查点文本"), path, QStringLiteral("文本文件 (*.txt)"));
	if (!fileName.isEmpty())
	{
		// 将"/"转换为"\\"
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));
	}
}

void ipfModelerZCheckDialog::on_pushButton_3_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("选择精度报告输出文件夹"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty())
	{
		return;
	}
	ui.lineEdit_2->setText(QDir::toNativeSeparators(dir));
}

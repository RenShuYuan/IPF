#include "ipfModelerFracExtentCheckDialog.h"
#include <QFileDialog>

ipfModelerFracExtentCheckDialog::ipfModelerFracExtentCheckDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerFracExtentCheckDialog::~ipfModelerFracExtentCheckDialog()
{
}

QMap<QString, QString> ipfModelerFracExtentCheckDialog::getParameter()
{
	QMap<QString, QString> map;
	map["ext"] = QString::number(ext);
	map["saveName"] = saveName;

	return map;
}

void ipfModelerFracExtentCheckDialog::setParameter(QMap<QString, QString> map)
{
	ext = map["ext"].toInt();
	saveName = map["saveName"];

	ui.lineEdit_2->setText(saveName);
	ui.spinBox->setValue(ext);
}

void ipfModelerFracExtentCheckDialog::on_pushButton_clicked()
{
	saveName = ui.lineEdit_2->text();
	ext = ui.spinBox->value();

	accept();
}

void ipfModelerFracExtentCheckDialog::on_pushButton_3_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString saveName = QFileDialog::getSaveFileName(this, QStringLiteral("输出错误文件")
		, path + QStringLiteral("/分幅范围检查问题.txt")
		, QStringLiteral("文本文件(*.txt)"));
	if (!saveName.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(saveName));     // 将"/"转换为"\\"
	}
}

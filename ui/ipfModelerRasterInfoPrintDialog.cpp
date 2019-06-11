#include "ipfModelerRasterInfoPrintDialog.h"

#include <QDir>

ipfModelerRasterInfoPrintDialog::ipfModelerRasterInfoPrintDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerRasterInfoPrintDialog::~ipfModelerRasterInfoPrintDialog()
{
}

QMap<QString, QString> ipfModelerRasterInfoPrintDialog::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;

	return map;
}

void ipfModelerRasterInfoPrintDialog::setParameter(QMap<QString, QString> map)
{
	saveName = map["saveName"];
	ui.lineEdit_2->setText(saveName);
}

void ipfModelerRasterInfoPrintDialog::on_pushButton_clicked()
{
	saveName = ui.lineEdit_2->text();
	accept();
}

void ipfModelerRasterInfoPrintDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString saveName = QFileDialog::getSaveFileName(this, QStringLiteral("输出栅格信息")
		, path + QStringLiteral("/栅格信息.txt")
		, QStringLiteral("文本文件(*.txt)"));
	if (!saveName.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(saveName));     // 将"/"转换为"\\"
	}
}

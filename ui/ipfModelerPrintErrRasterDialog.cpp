#include "ipfModelerPrintErrRasterDialog.h"
#include <QFileDialog>

ipfModelerPrintErrRasterDialog::ipfModelerPrintErrRasterDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerPrintErrRasterDialog::~ipfModelerPrintErrRasterDialog()
{
}

QMap<QString, QString> ipfModelerPrintErrRasterDialog::getParameter()
{
	QMap<QString, QString> map;
	map["saveName"] = saveName;

	return map;
}

void ipfModelerPrintErrRasterDialog::setParameter(QMap<QString, QString> map)
{
	saveName = map["saveName"];

	ui.lineEdit_2->setText(saveName);
}

void ipfModelerPrintErrRasterDialog::on_pushButton_clicked()
{
	saveName = ui.lineEdit_2->text();

	accept();
}

void ipfModelerPrintErrRasterDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Ñ¡ÔñÎÄ¼þ¼Ð"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(dir));
	}
}

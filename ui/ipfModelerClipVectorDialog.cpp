#include "ipfModelerClipVectorDialog.h"

#include <QFileDialog>
#include <QDir>

ipfModelerClipVectorDialog::ipfModelerClipVectorDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerClipVectorDialog::~ipfModelerClipVectorDialog()
{
}

QString ipfModelerClipVectorDialog::getParameter()
{
	return vectorName;
}

void ipfModelerClipVectorDialog::setParameter(QMap<QString, QString> map)
{
	vectorName = map["vectorName"];
	ui.lineEdit->setText(QDir::toNativeSeparators(vectorName));     // ��"/"ת��Ϊ"\\"
}

void ipfModelerClipVectorDialog::on_pushButton_clicked()
{
	vectorName = ui.lineEdit->text();
	accept();
}

void ipfModelerClipVectorDialog::on_pushButton_2_clicked()
{
	QSettings mSettings;
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ��ʸ���ļ�"), path, QStringLiteral("shp�ļ� (*.shp)"));
	if (!fileName.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));     // ��"/"ת��Ϊ"\\"
	}
}
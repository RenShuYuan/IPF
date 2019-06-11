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
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ�����ָ߳�ģ��"), path, mRasterFileFilter);
	if (!fileName.isEmpty())
	{
		// ��"/"ת��Ϊ"\\"
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));
	}
}

void ipfModelerDemGrossErrorCheckDialog::on_pushButton_3_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString saveName = QFileDialog::getSaveFileName(this, QStringLiteral("��������ļ�")
		, path + QStringLiteral("/���ָ߳�ģ�ʹֲ�������.txt")
		, QStringLiteral("�ı��ļ�(*.txt)"));
	if (!saveName.isEmpty())
	{
		// ��"/"ת��Ϊ"\\"
		ui.lineEdit_2->setText(QDir::toNativeSeparators(saveName));
	}
}

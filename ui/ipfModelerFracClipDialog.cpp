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
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ���׼ͼ���ı�"), path, QStringLiteral("�ı��ļ� (*.txt)"));
	if (!fileName.isEmpty())
	{
		// ��"/"ת��Ϊ"\\"
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));
	}
}

void ipfModelerFracClipDialog::on_pushButton_3_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString saveName = QFileDialog::getSaveFileName(this, QStringLiteral("�����������")
		, path + QStringLiteral("/out.txt")
		, QStringLiteral("�ı��ļ�(*.txt)"));
	if (!saveName.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(saveName));     // ��"/"ת��Ϊ"\\"
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

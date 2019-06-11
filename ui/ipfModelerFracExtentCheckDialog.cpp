#include "ipfModelerFracExtentCheckDialog.h"

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
	map["saveName"] = saveName;
	return map;
}

void ipfModelerFracExtentCheckDialog::setParameter(QMap<QString, QString> map)
{
	saveName = map["saveName"];
	ui.lineEdit_2->setText(saveName);
}

void ipfModelerFracExtentCheckDialog::on_pushButton_clicked()
{
	saveName = ui.lineEdit_2->text();
	accept();
}

void ipfModelerFracExtentCheckDialog::on_pushButton_3_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString saveName = QFileDialog::getSaveFileName(this, QStringLiteral("��������ļ�")
		, path + QStringLiteral("/�ַ���Χ�������.txt")
		, QStringLiteral("�ı��ļ�(*.txt)"));
	if (!saveName.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(saveName));     // ��"/"ת��Ϊ"\\"
	}
}

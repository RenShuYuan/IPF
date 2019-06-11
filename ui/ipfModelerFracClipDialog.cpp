#include "ipfModelerFracClipDialog.h"
#include "qgsprojectionselectionwidget.h"

#include <QGroupBox>

ipfModelerFracClipDialog::ipfModelerFracClipDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList list;
	list << QStringLiteral("DOM")
		<< QStringLiteral("DSM")
		<< QStringLiteral("DEM");
	ui.comboBox->addItems(list);
}

ipfModelerFracClipDialog::~ipfModelerFracClipDialog()
{
}

void ipfModelerFracClipDialog::on_pushButton_clicked()
{
	fileName = ui.lineEdit->text();
	dateType = ui.comboBox->currentText();

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

QMap<QString, QString> ipfModelerFracClipDialog::getParameter()
{
	QMap<QString, QString> map;
	map["fileName"] = fileName;
	map["dateType"] = dateType;

	return map;
}

void ipfModelerFracClipDialog::setParameter(QMap<QString, QString> map)
{
	fileName = map["fileName"];
	dateType = map["dateType"];

	ui.lineEdit->setText(fileName);
	ui.comboBox->setCurrentText(dateType);
}

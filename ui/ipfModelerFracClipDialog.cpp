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
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择标准图号文本"), path, QStringLiteral("文本文件 (*.txt)"));
	if (!fileName.isEmpty())
	{
		// 将"/"转换为"\\"
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

#include "ipfModelerCreateMetadataDialog.h"

#include <QFileDialog>

ipfModelerCreateMetadataDialog::ipfModelerCreateMetadataDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList list;
	list << QStringLiteral("����Ԫ����")
		<< QStringLiteral("DSMԪ����")
		<< QStringLiteral("DEMԪ����")
		<< QStringLiteral("DOMԪ����");
	ui.comboBox->addItems(list);
}

ipfModelerCreateMetadataDialog::~ipfModelerCreateMetadataDialog()
{
}

QMap<QString, QString> ipfModelerCreateMetadataDialog::getParameter()
{
	QMap<QString, QString> map;
	map["metaDataType"] = metaDataType;
	map["sample"] = sample;
	map["outPath"] = outPath;

	return map;
}

void ipfModelerCreateMetadataDialog::setParameter(QMap<QString, QString> map)
{
	metaDataType = map["metaDataType"];
	sample = map["sample"];
	outPath = map["outPath"];

	ui.comboBox->setCurrentText(metaDataType);
	ui.lineEdit->setText(sample);
	ui.lineEdit_2->setText(outPath);
}

void ipfModelerCreateMetadataDialog::on_pushButton_clicked()
{
	metaDataType = ui.comboBox->currentText();
	sample = ui.lineEdit->text();
	outPath = ui.lineEdit_2->text();

	accept();
}

void ipfModelerCreateMetadataDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ��Ԫ���������ļ�"), path, QStringLiteral("excel (*.xls)"));
	if (!fileName.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));     // ��"/"ת��Ϊ"\\"
	}
}

void ipfModelerCreateMetadataDialog::on_pushButton_3_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("ѡ���ļ���"), path
		, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.lineEdit_2->setText(QDir::toNativeSeparators(dir));
	}
}

void ipfModelerCreateMetadataDialog::on_comboBox_currentTextChanged(const QString & text)
{
	if (text == QStringLiteral("����Ԫ����"))
		ui.label_2->setText(QStringLiteral("    ��ģ���������Ϊ����Ӱ��������������֯��Ҫ����ĿҪ������ã�����������ݻ���дʧ�ܻ�����Ԫ��������ʧ�ܡ�\n    ��ģ����Զ��ҳ�ȫɫӰ�񣬲���������ƥ������Ӱ��"));
	else if (text == QStringLiteral("DSMԪ����")
			|| text == QStringLiteral("DEMԪ����")
			|| text == QStringLiteral("DOMԪ����"))
		ui.label_2->setText(QStringLiteral("    ��ģ���������Ϊ�ַ����к�DSM/DEM/DOM���ݡ�"));
}

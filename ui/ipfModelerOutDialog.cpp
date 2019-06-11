#include "ipfModelerOutDialog.h"
#include <QDir>

ipfModelerOutDialog::ipfModelerOutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList formats;
	formats << QStringLiteral("tif: TIFF")
			<< QStringLiteral("img: Erdas Imagine Images")
			<< QStringLiteral("vrt: GDAL VRT Format")
			<< QStringLiteral("pix: PCIDSK Database File");
	ui.comboBox->addItems(formats);

	QStringList compressList;
	compressList << "NONE" << "JPEG" << "LZW"
			<< "PACKBITS" << "DEFLATE" << "CCITTRLE"
			<< "CCITTFAX3" << "CCITTFAX4" << "LZMA" << "ZSTD";
	ui.comboBox_2->addItems(compressList);

	format = QStringLiteral("tif");
	compress = "NONE";
	isTfw = "NO";
	noData = "none";
}

ipfModelerOutDialog::~ipfModelerOutDialog()
{
}

QMap<QString, QString> ipfModelerOutDialog::getParameter()
{
	QMap<QString, QString> map;
	map["format"] = format;
	map["outPath"] = outPath;
	map["noData"] = noData;

	if (ui.comboBox->currentText() == QStringLiteral("tif: TIFF"))
	{
		map["compress"] = compress;
		map["isTfw"] = isTfw;
	}
	else
	{
		map["compress"] = "NONE";
		map["isTfw"] = "NO";
	}

	return map;
}

void ipfModelerOutDialog::setParameter(QMap<QString, QString> map)
{
	format = map["format"];
	outPath = map["outPath"];
	compress = map["compress"];
	isTfw = map["isTfw"];
	
	if (map["noData"] == "none")
		noData = QString();
	else
		noData = map["noData"];

	int index = ui.comboBox->findText(format, Qt::MatchStartsWith);
	if (index != -1)
		ui.comboBox->setCurrentIndex(index);
	ui.lineEdit->setText(outPath);
	ui.lineEdit_2->setText(noData);
	ui.comboBox_2->setCurrentText(compress);
	if (isTfw == "YES")
		ui.checkBox->setChecked(true);
	else
		ui.checkBox->setChecked(false);
}

void ipfModelerOutDialog::on_pushButton_2_clicked()
{
	QSettings mSettings;
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Ñ¡ÔñÎÄ¼þ¼Ð"), path
				, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty())
	{
		return;
	}
	ui.lineEdit->setText(QDir::toNativeSeparators(dir));
}

void ipfModelerOutDialog::on_comboBox_currentTextChanged(const QString & text)
{
	if (text == QStringLiteral("tif: TIFF"))
	{
		ui.comboBox_2->setEnabled(true);
		ui.checkBox->setEnabled(true);
	}
	else
	{
		ui.comboBox_2->setEnabled(false);
		ui.checkBox->setEnabled(false);
	}
}

void ipfModelerOutDialog::on_pushButton_clicked()
{
	QString str = ui.comboBox->currentText();
	format = str.split(':').at(0);

	outPath = ui.lineEdit->text();
	if (outPath.back() == '\\')
	{
		outPath.remove(outPath.size() - 1, 1);
	}

	noData = ui.lineEdit_2->text();
	if (noData.isEmpty())
		noData = "none";
	bool isOk;
	long l = noData.toLong(&isOk);
	if (!isOk)
		noData = "none";

	compress = ui.comboBox_2->currentText();
	if (ui.checkBox->isChecked())
		isTfw = "YES";
	else
		isTfw = "NO";

	accept();
}
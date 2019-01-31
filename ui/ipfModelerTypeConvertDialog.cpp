#include "ipfModelerTypeConvertDialog.h"
#include "../ipf/gdal/ipfgdalprogresstools.h"

ipfModelerTypeConvertDialog::ipfModelerTypeConvertDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList formats;
	formats << QStringLiteral("Byte")
		<< QStringLiteral("UInt16")
		<< QStringLiteral("Int16");

	ui.comboBox->addItems(formats);
}

ipfModelerTypeConvertDialog::~ipfModelerTypeConvertDialog()
{
}

void ipfModelerTypeConvertDialog::setParameter(QMap<QString, QString> map)
{
	dataType = map["dataType"];
	ui.comboBox->setCurrentText(dataType);
}

void ipfModelerTypeConvertDialog::on_pushButton_clicked()
{
	dataType = ui.comboBox->currentText();
	accept();
}

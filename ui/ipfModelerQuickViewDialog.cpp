#include "ipfModelerQuickViewDialog.h"

ipfModelerQuickViewDialog::ipfModelerQuickViewDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList list;
	list << QStringLiteral("1")
		<< QStringLiteral("2")
		<< QStringLiteral("3")
		<< QStringLiteral("4")
		<< QStringLiteral("5")
		<< QStringLiteral("6")
		<< QStringLiteral("7")
		<< QStringLiteral("8");
	ui.comboBox->addItems(list);

	bs = 1;
}

ipfModelerQuickViewDialog::~ipfModelerQuickViewDialog()
{
}

void ipfModelerQuickViewDialog::setParameter(QMap<QString, QString> map)
{
	bs = map["bs"].toInt();
	if (bs > 0)
		ui.comboBox->setCurrentIndex(bs - 1);
}

void ipfModelerQuickViewDialog::on_pushButton_clicked()
{
	int index = ui.comboBox->currentIndex();
	if (index != -1)
		bs = index+1;

	accept();
}
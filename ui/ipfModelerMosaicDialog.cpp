#include "ipfModelerMosaicDialog.h"

ipfModelerMosaicDialog::ipfModelerMosaicDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ipfModelerMosaicDialog::~ipfModelerMosaicDialog()
{
}

void ipfModelerMosaicDialog::on_pushButton_clicked()
{
	accept();
}
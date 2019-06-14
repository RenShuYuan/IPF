#include "ipfModelerInDialog.h"
#include <QStandardItemModel>
#include <QFileInfo>
#include <QCloseEvent>
#include <QDir>

#include "qgsproviderregistry.h"
#include "qgsapplication.h"

ipfModelerInDialog::ipfModelerInDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.tableWidget->setRowCount(0);
	ui.tableWidget->setColumnCount(1);
	ui.tableWidget->setHorizontalHeaderLabels(QStringList() <<  QStringLiteral("已添加的栅格数据"));
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	mRasterFileFilter = (
		"* (*.*);;"
		"TIFF (*.tif);;"
		"Erdas Imagine Images (*.img);;"
		"PCIDSK Database File (*.pix);;"
		"Esri Grid (hdr.adf)");
}

ipfModelerInDialog::~ipfModelerInDialog()
{
}

void ipfModelerInDialog::on_pushButton_2_clicked()
{
	files.clear();
	for (int i = 0; i < ui.tableWidget->rowCount(); ++i)
	{
		QTableWidgetItem *item = ui.tableWidget->item(i, 0);
		files.append(item->text());
	}

	accept();
}

void ipfModelerInDialog::on_pushButton_3_clicked()
{
	QList<QTableWidgetItem *> Items = ui.tableWidget->selectedItems();
	foreach(QTableWidgetItem *item, Items)
	{
		ui.tableWidget->removeRow(item->row());
	}
}

void ipfModelerInDialog::on_pushButton_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QStringList files = QFileDialog::getOpenFileNames(this, QStringLiteral("选择栅格文件"), path, mRasterFileFilter);

	int size = files.size();
	if (!size)	return;

	QFileInfo info(files.at(0));
	mSettings.setValue("/rasterPath", info.filePath());
	
	int rowCount = ui.tableWidget->rowCount();
	ui.tableWidget->setRowCount(rowCount + size);

	foreach (QString file, files)
	{
		ui.tableWidget->setItem(rowCount++, 0, new QTableWidgetItem(QDir::toNativeSeparators(file)));
	}
}
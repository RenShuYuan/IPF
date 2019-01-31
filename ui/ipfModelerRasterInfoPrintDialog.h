#ifndef IPFMODELERRASTERINFOPRINTDIALOG_H
#define IPFMODELERRASTERINFOPRINTDIALOG_H

#include <QDialog>
#include "ui_ipfModelerRasterInfoPrintDialog.h"
#include "head.h"

class ipfModelerRasterInfoPrintDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerRasterInfoPrintDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerRasterInfoPrintDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

private:
	Ui::ipfModelerRasterInfoPrintDialog ui;
	
	QSettings mSettings;
	QString saveName;
};

#endif // IPFMODELERRASTERINFOPRINTDIALOG_H
#ifndef IPFMODELERPRINTERRRASTERDIALOG_H
#define IPFMODELERPRINTERRRASTERDIALOG_H

#include "head.h"
#include <QDialog>
#include "ui_ipfModelerPrintErrRasterDialog.h"

class ipfModelerPrintErrRasterDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerPrintErrRasterDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerPrintErrRasterDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);
private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

private:
	Ui::ipfModelerPrintErrRasterDialog ui;
	QSettings mSettings;

	QString saveName;
};

#endif // IPFMODELERPRINTERRRASTERDIALOG_H
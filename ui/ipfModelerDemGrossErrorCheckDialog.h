#ifndef IPFMODELERDEMGROSSERRORCHECKDIALOG_H
#define IPFMODELERDEMGROSSERRORCHECKDIALOG_H

#include <QDialog>
#include "ui_ipfModelerDemGrossErrorCheckDialog.h"
#include "head.h"

class ipfModelerDemGrossErrorCheckDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerDemGrossErrorCheckDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerDemGrossErrorCheckDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();

private:
	Ui::ipfModelerDemGrossErrorCheckDialog ui;
	QSettings mSettings;
	QString rasterName;
	QString errFile;
	double threshold;
};

#endif // IPFMODELERDEMGROSSERRORCHECKDIALOG_H
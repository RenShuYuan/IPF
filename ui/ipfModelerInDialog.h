#ifndef IPFMODELERINDIALOG_H
#define IPFMODELERINDIALOG_H

#include <QDialog>
#include "ui_ipfModelerIn.h"
#include "head.h"

class ipfModelerInDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerInDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerInDialog();

	QStringList getParameter() { return files; };

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();

private:
	Ui::ipfModelerIn ui;
	QSettings mSettings;

	QString mRasterFileFilter;
	QStringList files;
};

#endif // IPFMODELERINDIALOG_H
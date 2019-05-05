#ifndef IPFMODELERDIFFECHECKDIALOG_H
#define IPFMODELERDIFFECHECKDIALOG_H

#include <QDialog>
#include "ui_ipfModelerDiffeCheckDialog.h"
#include "head.h"

class ipfModelerDiffeCheckDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerDiffeCheckDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerDiffeCheckDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

private:
	Ui::ipfModelerDiffeCheckDialog ui;
	QSettings mSettings;

	QString saveName;
	double valueMax;
};

#endif // IPFMODELERDIFFECHECKDIALOG_H
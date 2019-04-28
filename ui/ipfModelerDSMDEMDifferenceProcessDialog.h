#ifndef IPFMODELERDSMDEMDIFFERENCEPROCESSDIALOG_H
#define IPFMODELERDSMDEMDIFFERENCEPROCESSDIALOG_H

#include <QDialog>
#include "ui_ipfModelerDSMDEMDifferenceProcessDialog.h"
#include "head.h"

class ipfModelerDSMDEMDifferenceProcessDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerDSMDEMDifferenceProcessDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerDSMDEMDifferenceProcessDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();

private:
	Ui::ipfModelerDSMDEMDifferenceProcessDialog ui;
	QSettings mSettings;

	QString type;
	double threshold;
	bool isFillNodata;
};

#endif // IPFMODELERDSMDEMDIFFERENCEPROCESSDIALOG_H
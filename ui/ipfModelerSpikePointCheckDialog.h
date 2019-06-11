#ifndef IPFMODELERSPIKEPOINTCHECKDIALOG_H
#define IPFMODELERSPIKEPOINTCHECKDIALOG_H

#include <QDialog>
#include "ui_ipfModelerSpikePointCheckDialog.h"
#include "head.h"

class ipfModelerSpikePointCheckDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerSpikePointCheckDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerSpikePointCheckDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_1_clicked();

private:
	Ui::ipfModelerSpikePointCheckDialog ui;
	QSettings mSettings;
	QString saveName;
	double threshold;
};

#endif // IPFMODELERSPIKEPOINTCHECKDIALOG_H
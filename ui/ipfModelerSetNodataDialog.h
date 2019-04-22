#ifndef IPFMODELERSETNODATADIALOG_H
#define IPFMODELERSETNODATADIALOG_H

#include <QDialog>
#include "ui_ipfModelerSetNodataDialog.h"
#include "head.h"

class ipfModelerSetNodataDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerSetNodataDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerSetNodataDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_checkBox_clicked(bool checked);

private:
	Ui::ipfModelerSetNodataDialog ui;
	QSettings mSettings;

	QString nodata;
	bool isDel;
};

#endif IPFMODELERSETNODATADIALOG_H
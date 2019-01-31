#ifndef IPFMODELERWATERFLATTENDIALOG_H
#define IPFMODELERWATERFLATTENDIALOG_H

#include <QDialog>
#include "ui_ipfModelerWaterFlattenDialog.h"
#include "head.h"

class ipfModelerWaterFlattenDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerWaterFlattenDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerWaterFlattenDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();

private:
	Ui::ipfModelerWaterFlattenDialog ui;
	QString vectorName;
	QString outPath;
};

#endif // IPFMODELERWATERFLATTENDIALOG_H
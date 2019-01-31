#ifndef IPFMODELERZCHECKDIALOG_H
#define IPFMODELERZCHECKDIALOG_H

#include <QDialog>
#include "ui_ipfModelerZCheckDialog.h"
#include "head.h"

class ipfModelerZCheckDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerZCheckDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerZCheckDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();

private:
	Ui::ipfModelerZCheckDialog ui;

	QSettings mSettings;
	QString flies;
	QString saveName;

};

#endif // IPFMODELERZCHECKDIALOG_H
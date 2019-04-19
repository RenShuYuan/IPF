#ifndef IPFMODELERFRACEXTENTCHECKDIALOG_H
#define IPFMODELERFRACEXTENTCHECKDIALOG_H

#include <QDialog>
#include "ui_ipfModelerFracExtentCheckDialog.h"
#include "head.h"

class ipfModelerFracExtentCheckDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerFracExtentCheckDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerFracExtentCheckDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_3_clicked();
private:
	Ui::ipfModelerFracExtentCheckDialog ui;
	QSettings mSettings;

	QString saveName;
};

#endif // IPFMODELERFRACEXTENTCHECKDIALOG_H
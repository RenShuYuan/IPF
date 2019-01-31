#ifndef IPFMODELERFRACCLIPDIALOG_H
#define IPFMODELERFRACCLIPDIALOG_H

#include <QDialog>
#include "ui_ipfModelerFracClipDialog.h"
#include "head.h"

class ipfModelerFracClipDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerFracClipDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerFracClipDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();
	void on_groupBox_checked(bool checked);
private:
	Ui::ipfModelerFracClipDialog ui;
	QSettings mSettings;

	QString fileName;
	QString saveName;
	int ext;
};

#endif // IPFMODELERFRACCLIPDIALOG_H
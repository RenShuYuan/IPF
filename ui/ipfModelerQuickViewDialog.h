#ifndef IPFMODELERQUICKVIEWDIALOG_H
#define IPFMODELERQUICKVIEWDIALOG_H

#include <QDialog>
#include "ui_ipfModelerQuickViewDialog.h"

class ipfModelerQuickViewDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerQuickViewDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerQuickViewDialog();

	int getParameter() { return bs; };
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();

private:
	Ui::ipfModelerQuickViewDialog ui;
	int bs;
};

#endif // IPFMODELERQUICKVIEWDIALOG_H
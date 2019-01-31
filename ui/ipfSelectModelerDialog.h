#ifndef IPFSELECTMODELERDIALOG_H
#define IPFSELECTMODELERDIALOG_H

#include <QDialog>
#include "ui_ipfSelectModelerDialog.h"

class ipfSelectModelerDialog : public QDialog
{
	Q_OBJECT

public:
	ipfSelectModelerDialog(QWidget *parent = Q_NULLPTR);
	~ipfSelectModelerDialog();

	QString getParameter() { return moderName; };

private slots:
	void on_pushButton_clicked();

private:
	Ui::ipfSelectModelerDialog ui;
	QString moderName;
};

#endif // IPFSELECTMODELERDIALOG_H
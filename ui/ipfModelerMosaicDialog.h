#ifndef IPFMODELERMOSAICDIALOG_H
#define IPFMODELERMOSAICDIALOG_H

#include <QDialog>
#include "ui_ipfModelerMosaicDialog.h"

class ipfModelerMosaicDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerMosaicDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerMosaicDialog();

private slots:
	void on_pushButton_clicked();
private:
	Ui::ipfModelerMosaicDialog ui;
};

#endif // IPFMODELERMOSAICDIALOG_H
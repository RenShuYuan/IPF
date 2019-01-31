#ifndef IPFMODELERPIXELDECIMALSDIALOG_H
#define IPFMODELERPIXELDECIMALSDIALOG_H

#include <QDialog>
#include "ui_ipfModelerPixelDecimalsDialog.h"

class ipfModelerPixelDecimalsDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerPixelDecimalsDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerPixelDecimalsDialog();

	int getParameter() { return decimals; };
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();

private:
	Ui::ipfModelerPixelDecimalsDialog ui;
	int decimals;
};

#endif // IPFMODELERPIXELDECIMALSDIALOG_H
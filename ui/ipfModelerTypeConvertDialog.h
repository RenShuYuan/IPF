#ifndef IPFMODELERTYPECONVERTDIALOG_H
#define IPFMODELERTYPECONVERTDIALOG_H

#include <QDialog>
#include "ui_ipfModelerTypeConvertDialog.h"

class ipfModelerTypeConvertDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerTypeConvertDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerTypeConvertDialog();

	QString getParameter() { return dataType; };
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();

private:
	Ui::ipfModelerTypeConvertDialog ui;
	QString dataType;
};

#endif // IPFMODELERTYPECONVERTDIALOG_H
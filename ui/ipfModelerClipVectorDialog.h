#ifndef IPFMODELERCLIPVECTORDIALOG_H
#define IPFMODELERCLIPVECTORDIALOG_H

#include "head.h"

#include <QDialog>
#include "ui_ipfModelerClipVectorDialog.h"

class ipfModelerClipVectorDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerClipVectorDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerClipVectorDialog();

	QString getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

private:
	Ui::ipfModelerClipVectorDialog ui;
	QString vectorName;
};

#endif // IPFMODELERCLIPVECTORDIALOG_H
#ifndef IPFMODELERRESAMPLEDIALOG_H
#define IPFMODELERRESAMPLEDIALOG_H

#include <QDialog>
#include "ui_ipfModelerResampleDialog.h"

class ipfModelerResampleDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerResampleDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerResampleDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();

private:
	Ui::ipfModelerResampleDialog ui;
	QString resampling_method;
	double res;
};

#endif // IPFMODELERRESAMPLEDIALOG_H
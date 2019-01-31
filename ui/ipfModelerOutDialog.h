#ifndef IPFMODELEROUT_H
#define IPFMODELEROUT_H

#include "head.h"
#include <QDialog>
#include "ui_ipfModelerOut.h"

class ipfModelerOutDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerOutDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerOutDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_comboBox_currentTextChanged(const QString &text);

private:
	Ui::ipfModelerOut ui;
	QString format;
	QString outPath;
	QString compress;
	QString isTfw;
	QString noData;
};

#endif // IPFMODELEROUT_H
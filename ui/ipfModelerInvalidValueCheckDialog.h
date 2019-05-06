#ifndef IPFMODELERINVALIDVALUECHECKDIALOG_H
#define IPFMODELERINVALIDVALUECHECKDIALOG_H

#include <QDialog>
#include "ui_ipfModelerInvalidValueCheckDialog.h"
#include "head.h"

class ipfModelerInvalidValueCheckDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerInvalidValueCheckDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerInvalidValueCheckDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

private:
	Ui::ipfModelerInvalidValueCheckDialog ui;
	QSettings mSettings;

	QString saveName;
	QString invalidValue;
	bool isNegative;
	bool isNodata;
	bool isShape;
	bool bands_noDiffe;
};

#endif // IPFMODELERINVALIDVALUECHECKDIALOG_H
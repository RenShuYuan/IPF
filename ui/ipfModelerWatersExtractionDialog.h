#ifndef IPFMODELERWATERSEXTRACTIONDIALOG_H
#define IPFMODELERWATERSEXTRACTIONDIALOG_H

#include <QDialog>
#include "ui_ipfModelerWatersExtractionDialog.h"
#include "head.h"

class ipfModelerWatersExtractionDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerWatersExtractionDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerWatersExtractionDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_1_clicked();

	void setSliderValue(const double value);
	void setDoubleSpinBoxValue(const int value);

private:
	Ui::ipfModelerWatersExtractionDialog ui;
	QSettings mSettings;

	QString fileName;
	double index;
	int minimumArea;
	int minimumRingsArea;
};

#endif // IPFMODELERWATERSEXTRACTIONDIALOG_H
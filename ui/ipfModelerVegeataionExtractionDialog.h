#ifndef IPFMODELERVEGEATAIONEXTRACTIONDIALOG_H
#define IPFMODELERVEGEATAIONEXTRACTIONDIALOG_H

#include <QDialog>
#include "ui_ipfModelerVegeataionExtractionDialog.h"
#include "head.h"

class ipfModelerVegeataionExtractionDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerVegeataionExtractionDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerVegeataionExtractionDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

	void setSliderValue(const double value);
	void setDoubleSpinBoxValue(const int value);

	void setSlider_stlip_Value(const double value);
	void setDoubleSpinBox_stlip_Value(const int value);

private:
	Ui::ipfModelerVegeataionExtractionDialog ui;
	QSettings mSettings;

	QString fileName;
	double index;
	double stlip_index;
	int minimumArea;
	int minimumRingsArea;
	int buffer;
};

#endif // IPFMODELERVEGEATAIONEXTRACTIONDIALOG_H
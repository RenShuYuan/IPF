#pragma once

#include <QDialog>
#include "ui_ipfProgress.h"

class ipfProgress : public QDialog
{
	Q_OBJECT

public:
	ipfProgress(QWidget *parent = Q_NULLPTR);
	~ipfProgress();

	void setTitle(const QString& label);
	void setValue(int value);
	void pulsValue();
	void userPulsValueTatal();
	void setRangeChild(int minimum, int maximum);
	void setRangeTotal(int minimum, int maximum);

	bool wasCanceled() { return isKeep; };

private:
	void pulsValueTatal();

private slots:
	void on_pushButton_clicked();

private:
	Ui::ipfProgress ui;
	bool isKeep;
	bool isPuls;
	int childCount;
	int tatalCount;
};

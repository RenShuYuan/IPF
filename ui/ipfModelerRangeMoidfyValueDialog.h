#ifndef IPFMODELERRANGEMOIDFYVALUEDIALOG_H
#define IPFMODELERRANGEMOIDFYVALUEDIALOG_H

#include <QDialog>
#include "ui_ipfModelerRangeMoidfyValueDialog.h"
#include "head.h"

class ipfModelerRangeMoidfyValueDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerRangeMoidfyValueDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerRangeMoidfyValueDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

	void setValueEnable(const bool enable);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

private:
	Ui::ipfModelerRangeMoidfyValueDialog ui;
	QSettings mSettings;

	QString vectorName;
	double value;
};

#endif // IPFMODELERRANGEMOIDFYVALUEDIALOG_H
#ifndef IPFMODELEREXTRACTRASTERRANGEDIALOG
#define IPFMODELEREXTRACTRASTERRANGEDIALOG

#include <QDialog>
#include "ui_ipfModelerExtractRasterRangeDialog.h"
#include "head.h"

class ipfModelerExtractRasterRangeDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerExtractRasterRangeDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerExtractRasterRangeDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

private:
	Ui::ipfModelerExtractRasterRangeDialog ui;
	QSettings mSettings;

	QString fileName;
	double background;
	int minimumRingsArea;
};

#endif // IPFMODELEREXTRACTRASTERRANGEDIALOG
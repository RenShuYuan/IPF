#ifndef IPFMODELERPROJECTIONCHECKDIALOG_H
#define IPFMODELERPROJECTIONCHECKDIALOG_H

#include <QDialog>
#include "ui_ipfModelerProjectionCheckDialog.h"
#include "head.h"

class QgsProjectionSelectionWidget;

class ipfModelerProjectionCheckDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerProjectionCheckDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerProjectionCheckDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

private:
	Ui::ipfModelerProjectionCheckDialog ui;
	QSettings mSettings;
	QgsProjectionSelectionWidget *leUavLayerSrcCrs;
	QString s_srs;
	QString saveName;
};

#endif // IPFMODELERPROJECTIONCHECKDIALOG_H
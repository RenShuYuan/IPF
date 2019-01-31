#ifndef IPFMODELERTRANSFORMDIALOG_H
#define IPFMODELERTRANSFORMDIALOG_H

#include <QDialog>
#include "ui_ipfModelerTransformDialog.h"

class QgsProjectionSelectionWidget;

class ipfModelerTransformDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerTransformDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerTransformDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();

private:
	Ui::ipfModelerTransformDialog ui;
	QgsProjectionSelectionWidget *leUavLayerSrcCrs;
	QgsProjectionSelectionWidget *leUavLayerDestCrs;

	QString resampling_method;
	QString s_srs;
	QString t_srs;
};

#endif // IPFMODELERTRANSFORMDIALOG_H
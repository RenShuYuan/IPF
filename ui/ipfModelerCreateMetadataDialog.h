#ifndef IPFMODELERZJMETADATADIALOG_H
#define IPFMODELERZJMETADATADIALOG_H

#include <QDialog>
#include "ui_ipfModelerCreateMetadataDialog.h"
#include "head.h"

class ipfModelerCreateMetadataDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerCreateMetadataDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerCreateMetadataDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();
	void on_comboBox_currentTextChanged(const QString &text);

private:
	Ui::ipfModelerZjMetadataDialog ui;
	QSettings mSettings;
	QString metaDataType;
	QString outPath;
	QString sample;
};

#endif // IPFMODELERZJMETADATADIALOG_H
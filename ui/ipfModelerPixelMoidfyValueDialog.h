#ifndef IPFMODELERPROCESSCHILDPIXELMOIDFYVALUEDIALOG_H
#define IPFMODELERPROCESSCHILDPIXELMOIDFYVALUEDIALOG_H

#include <QDialog>
#include "ui_ipfModelerPixelMoidfyValueDialog.h"

class ipfModelerPixelMoidfyValueDialog : public QDialog
{
	Q_OBJECT

public:
	ipfModelerPixelMoidfyValueDialog(QWidget *parent = Q_NULLPTR);
	~ipfModelerPixelMoidfyValueDialog();

	QMap<QString, QString> getParameter();
	void setParameter(QMap<QString, QString> map);

private slots:
	void on_pushButton_clicked();

private:
	Ui::ipfModelerProcessChildPixelMoidfyValueDialog ui;
	QString oldValue_1;
	QString newValue_1;
	QString oldValue_2;
	QString newValue_2;
	bool bands_noDiffe;
};

#endif // IPFMODELERPROCESSCHILDPIXELMOIDFYVALUEDIALOG_H
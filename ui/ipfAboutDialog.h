#ifndef IPFABOUTDIALOG_H
#define IPFABOUTDIALOG_H

#include <QDialog>
#include "ui_ipfAboutDialog.h"

class ipfAboutDialog : public QDialog
{
	Q_OBJECT

public:
	ipfAboutDialog(QWidget *parent = Q_NULLPTR);
	~ipfAboutDialog();

private:
	Ui::ipfAboutDialog ui;
};

#endif // IPFABOUTDIALOG_H
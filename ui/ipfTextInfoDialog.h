#ifndef IPFTEXTINFODIALOG_H
#define IPFTEXTINFODIALOG_H

#include <QDialog>
#include "ui_ipfTextInfoDialog.h"

class ipfTextInfoDialog : public QDialog
{
	Q_OBJECT

public:
	ipfTextInfoDialog(QWidget *parent = Q_NULLPTR);
	~ipfTextInfoDialog();

	void appendText(const QString &str);
	void clear();

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

private:
	Ui::ipfTextInfoDialog ui;
	QPoint windowPos;
	QPoint mousePos;
	QPoint dPos;
};

#endif // IPFTEXTINFODIALOG_H
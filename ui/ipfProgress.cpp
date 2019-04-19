#include "ipfProgress.h"

ipfProgress::ipfProgress(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	isKeep = false;
	tatalCount = 0;
}

ipfProgress::~ipfProgress()
{
}

void ipfProgress::setTitle(const QString & label)
{
	if (label.isEmpty())
		this->setWindowTitle(QStringLiteral("处理进度"));
	else
		this->setWindowTitle(label);
}

void ipfProgress::setValue(int value)
{
	ui.progressBarChild->setValue(value);
	if (value == ui.progressBarChild->maximum())
		pulsValueTatal();
}

void ipfProgress::pulsValueTatal()
{
	ui.progressBar->setValue(++tatalCount);
}

void ipfProgress::setRangeChild(int minimum, int maximum)
{
	ui.progressBarChild->setRange(minimum, maximum);
}

void ipfProgress::setRangeTotal(int minimum, int maximum)
{
	ui.progressBar->setRange(minimum, maximum);
}

void ipfProgress::on_pushButton_clicked()
{
	isKeep = true;
}

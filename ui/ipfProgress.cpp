#include "ipfProgress.h"

ipfProgress::ipfProgress(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	isKeep = false;
	isPuls = false;
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
	QApplication::processEvents();
	if (value == ui.progressBarChild->maximum())
		pulsValueTatal();
	else
		isPuls = true;
}

void ipfProgress::pulsValueTatal()
{
	if (isPuls)
	{
		isPuls = false;
		ui.progressBar->setValue(++tatalCount);
		QApplication::processEvents();
	}
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

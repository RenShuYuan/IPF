#include "ipfProgress.h"

ipfProgress::ipfProgress(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	isKeep = false;
	isPuls = false;
	childCount = 0;
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
	// 防止使用omp加速时冲突
#pragma omp critical
	{
		if (value == ui.progressBarChild->maximum())
			pulsValueTatal();
		else if (value < ui.progressBarChild->maximum())
		{
			isPuls = true;
			ui.progressBarChild->setValue(value);
			QApplication::processEvents();
		}
	}
}

void ipfProgress::pulsValue()
{
	// 防止使用omp加速时冲突
#pragma omp critical
	{
		++childCount;
		if (childCount == ui.progressBarChild->maximum())
			pulsValueTatal();
		else if (childCount < ui.progressBarChild->maximum())
		{
			isPuls = true;
			ui.progressBarChild->setValue(childCount);
			QApplication::processEvents();
		}
	}
}

void ipfProgress::userPulsValueTatal()
{
	childCount = ui.progressBarChild->minimum();
	ui.progressBar->setValue(++tatalCount);
	QApplication::processEvents();
}

void ipfProgress::pulsValueTatal()
{
	if (isPuls)
	{
		isPuls = false;
		childCount = ui.progressBarChild->minimum();
		ui.progressBar->setValue(++tatalCount);
		QApplication::processEvents();
	}
}

void ipfProgress::setRangeChild(int minimum, int maximum)
{
	ui.progressBarChild->setRange(minimum, maximum);
	ui.progressBarChild->setValue(childCount = minimum);
	QApplication::processEvents();
}

void ipfProgress::setRangeTotal(int minimum, int maximum)
{
	ui.progressBar->setRange(minimum, maximum);
	ui.progressBar->setValue(tatalCount = minimum);
}

void ipfProgress::on_pushButton_clicked()
{
	isKeep = true;
}

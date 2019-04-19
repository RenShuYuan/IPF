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

void ipfProgress::addProgress(QProgressBar * pProcess)
{
	if (childList.contains(pProcess))
		return;

//#pragma omp critical
	//{
		childList << pProcess;
		ui.verticalLayout->insertWidget(0, pProcess);
		QApplication::processEvents();
	//}
}

void ipfProgress::setTitle(const QString & label)
{
	if (label.isEmpty())
		this->setWindowTitle(QStringLiteral("处理进度"));
	else
		this->setWindowTitle(label);
}

void ipfProgress::setValue(int value, QProgressBar *pProcessChild)
{
#pragma omp critical
		{
		pProcessChild->setValue(value);
		QApplication::processEvents();
		if (value == ui.progressBarChild->maximum())
		{
			ui.verticalLayout->removeWidget(pProcessChild);
			pulsValueTatal();
		}
	}
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

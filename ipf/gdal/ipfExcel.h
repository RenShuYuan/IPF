#ifndef IPFEXCEL_H
#define IPFEXCEL_H

#include <QObject>
#include "head.h"

class QAxObject;

class ipfExcel : public QObject
{
	Q_OBJECT

public:
	ipfExcel();
	~ipfExcel();

	QString open(const QString &excelFile);
	void close();
	void editCell(const QString &range, const QVariant & value);
	QString getCell(const QString &range);

private:
	QAxObject * excel;
	QAxObject * workbook;
	QAxObject* worksheet;
};

#endif // IPFEXCEL_H
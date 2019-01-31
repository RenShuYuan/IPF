#ifndef IPFMODELERPROCESSCHILDWATERSEXTRACTION
#define IPFMODELERPROCESSCHILDWATERSEXTRACTION

#include "ipfModelerProcessOut.h"

class ipfModelerWatersExtractionDialog;

class ipfModelerProcessChildWatersExtraction : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildWatersExtraction(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildWatersExtraction();

	ipfModelerProcessChildWatersExtraction* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();
private:
	double watersIndex(const double G, const double NIR);

private:
	ipfModelerWatersExtractionDialog * dialog;
	QString fieldName;
	QString fileName;
	double index;
	double minimumArea;
	double minimumRingsArea;
};

#endif // IPFMODELERPROCESSCHILDWATERSEXTRACTION
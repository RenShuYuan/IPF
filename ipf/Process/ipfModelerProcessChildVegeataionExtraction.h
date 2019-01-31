#ifndef IPFMODELERPROCESSCHILDVEGEATAIONEXTRACTION
#define IPFMODELERPROCESSCHILDVEGEATAIONEXTRACTION

#include "ipfModelerProcessOut.h"

class ipfModelerVegeataionExtractionDialog;

class ipfModelerProcessChildVegeataionExtraction : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildVegeataionExtraction(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildVegeataionExtraction();

	ipfModelerProcessChildVegeataionExtraction* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	double vegeataionIndex(const double R, const double NIR);
	double ylviIndex(const double B, const double G);

private:
	ipfModelerVegeataionExtractionDialog * dialog;
	QString fieldName;
	QString fileName;
	double index;
	double stlip_index;
	double minimumArea;
	double minimumRingsArea;
	double buffer;
};

#endif // IPFMODELERPROCESSCHILDVEGEATAIONEXTRACTION
#ifndef IPFMODELEREXTRACTRASTERRANGE
#define IPFMODELEREXTRACTRASTERRANGE

#include "ipfModelerProcessOut.h"
class ipfModelerExtractRasterRangeDialog;

class ipfModelerProcessChildExtractRasterRange : public ipfModelerProcessOut
{
public:
	ipfModelerProcessChildExtractRasterRange(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildExtractRasterRange();

	ipfModelerProcessChildExtractRasterRange* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	ipfModelerExtractRasterRangeDialog * dialog;
	QString fieldName;
	QString fileName;
	double minimumRingsArea;
	double background;
};

#endif // IPFMODELEREXTRACTRASTERRANGE
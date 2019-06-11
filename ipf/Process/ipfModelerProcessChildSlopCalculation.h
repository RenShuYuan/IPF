#ifndef IPFMODELERPROCESSCHILDSLOPCALCULATION_H
#define IPFMODELERPROCESSCHILDSLOPCALCULATION_H

#include "ipfModelerProcessBase.h"
#include "QgsVectorLayer.h"
#include "QgsSpatialIndex.h"
#include "../../clipper/clipper.hpp"

using namespace ClipperLib;

class ipfModelerClipVectorDialog;

class ipfModelerProcessChildSlopCalculation :
	public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildSlopCalculation(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildSlopCalculation();

	ipfModelerProcessChildSlopCalculation* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();
private:
	ipfModelerClipVectorDialog * dialog;
};

#endif // IPFMODELERPROCESSCHILDSLOPCALCULATION_H
#ifndef IPFMODELERPROCESSCHILDCONSISTENCY_H
#define IPFMODELERPROCESSCHILDCONSISTENCY_H

#include "ipfModelerProcessBase.h"
class ipfModelerProcessChildConsistency : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildConsistency(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildConsistency();

	ipfModelerProcessChildConsistency* classType() { return this; };

	bool checkParameter();
	void setParameter();

	void run();

private:
	// �������б��в��Ҷ�Ӧͼ��
	int getFilesIndex(const QStringList &lists, const QString &th);

	bool chackRasterVaule0(const QString &file);

};

#endif // IPFMODELERPROCESSCHILDCONSISTENCY_H
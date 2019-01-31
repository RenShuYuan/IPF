#ifndef IPFFLOWMANAGE_H
#define IPFFLOWMANAGE_H

#include <QObject>
#include <QList>
#include <QTemporaryDir>

class ipfModelerGraphicItem;

class ipfFlowManage : public QObject
{
	Q_OBJECT

public:
	ipfFlowManage(QObject *parent);
	~ipfFlowManage();

	void new_();
	void save();
	void run();
	void check();

	static ipfFlowManage *instance() { return smInstance; }

	// ����vrt��ʽ����ʱ�ļ�����Ҫ·��+�ļ�����+��չ��
	QString getTempVrtFile(const QString& file);
	
	// ����ָ����ʽ����ʱ�ļ���file�������ļ�·��, format����չ��
	QString getTempFormatFile(const QString& file, const QString& format);

	// Ĭ�������ߺ�׷��
	void appendItem(ipfModelerGraphicItem* item);

	// ����һ���·�֧
	void appendBranchItem(ipfModelerGraphicItem* prvItem, ipfModelerGraphicItem* item);
	void insrtItem(ipfModelerGraphicItem* prvItem, ipfModelerGraphicItem* item);

	// ���鷳�ĺ���
	void deleteItem(ipfModelerGraphicItem* item);

	// ����ģ��
	ipfModelerGraphicItem* findModeler(const QString& id);

	bool isEmpty();

private:
	// �ݹ麯�������ҽ�������з�֧
	void deleteTreeBranch(ipfModelerGraphicItem* item);
	void deleteAllBranch(QList<ipfModelerGraphicItem*> *items);

	void printBranch();

private:
	QList< QList<ipfModelerGraphicItem*>* > branchManagement;

	static ipfFlowManage *smInstance;
	QTemporaryDir tempDir;

	bool isCheck;
};

#endif // IPFFLOWMANAGE_H
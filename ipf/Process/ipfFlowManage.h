#ifndef IPFFLOWMANAGE_H
#define IPFFLOWMANAGE_H

#include <QObject>
#include <QList>

class ipfModelerGraphicItem;

class ipfFlowManage : public QObject
{
	Q_OBJECT

public:
	ipfFlowManage(QObject *parent);
	~ipfFlowManage();

	static ipfFlowManage *instance() { return smInstance; }

	void new_();
	void save();
	void run();
	void check();

	// 默认在主线后追加
	void appendItem(ipfModelerGraphicItem* item);

	// 创建一条新分支
	void appendBranchItem(ipfModelerGraphicItem* prvItem, ipfModelerGraphicItem* item);
	void insrtItem(ipfModelerGraphicItem* prvItem, ipfModelerGraphicItem* item);

	// 最麻烦的函数
	void deleteItem(ipfModelerGraphicItem* item);

	// 查找模块
	ipfModelerGraphicItem* findModeler(const QString& id);

	bool isEmpty();

private:
	// 递归函数，查找结点下所有分支
	void deleteTreeBranch(ipfModelerGraphicItem* item);
	void deleteAllBranch(QList<ipfModelerGraphicItem*> *items);

	void printBranch();

private:
	QList< QList<ipfModelerGraphicItem*>* > branchManagement;
	static ipfFlowManage *smInstance;
	bool isCheck;
};

#endif // IPFFLOWMANAGE_H
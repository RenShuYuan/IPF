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

	// 生成vrt格式的临时文件，需要路径+文件名称+扩展名
	QString getTempVrtFile(const QString& filePath);
	
	// 生成指定格式的临时文件，file：完整文件路径, format：扩展名(.vrt)
	QString getTempFormatFile(const QString& filePath, const QString& format);

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
	QTemporaryDir tempDir;
	bool isCheck;
};

#endif // IPFFLOWMANAGE_H
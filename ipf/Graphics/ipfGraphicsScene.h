#ifndef IPFGRAPHICSSCENE_H
#define IPFGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QDomDocument>
#include <QMenu>
#include <QAction>

class ipfModelerGraphicItem;
class ipfModelerProcessBase;
class ipfFlowManage;

class ipfGraphicsScene : public QGraphicsScene
{
public:
	ipfGraphicsScene(ipfFlowManage *flow, QObject *parent = Q_NULLPTR);
	~ipfGraphicsScene();

	void addModel(const QString &itemName, QPointF pt);
	void insrtModel(ipfModelerGraphicItem *prvItem, const QString &itemName);

	void new_();
	void load();
	void load(const QString &fileName);
	void save();

protected:
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
	// 创建分支
	void createBranch();

	// 设置模块的默认参数
	void setModelerDialogParameter(ipfModelerProcessBase* process, QDomNode node);

	ipfModelerProcessBase* createProcessBase(const QString &itemName);
private:
	QList<ipfModelerGraphicItem*> paramItems;
	ipfFlowManage *flow;
	QMenu *menu;

	// 右键选中的图形
	QGraphicsItem *item;
};

#endif // IPFGRAPHICSVIEW_H
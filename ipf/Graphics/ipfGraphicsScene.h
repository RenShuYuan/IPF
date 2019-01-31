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
	// ������֧
	void createBranch();

	// ����ģ���Ĭ�ϲ���
	void setModelerDialogParameter(ipfModelerProcessBase* process, QDomNode node);

	ipfModelerProcessBase* createProcessBase(const QString &itemName);
private:
	QList<ipfModelerGraphicItem*> paramItems;
	ipfFlowManage *flow;
	QMenu *menu;

	// �Ҽ�ѡ�е�ͼ��
	QGraphicsItem *item;
};

#endif // IPFGRAPHICSVIEW_H